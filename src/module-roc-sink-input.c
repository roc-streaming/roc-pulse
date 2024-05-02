/*
 * This file is part of Roc PulseAudio integration.
 *
 * Copyright (c) Roc Streaming authors
 *
 * Licensed under GNU Lesser General Public License 2.1 or any later version.
 */

/* config.h from pulseaudio directory (generated after ./configure) */
#include <config.h>

/* public pulseaudio headers */
#include <pulse/xmalloc.h>

/* private pulseaudio headers */
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/namereg.h>
#include <pulsecore/sink-input.h>

/* roc headers */
#include <roc/context.h>
#include <roc/log.h>
#include <roc/receiver.h>
#include <roc/version.h>

/* local headers */
#include "rocpulse_helpers.h"

PA_MODULE_AUTHOR("Roc Streaming authors");
PA_MODULE_DESCRIPTION("Read audio stream from Roc receiver");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE("sink=<name for the sink> "
                "sink_input_properties=<properties for the sink input> "
                "local_ip=<local receiver ip> "
                "local_source_port=<local receiver port for source (RTP) packets> "
                "local_repair_port=<local receiver port for repair (FEC) packets> "
                "local_control_port=<local receiver port for control (RTCP) packets> "
                "packet_encoding_id=<8-bit number> "
                "packet_encoding_rate=<sample rate> "
                "packet_encoding_format=s16 "
                "packet_encoding_chans=mono|stereo "
                "fec_encoding=disable|rs8m|ldpc "
                "resampler_backend=default|builtin|speex|speexdec "
                "resampler_profile=default|high|medium|low "
                "latency_backend=default|niq "
                "latency_profile=default|intact|responsive|gradual "
                "target_latency_msec=<target latency in milliseconds> "
                "min_latency_msec=<minimum latency in milliseconds> "
                "max_latency_msec=<maximum latency in milliseconds> "
                "io_latency_msec=<playback latency in milliseconds> "
                "no_play_timeout_msec=<no playback timeout in milliseconds> "
                "choppy_play_timeout_msec=<choppy playback timeout in milliseconds>");

struct roc_sink_input_userdata {
    pa_module* module;
    pa_sink_input* sink_input;

    roc_endpoint* local_source_endp;
    roc_endpoint* local_repair_endp;
    roc_endpoint* local_control_endp;

    roc_context* context;
    roc_receiver* receiver;
};

static const char* const roc_sink_input_modargs[] = { //
    "sink",                                           //
    "sink_input_name",                                //
    "sink_input_properties",                          //
    "local_ip",                                       //
    "local_source_port",                              //
    "local_repair_port",                              //
    "local_control_port",                             //
    "packet_encoding_id",                             //
    "packet_encoding_rate",                           //
    "packet_encoding_format",                         //
    "packet_encoding_chans",                          //
    "fec_encoding",                                   //
    "resampler_backend",                              //
    "resampler_profile",                              //
    "latency_backend",                                //
    "latency_profile",                                //
    "target_latency_msec",                            //
    "min_latency_msec",                               //
    "max_latency_msec",                               //
    "io_latency_msec",                                //
    "no_play_timeout_msec",                           //
    "choppy_play_timeout_msec",                       //
    NULL
};

static int process_message(
    pa_msgobject* o, int code, void* data, int64_t offset, pa_memchunk* chunk) {
    struct roc_sink_input_userdata* u = PA_SINK_INPUT(o)->userdata;
    pa_assert(u);

    switch (code) {
    case PA_SINK_INPUT_MESSAGE_GET_LATENCY:
        /* TODO: we should report internal latency here */
        *((pa_usec_t*)data) = 0;

        /* don't return, the default handler will add in the extra latency
         * added by the resampler
         */
        break;
    }

    return pa_sink_input_process_msg(o, code, data, offset, chunk);
}

static int pop_cb(pa_sink_input* i, size_t length, pa_memchunk* chunk) {
    pa_sink_input_assert_ref(i);

    struct roc_sink_input_userdata* u = i->userdata;
    pa_assert(u);

    /* ensure that all chunk fields are set to zero */
    pa_memchunk_reset(chunk);

    /* allocate memblock */
    chunk->memblock = pa_memblock_new(u->module->core->mempool, length);

    /* start writing memblock */
    char* buf = pa_memblock_acquire(chunk->memblock);

    /* prepare audio frame */
    roc_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.samples = buf;
    frame.samples_size = length;

    /* read samples from file to memblock */
    int ret = roc_receiver_read(u->receiver, &frame);

    /* finish writing memblock */
    pa_memblock_release(chunk->memblock);

    /* handle eof and error */
    if (ret != 0) {
        pa_module_unload_request(u->module, true);
        return -1;
    }

    /* setup chunk boundaries */
    chunk->index = 0;
    chunk->length = frame.samples_size;

    return 0;
}

static void rewind_cb(pa_sink_input* i, size_t nbytes) {
    pa_sink_input_assert_ref(i);

    struct roc_sink_input_userdata* u = i->userdata;
    pa_assert(u);

    (void)nbytes;
}

static void kill_cb(pa_sink_input* i) {
    pa_sink_input_assert_ref(i);

    struct roc_sink_input_userdata* u = i->userdata;
    pa_assert(u);

    pa_module_unload_request(u->module, true);

    pa_sink_input_unlink(u->sink_input);
    pa_sink_input_unref(u->sink_input);
    u->sink_input = NULL;
}

void pa__done(pa_module*);

int pa__init(pa_module* m) {
    pa_assert(m);

    /* setup logs */
    roc_log_set_level(ROC_LOG_DEBUG);
    roc_log_set_handler(rocpulse_log_handler, NULL);

    /* get module arguments (key-value list passed to load-module) */
    pa_modargs* args;
    if (!(args = pa_modargs_new(m->argument, roc_sink_input_modargs))) {
        pa_log("failed to parse module arguments");
        goto error;
    }

    /* get sink from arguments */
    pa_sink* sink = pa_namereg_get(m->core, pa_modargs_get_value(args, "sink", NULL),
                                   PA_NAMEREG_SINK);
    if (!sink) {
        pa_log("sink does not exist");
        goto error;
    }

    /* create and initialize module-specific data */
    struct roc_sink_input_userdata* u = pa_xnew0(struct roc_sink_input_userdata, 1);
    pa_assert(u);
    m->userdata = u;

    u->module = m;

    /* roc context */
    roc_context_config context_config;
    memset(&context_config, 0, sizeof(context_config));

    if (roc_context_open(&context_config, &u->context) < 0) {
        pa_log("can't create roc context");
        goto error;
    }

    /* roc receiver config */
    roc_receiver_config receiver_config;
    memset(&receiver_config, 0, sizeof(receiver_config));

    receiver_config.frame_encoding.rate = 44100;
    receiver_config.frame_encoding.channels = ROC_CHANNEL_LAYOUT_STEREO;
    receiver_config.frame_encoding.format = ROC_FORMAT_PCM_FLOAT32;

    roc_packet_encoding receiver_packet_encoding = 0;

    if (rocpulse_parse_packet_encoding(&receiver_packet_encoding, args,
                                       "packet_encoding_id")
        < 0) {
        goto error;
    }

    if (receiver_packet_encoding != 0) {
        roc_media_encoding encoding;

        if (rocpulse_parse_media_encoding(&encoding, args, "packet_encoding_rate",
                                          "packet_encoding_format",
                                          "packet_encoding_chans")
            < 0) {
            goto error;
        }

        if (roc_context_register_encoding(u->context, receiver_packet_encoding, &encoding)
            < 0) {
            pa_log("can't register packet encoding");
            goto error;
        }

        /* propagate packet encoding to sink input */
        receiver_config.frame_encoding.rate = encoding.rate;
        receiver_config.frame_encoding.channels = encoding.channels;
    }

    roc_fec_encoding receiver_fec_encoding = ROC_FEC_ENCODING_DEFAULT;

    if (rocpulse_parse_fec_encoding(&receiver_fec_encoding, args, "fec_encoding") < 0) {
        goto error;
    }

    if (rocpulse_parse_resampler_backend(&receiver_config.resampler_backend, args,
                                         "resampler_backend")
        < 0) {
        goto error;
    }

    if (rocpulse_parse_resampler_profile(&receiver_config.resampler_profile, args,
                                         "resampler_profile")
        < 0) {
        goto error;
    }

#if ROC_VERSION >= ROC_VERSION_CODE(0, 4, 0)
    if (rocpulse_parse_latency_tuner_backend(&receiver_config.latency_tuner_backend, args,
                                             "latency_backend")
        < 0) {
        goto error;
    }

    if (rocpulse_parse_latency_tuner_profile(&receiver_config.latency_tuner_profile, args,
                                             "latency_profile")
        < 0) {
        goto error;
    }

    if (rocpulse_parse_duration_msec_ll(&receiver_config.min_latency, 1, args,
                                        "min_latency_msec", "0")
        < 0) {
        goto error;
    }

    if (rocpulse_parse_duration_msec_ll(&receiver_config.max_latency, 1, args,
                                        "max_latency_msec", "0")
        < 0) {
        goto error;
    }
#endif // ROC_VERSION >= ROC_VERSION_CODE(0, 4, 0)

    if (rocpulse_parse_duration_msec_ul(&receiver_config.target_latency, 1, args,
                                        "target_latency_msec", "0")
        < 0) {
        goto error;
    }

    if (rocpulse_parse_duration_msec_ll(&receiver_config.no_playback_timeout, 1, args,
                                        "no_play_timeout_msec", "0")
        < 0) {
        goto error;
    }

    if (rocpulse_parse_duration_msec_ll(&receiver_config.choppy_playback_timeout, 1, args,
                                        "choppy_play_timeout_msec", "0")
        < 0) {
        goto error;
    }

    /* roc receiver endpoints */
    if (rocpulse_parse_endpoint(&u->local_source_endp, ROC_INTERFACE_AUDIO_SOURCE,
                                receiver_fec_encoding, args, "local_ip",
                                ROCPULSE_DEFAULT_IP, "local_source_port",
                                ROCPULSE_DEFAULT_SOURCE_PORT)
        < 0) {
        goto error;
    }

    if (receiver_fec_encoding != ROC_FEC_ENCODING_DISABLE) {
        if (rocpulse_parse_endpoint(&u->local_repair_endp, ROC_INTERFACE_AUDIO_REPAIR,
                                    receiver_fec_encoding, args, "local_ip",
                                    ROCPULSE_DEFAULT_IP, "local_repair_port",
                                    ROCPULSE_DEFAULT_REPAIR_PORT)
            < 0) {
            goto error;
        }
    }

    if (rocpulse_parse_endpoint(&u->local_control_endp, ROC_INTERFACE_AUDIO_CONTROL,
                                receiver_fec_encoding, args, "local_ip",
                                ROCPULSE_DEFAULT_IP, "local_control_port",
                                ROCPULSE_DEFAULT_CONTROL_PORT)
        < 0) {
        goto error;
    }

    /* open and bind */
    if (roc_receiver_open(u->context, &receiver_config, &u->receiver) < 0) {
        pa_log("can't create roc receiver");
        goto error;
    }

    if (roc_receiver_bind(u->receiver, ROC_SLOT_DEFAULT, ROC_INTERFACE_AUDIO_SOURCE,
                          u->local_source_endp)
        != 0) {
        pa_log("can't bind roc receiver to local address");
        goto error;
    }

    if (u->local_repair_endp) {
        if (roc_receiver_bind(u->receiver, ROC_SLOT_DEFAULT, ROC_INTERFACE_AUDIO_REPAIR,
                              u->local_repair_endp)
            != 0) {
            pa_log("can't bind roc receiver to local address");
            goto error;
        }
    }

    if (roc_receiver_bind(u->receiver, ROC_SLOT_DEFAULT, ROC_INTERFACE_AUDIO_CONTROL,
                          u->local_control_endp)
        != 0) {
        pa_log("can't bind roc receiver to local address");
        goto error;
    }

    /* prepare sample spec and channel map used for sink input */
    pa_sample_spec sample_spec;
    pa_channel_map channel_map;

    if (rocpulse_extract_encoding(&receiver_config.frame_encoding, &sample_spec,
                                  &channel_map)
        < 0) {
        goto error;
    }

    /* create and initialize sink input */
    pa_sink_input_new_data data;
    pa_sink_input_new_data_init(&data);
#if PA_CHECK_VERSION(11, 99, 0)
    pa_sink_input_new_data_set_sink(&data, sink, false, false);
#else
    pa_sink_input_new_data_set_sink(&data, sink, false);
#endif
    data.driver = "roc-sink-input";
    data.module = u->module;
    pa_sink_input_new_data_set_sample_spec(&data, &sample_spec);
    pa_sink_input_new_data_set_channel_map(&data, &channel_map);

    pa_proplist_sets(data.proplist, PA_PROP_MEDIA_NAME, "Roc Receiver");

    if (pa_modargs_get_proplist(args, "sink_input_properties", data.proplist,
                                PA_UPDATE_REPLACE)
        < 0) {
        pa_log("invalid sink input properties");
        pa_sink_input_new_data_done(&data);
        goto error;
    }

    pa_sink_input_new(&u->sink_input, u->module->core, &data);
    pa_sink_input_new_data_done(&data);

    if (!u->sink_input) {
        pa_log("failed to create sink input");
        goto error;
    }

    u->sink_input->userdata = u;
    u->sink_input->parent.process_msg = process_message;
    u->sink_input->pop = pop_cb;
    u->sink_input->process_rewind = rewind_cb;
    u->sink_input->kill = kill_cb;
    pa_sink_input_put(u->sink_input);

    unsigned long long playback_latency_us = 0;
    if (rocpulse_parse_duration_msec_ul(&playback_latency_us, 1000, args,
                                        "io_latency_msec", "40")
        < 0) {
        goto error;
    }
    pa_sink_input_set_requested_latency(u->sink_input, playback_latency_us);

    pa_modargs_free(args);

    return 0;

error:
    if (args) {
        pa_modargs_free(args);
    }
    pa__done(m);

    return -1;
}

void pa__done(pa_module* m) {
    pa_assert(m);

    struct roc_sink_input_userdata* u = m->userdata;
    if (!u) {
        return;
    }

    if (u->sink_input) {
        pa_sink_input_unlink(u->sink_input);
        pa_sink_input_unref(u->sink_input);
    }

    if (u->receiver) {
        if (roc_receiver_close(u->receiver) != 0) {
            pa_log("failed to close roc receiver");
        }
    }

    if (u->context) {
        if (roc_context_close(u->context) != 0) {
            pa_log("failed to close roc context");
        }
    }

    if (u->local_source_endp) {
        if (roc_endpoint_deallocate(u->local_source_endp) != 0) {
            pa_log("failed to deallocate roc endpoint");
        }
    }

    if (u->local_repair_endp) {
        if (roc_endpoint_deallocate(u->local_repair_endp) != 0) {
            pa_log("failed to deallocate roc endpoint");
        }
    }

    if (u->local_control_endp) {
        if (roc_endpoint_deallocate(u->local_control_endp) != 0) {
            pa_log("failed to deallocate roc endpoint");
        }
    }

    pa_xfree(u);
}

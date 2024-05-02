/*
 * This file is part of Roc PulseAudio integration.
 *
 * Copyright (c) Roc Streaming authors
 *
 * Licensed under GNU Lesser General Public License 2.1 or any later version.
 */

#pragma once

/* config.h from pulseaudio directory (generated after ./configure) */
#include <config.h>

/* private pulseaudio headers */
#include <pulsecore/modargs.h>

/* roc headers */
#include <roc/config.h>
#include <roc/endpoint.h>
#include <roc/log.h>
#include <roc/version.h>

#define ROCPULSE_DEFAULT_IP "0.0.0.0"

#define ROCPULSE_DEFAULT_SOURCE_PORT "10001"
#define ROCPULSE_DEFAULT_REPAIR_PORT "10002"
#define ROCPULSE_DEFAULT_CONTROL_PORT "10003"

void rocpulse_log_handler(const roc_log_message* message, void* argument);

int rocpulse_parse_endpoint(roc_endpoint** endp,
                            roc_interface iface,
                            roc_fec_encoding fec_encoding,
                            pa_modargs* args,
                            const char* ip_arg,
                            const char* default_ip_arg,
                            const char* port_arg,
                            const char* default_port_arg);

int rocpulse_parse_uint(unsigned int* out,
                        pa_modargs* args,
                        const char* arg_name,
                        const char* arg_default);

int rocpulse_parse_duration_msec_ul(unsigned long long* out,
                                    unsigned long out_base,
                                    pa_modargs* args,
                                    const char* arg_name,
                                    const char* arg_default);

int rocpulse_parse_duration_msec_ll(long long* out,
                                    unsigned long out_base,
                                    pa_modargs* args,
                                    const char* arg_name,
                                    const char* arg_default);

int rocpulse_parse_packet_encoding(roc_packet_encoding* out,
                                   pa_modargs* args,
                                   const char* arg_name);

int rocpulse_parse_media_encoding(roc_media_encoding* out,
                                  pa_modargs* args,
                                  const char* rate_arg_name,
                                  const char* format_arg_name,
                                  const char* chans_arg_name);

int rocpulse_parse_fec_encoding(roc_fec_encoding* out,
                                pa_modargs* args,
                                const char* arg_name);

#if ROC_VERSION >= ROC_VERSION_CODE(0, 4, 0)
int rocpulse_parse_latency_tuner_backend(roc_latency_tuner_backend* out,
                                         pa_modargs* args,
                                         const char* arg_name);

int rocpulse_parse_latency_tuner_profile(roc_latency_tuner_profile* out,
                                         pa_modargs* args,
                                         const char* arg_name);
#endif // ROC_VERSION >= ROC_VERSION_CODE(0, 4, 0)

int rocpulse_parse_resampler_backend(roc_resampler_backend* out,
                                     pa_modargs* args,
                                     const char* arg_name);

int rocpulse_parse_resampler_profile(roc_resampler_profile* out,
                                     pa_modargs* args,
                                     const char* arg_name);

int rocpulse_extract_encoding(const roc_media_encoding* src_encoding,
                              pa_sample_spec* dst_sample_spec,
                              pa_channel_map* dst_channel_map);

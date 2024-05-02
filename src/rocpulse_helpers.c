/*
 * This file is part of Roc PulseAudio integration.
 *
 * Copyright (c) Roc Streaming authors
 *
 * Licensed under GNU Lesser General Public License 2.1 or any later version.
 */

/* system headers */
#include <arpa/inet.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>

/* local headers */
#include "rocpulse_helpers.h"

void rocpulse_log_handler(const roc_log_message* message, void* argument) {
    (void)argument;

    switch (message->level) {
    case ROC_LOG_ERROR:
        pa_log_level_meta(PA_LOG_ERROR, message->module, -1, NULL, "%s", message->text);
        return;

    case ROC_LOG_INFO:
        pa_log_level_meta(PA_LOG_INFO, message->module, -1, NULL, "%s", message->text);
        return;

    default:
        pa_log_level_meta(PA_LOG_DEBUG, message->module, -1, NULL, "%s", message->text);
        return;
    }
}

int rocpulse_parse_endpoint(roc_endpoint** endp,
                            roc_interface iface,
                            roc_fec_encoding fec_encoding,
                            pa_modargs* args,
                            const char* ip_arg,
                            const char* default_ip_arg,
                            const char* port_arg,
                            const char* default_port_arg) {
    if (roc_endpoint_allocate(endp) != 0) {
        pa_log("can't allocate endpoint");
        return -1;
    }

    roc_protocol proto = 0;

    switch (iface) {
    case ROC_INTERFACE_AUDIO_SOURCE:
        switch (fec_encoding) {
        case ROC_FEC_ENCODING_DISABLE:
            proto = ROC_PROTO_RTP;
            break;
        case ROC_FEC_ENCODING_DEFAULT:
        case ROC_FEC_ENCODING_RS8M:
            proto = ROC_PROTO_RTP_RS8M_SOURCE;
            break;
        case ROC_FEC_ENCODING_LDPC_STAIRCASE:
            proto = ROC_PROTO_RTP_LDPC_SOURCE;
            break;
        default:
            pa_log("can't select endpoint protocol");
            return -1;
        }
        break;

    case ROC_INTERFACE_AUDIO_REPAIR:
        switch (fec_encoding) {
        case ROC_FEC_ENCODING_DISABLE:
            pa_log("can't select endpoint protocol");
            return -1;
        case ROC_FEC_ENCODING_DEFAULT:
        case ROC_FEC_ENCODING_RS8M:
            proto = ROC_PROTO_RS8M_REPAIR;
            break;
        case ROC_FEC_ENCODING_LDPC_STAIRCASE:
            proto = ROC_PROTO_LDPC_REPAIR;
            break;
        default:
            pa_log("can't select endpoint protocol");
            return -1;
        }
        break;

    case ROC_INTERFACE_AUDIO_CONTROL:
        proto = ROC_PROTO_RTCP;
        break;

    default:
        pa_log("can't select endpoint protocol");
        return -1;
    }

    if (roc_endpoint_set_protocol(*endp, proto) != 0) {
        pa_log("can't set endpoint protocol");
        return -1;
    }

    const char* ip_str;
    if (ip_arg) {
        ip_str = pa_modargs_get_value(args, ip_arg, default_ip_arg);
    } else {
        ip_str = default_ip_arg;
    }

    if (!*ip_str) {
        ip_str = "0.0.0.0";
    }

    if (roc_endpoint_set_host(*endp, ip_str) != 0) {
        pa_log("can't set endpoint host");
        return -1;
    }

    const char* port_str;
    if (port_arg) {
        port_str = pa_modargs_get_value(args, port_arg, default_port_arg);
    } else {
        port_str = default_port_arg;
    }

    char* end = NULL;
    long port_num = strtol(port_str, &end, 10);
    if (port_num == LONG_MIN || port_num == LONG_MAX || !end || *end) {
        pa_log("invalid %s: %s", port_arg, port_str);
        return -1;
    }

    if (roc_endpoint_set_port(*endp, (int)port_num) != 0) {
        pa_log("can't set endpoint port");
        return -1;
    }

    return 0;
}

int rocpulse_parse_uint(unsigned int* out,
                        pa_modargs* args,
                        const char* arg_name,
                        const char* arg_default) {
    const char* str = pa_modargs_get_value(args, arg_name, arg_default);

    char* end = NULL;
    long num = strtol(str, &end, 10);
    if (num == LONG_MIN || num == LONG_MAX || !end || *end) {
        pa_log("invalid %s: not a number: %s", arg_name, str);
        return -1;
    }

    if (num < 0 || (num > 0 && (unsigned long)num > (unsigned long)UINT_MAX)) {
        pa_log("invalid %s: out of range: %s", arg_name, str);
        return -1;
    }

    *out = (unsigned int)num;
    return 0;
}

int rocpulse_parse_duration_msec_ul(unsigned long long* out,
                                    unsigned long out_base,
                                    pa_modargs* args,
                                    const char* arg_name,
                                    const char* arg_default) {
    const char* str = pa_modargs_get_value(args, arg_name, arg_default);

    char* end = NULL;
    double num = strtod(str, &end);
    if (num == DBL_MIN || num == HUGE_VAL || !end || *end) {
        pa_log("invalid %s: not a number: %s", arg_name, str);
        return -1;
    }

    if (num < 0) {
        pa_log("invalid %s: should not be negative: %s", arg_name, str);
        return -1;
    }

    *out = (unsigned long long)(num / out_base * 1000000);
    return 0;
}

int rocpulse_parse_duration_msec_ll(long long* out,
                                    unsigned long out_base,
                                    pa_modargs* args,
                                    const char* arg_name,
                                    const char* arg_default) {
    const char* str = pa_modargs_get_value(args, arg_name, arg_default);

    char* end = NULL;
    double num = strtod(str, &end);
    if (num == DBL_MIN || num == HUGE_VAL || !end || *end) {
        pa_log("invalid %s: not a number: %s", arg_name, str);
        return -1;
    }

    *out = (unsigned long long)(num / out_base * 1000000);
    return 0;
}

int rocpulse_parse_packet_encoding(roc_packet_encoding* out,
                                   pa_modargs* args,
                                   const char* arg_name) {
    const char* str = pa_modargs_get_value(args, arg_name, "");

    if (!str || !*str) {
        *out = 0;
        return 0;
    } else {
        char* end = NULL;
        long num = strtol(str, &end, 10);
        if (num == LONG_MIN || num == LONG_MAX || !end || *end) {
            pa_log("invalid %s: not a number: %s", arg_name, str);
            return -1;
        }

        if (num <= 0 || num >= 256) {
            pa_log("invalid %s: out of range: %s", arg_name, str);
            return -1;
        }

        *out = (int)num;
        return 0;
    }
}

int rocpulse_parse_media_encoding(roc_media_encoding* out,
                                  pa_modargs* args,
                                  const char* rate_arg_name,
                                  const char* format_arg_name,
                                  const char* chans_arg_name) {
    memset(out, 0, sizeof(*out));

    /* rate */
    if (rocpulse_parse_uint(&out->rate, args, rate_arg_name, "44100") < 0) {
        return -1;
    }

    /* channels */
    const char* chans = pa_modargs_get_value(args, chans_arg_name, "stereo");
    if (!chans || !*chans || strcmp(chans, "stereo") == 0) {
        out->channels = ROC_CHANNEL_LAYOUT_STEREO;
    } else if (strcmp(chans, "mono")) {
        out->channels = ROC_CHANNEL_LAYOUT_MONO;
    } else {
        pa_log("invalid %s: %s", chans_arg_name, chans);
        return -1;
    }

    /* format */
    const char* format = pa_modargs_get_value(args, format_arg_name, "s16");
    if (!format || !*format || strcmp(format, "s16") == 0) {
        // TODO: use proper format when roc API is fixed
        out->format = ROC_FORMAT_PCM_FLOAT32;
    } else {
        pa_log("invalid %s: %s", format_arg_name, format);
        return -1;
    }

    return 0;
}

int rocpulse_parse_fec_encoding(roc_fec_encoding* out,
                                pa_modargs* args,
                                const char* arg_name) {
    const char* str = pa_modargs_get_value(args, arg_name, "");

    if (!str || !*str || strcmp(str, "default") == 0) {
        *out = ROC_FEC_ENCODING_DEFAULT;
        return 0;
    } else if (strcmp(str, "disable") == 0) {
        *out = ROC_FEC_ENCODING_DISABLE;
        return 0;
    } else if (strcmp(str, "rs8m") == 0) {
        *out = ROC_FEC_ENCODING_RS8M;
        return 0;
    } else if (strcmp(str, "ldpc") == 0) {
        *out = ROC_FEC_ENCODING_LDPC_STAIRCASE;
        return 0;
    } else {
        pa_log("invalid %s: %s", arg_name, str);
        return -1;
    }
}

#if ROC_VERSION >= ROC_VERSION_CODE(0, 4, 0)
int rocpulse_parse_latency_tuner_backend(roc_latency_tuner_backend* out,
                                         pa_modargs* args,
                                         const char* arg_name) {
    const char* str = pa_modargs_get_value(args, arg_name, "");

    if (!str || !*str || strcmp(str, "default") == 0) {
        *out = ROC_LATENCY_TUNER_BACKEND_DEFAULT;
        return 0;
    } else if (strcmp(str, "niq") == 0) {
        *out = ROC_LATENCY_TUNER_BACKEND_NIQ;
        return 0;
    } else {
        pa_log("invalid %s: %s", arg_name, str);
        return -1;
    }
}

int rocpulse_parse_latency_tuner_profile(roc_latency_tuner_profile* out,
                                         pa_modargs* args,
                                         const char* arg_name) {
    const char* str = pa_modargs_get_value(args, arg_name, "");

    if (!str || !*str || strcmp(str, "default") == 0) {
        *out = ROC_LATENCY_TUNER_PROFILE_DEFAULT;
        return 0;
    } else if (strcmp(str, "intact") == 0) {
        *out = ROC_LATENCY_TUNER_PROFILE_INTACT;
        return 0;
    } else if (strcmp(str, "responsive") == 0) {
        *out = ROC_LATENCY_TUNER_PROFILE_RESPONSIVE;
        return 0;
    } else if (strcmp(str, "gradual") == 0) {
        *out = ROC_LATENCY_TUNER_PROFILE_GRADUAL;
        return 0;
    } else {
        pa_log("invalid %s: %s", arg_name, str);
        return -1;
    }
}
#endif // ROC_VERSION >= ROC_VERSION_CODE(0, 4, 0)

int rocpulse_parse_resampler_backend(roc_resampler_backend* out,
                                     pa_modargs* args,
                                     const char* arg_name) {
    const char* str = pa_modargs_get_value(args, arg_name, "");

    if (!str || !*str || strcmp(str, "default") == 0) {
        *out = ROC_RESAMPLER_BACKEND_DEFAULT;
        return 0;
    } else if (strcmp(str, "builtin") == 0) {
        *out = ROC_RESAMPLER_BACKEND_BUILTIN;
        return 0;
    } else if (strcmp(str, "speex") == 0) {
        *out = ROC_RESAMPLER_BACKEND_SPEEX;
        return 0;
    } else if (strcmp(str, "speexdec") == 0) {
        *out = ROC_RESAMPLER_BACKEND_SPEEXDEC;
        return 0;
    } else {
        pa_log("invalid %s: %s", arg_name, str);
        return -1;
    }
}

int rocpulse_parse_resampler_profile(roc_resampler_profile* out,
                                     pa_modargs* args,
                                     const char* arg_name) {
    const char* str = pa_modargs_get_value(args, arg_name, "");

    if (!str || !*str || strcmp(str, "default") == 0) {
        *out = ROC_RESAMPLER_PROFILE_DEFAULT;
        return 0;
    } else if (strcmp(str, "high") == 0) {
        *out = ROC_RESAMPLER_PROFILE_HIGH;
        return 0;
    } else if (strcmp(str, "medium") == 0) {
        *out = ROC_RESAMPLER_PROFILE_MEDIUM;
        return 0;
    } else if (strcmp(str, "low") == 0) {
        *out = ROC_RESAMPLER_PROFILE_LOW;
        return 0;
    } else {
        pa_log("invalid %s: %s", arg_name, str);
        return -1;
    }
}

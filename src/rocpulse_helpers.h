/*
 * This file is part of Roc PulseAudio integration.
 *
 * Copyright (c) 2017 Roc Streaming authors
 *
 * Licensed under GNU Lesser General Public License 2.1 or any later version.
 */

/* config.h from pulseaudio directory (generated after ./configure) */
#include <config.h>

/* private pulseaudio headers */
#include <pulsecore/modargs.h>

/* roc headers */
#include <roc/config.h>
#include <roc/endpoint.h>
#include <roc/log.h>

#define ROCPULSE_DEFAULT_IP "0.0.0.0"

#define ROCPULSE_DEFAULT_SOURCE_PORT "10001"
#define ROCPULSE_DEFAULT_REPAIR_PORT "10002"

#define ROCPULSE_DEFAULT_SOURCE_PROTO ROC_PROTO_RTP_RS8M_SOURCE
#define ROCPULSE_DEFAULT_REPAIR_PROTO ROC_PROTO_RS8M_REPAIR

void rocpulse_log_handler(roc_log_level level, const char* module, const char* message);

int rocpulse_parse_endpoint(roc_endpoint** endp,
                            roc_protocol proto,
                            pa_modargs* args,
                            const char* ip_arg,
                            const char* default_ip_arg,
                            const char* port_arg,
                            const char* default_port_arg);

int rocpulse_parse_duration_msec(unsigned long long* out,
                                 unsigned long out_base,
                                 pa_modargs* args,
                                 const char* arg_name,
                                 const char* arg_default);

int rocpulse_parse_resampler_profile(roc_resampler_profile* out,
                                     pa_modargs* args,
                                     const char* arg_name);

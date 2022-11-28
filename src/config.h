/*
 * This file is part of Roc PulseAudio integration.
 *
 * Copyright (c) 2021 Roc Streaming authors
 *
 * Licensed under GNU Lesser General Public License 2.1 or any later version.
 */

#define STRINGIZE(s) _STRINGIZE(s)
#define _STRINGIZE(s) #s

#define PACKAGE
#define PACKAGE_VERSION STRINGIZE(ROC_PULSEAUDIO_VERSION)

#define HAVE_ATOMIC_BUILTINS

#include <sys/socket.h>

#! /bin/bash
set -euxo pipefail

sudo apt-get install -y \
    pkg-config \
    libtool \
    intltool \
    m4 \
    autoconf \
    automake \
    make \
    cmake \
    scons \
    python \
    pulseaudio

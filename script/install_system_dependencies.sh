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
    python

sudo apt-get install -y \
    scons \
    ragel \
    gengetopt \
    pulseaudio \
    libsndfile1-dev

#! /bin/bash
set -euxo pipefail

sudo apt-get install -y \
    gcc g++ \
    make \
    libtool intltool m4 autoconf automake \
    meson libsndfile-dev \
    cmake \
    scons \
    git \
    wget \
    python3 \
    pulseaudio

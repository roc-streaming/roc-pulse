#! /bin/bash
set -euxo pipefail

sudo apt-get install -y \
    gcc g++ \
    make \
    libtool intltool m4 autoconf automake \
    cmake \
    scons \
    git \
    wget \
    python \
    pulseaudio

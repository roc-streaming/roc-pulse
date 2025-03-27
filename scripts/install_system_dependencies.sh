#! /bin/bash
set -euxo pipefail

sudo apt-get update

sudo apt-get install -y \
    gcc g++ \
    make \
    libtool intltool m4 autoconf automake \
    meson libltdl-dev libsndfile-dev \
    cmake \
    scons \
    git \
    wget \
    python3 \
    pulseaudio

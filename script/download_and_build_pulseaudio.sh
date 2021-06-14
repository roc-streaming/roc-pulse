#! /bin/bash
set -euxo pipefail

source_dir="$1"
build_dir="$2"

ver="12.2"

mkdir -p "$source_dir"
mkdir -p "$build_dir"

wget -qO - "https://freedesktop.org/software/pulseaudio/releases/pulseaudio-$ver.tar.gz" \
     | tar --strip-components=1 -C "$source_dir" -xzf -

source_dir="$(cd "$source_dir" && pwd)"

cd "$build_dir"

"$source_dir"/configure \
             --enable-shared \
             --disable-static \
             --disable-tests \
             --disable-manpages \
             --disable-orc \
             --disable-webrtc-aec \
             --disable-openssl \
             --disable-neon-opt \
             --without-caps

make -j2

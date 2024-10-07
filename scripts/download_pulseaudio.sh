#! /bin/bash
set -euxo pipefail

dir="$1"
ver="$2"

mkdir -p "$dir"

wget -qO - "https://distributions.freedesktop.org/software/pulseaudio/releases/pulseaudio-$ver.tar.gz" \
     | tar --strip-components=1 -C "$dir" -xzf -

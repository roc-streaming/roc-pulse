#! /bin/bash
set -euxo pipefail

dir="$1"
ver="$2"

mkdir -p "$dir"

wget -qO - "ftp://ftp.gnu.org/gnu/libtool/libtool-$ver.tar.gz" \
     | tar --strip-components=1 -C "$dir" -xzf -

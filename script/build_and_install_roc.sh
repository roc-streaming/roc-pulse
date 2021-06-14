#! /bin/bash
as_root() {
    if which sudo >/dev/null 2>&1
    then
        sudo "$@"
    else
        "$@"
    fi
}

set -euxo pipefail

source_dir="$1"
install_dir="${2:-}"
host="${3:-}"

scons_args=(
    -Q
    -C "$source_dir"
    --disable-tools
    --disable-libunwind
    --disable-sox
    --disable-pulseaudio
    --build-3rdparty=all
)

if [ ! -z "$install_dir" ]
then
    as_root mkdir -p "$install_dir"
    scons_args+=( --prefix="$(cd "$install_dir" && pwd)" )
fi

if [ ! -z "$host" ]
then
    scons_args+=( --host="$host" )
fi

scons "${scons_args[@]}"

as_root scons "${scons_args[@]}" install

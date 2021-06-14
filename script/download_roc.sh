#! /bin/bash
set -euxo pipefail

dir="$1"
branch="develop"

git clone --recurse-submodules -b "$branch" \
    https://github.com/roc-streaming/roc-toolkit.git "$dir"

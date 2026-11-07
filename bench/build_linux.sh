#!/bin/bash
# Builds the Linux benchmark against one checkout of the engine.
#   bench/build_linux.sh <repo root> <out> [extra g++ flags]
# Use -DBENCH_OLD for checkouts from before MT2D_VAR stored its value inline, and -m32 for a 32 bit build.
set -e
R=$(cd "$1" && pwd); OUT=$2; shift 2
HERE=$(cd "$(dirname "$0")" && pwd)
E=$R/MT2D
. "$HERE/sources.inc"
B=$(mktemp -d); trap 'rm -rf "$B"' EXIT
# Some engine sources include headers with a different letter case (see tests/CMakeLists.txt).
mkdir -p "$B/shadow/MT2D"; ln -sf "$E/ObjectCore" "$B/shadow/MT2D/Objectcore"
g++ -O2 -std=gnu++11 -w -fpermissive -I"$R/tests/compat" -I"$R/tests/framework" -I"$B/shadow" -I"$R" "$@" \
    -Wl,--wrap=malloc,--wrap=realloc "$HERE/bench_linux.cpp" $SRC -o "$OUT"

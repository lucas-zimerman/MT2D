#!/bin/bash
# Builds the DOS benchmark with DJGPP (the toolchain from .github/workflows/djgpp.yml).
#   DJGPP=~/djgpp bench/build_dos.sh <repo root> <out.exe> [extra flags]
# Use -DBENCH_OLD for checkouts from before MT2D_VAR stored its value inline.
set -e
: "${DJGPP:?set DJGPP to the folder that contains bin/i586-pc-msdosdjgpp-g++}"
export PATH="$DJGPP/bin:$PATH"
R=$(cd "$1" && pwd); OUT=$2; shift 2
HERE=$(cd "$(dirname "$0")" && pwd)
E=$R/MT2D
. "$HERE/sources.inc"
B=$(mktemp -d); trap 'rm -rf "$B"' EXIT
mkdir -p "$B/shadow/MT2D"; ln -sf "$E/ObjectCore" "$B/shadow/MT2D/Objectcore"
i586-pc-msdosdjgpp-g++ -O2 -march=i586 -std=gnu++98 -w -fpermissive -I"$R/tests/compat" -I"$R/tests/framework" -I"$B/shadow" -I"$R" "$@" \
    -Wl,--wrap=malloc,--wrap=realloc "$HERE/bench_dos.cpp" $SRC -o "$OUT"

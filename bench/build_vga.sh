#!/bin/bash
# Builds the VGA renderer benchmark (bench_vga.c) with DJGPP against the renderer of a checkout.
#   DJGPP=~/djgpp bench/build_vga.sh <repo root> <out.exe> [extra flags, e.g. -DMSDOS_VGA_WAIT_RETRACE]
# The benchmark file always comes from this checkout, the renderer from <repo root>, so an old tree can be compared.
set -e
: "${DJGPP:?set DJGPP to the folder that contains bin/i586-pc-msdosdjgpp-g++}"
export PATH="$DJGPP/bin:$PATH"
R=$(cd "$1" && pwd); OUT=$2; shift 2
HERE=$(cd "$(dirname "$0")" && pwd)
B=$(mktemp -d); trap 'rm -rf "$B"' EXIT
CXXFLAGS="-O2 -march=i586 -std=gnu++98 -w -fpermissive -I$R -DMSDOS_USE_GRAPHICS"
CFLAGS="-O2 -march=i586 -std=gnu99 -w -I$R -DMSDOS_USE_GRAPHICS"
OBJS=
for f in "$R"/MT2D/_MSDOS/Render/*.c "$R"/MT2D/_MSDOS/Render/*.cpp; do
    [ -f "$f" ] || continue
    case "$(basename "$f")" in DtxtRend.cpp|M2Render.c) continue;; esac
    o="$B/$(basename "$f").o"
    case "$f" in *.c) i586-pc-msdosdjgpp-gcc $CFLAGS "$@" -c "$f" -o "$o";; *) i586-pc-msdosdjgpp-g++ $CXXFLAGS "$@" -c "$f" -o "$o";; esac
    OBJS="$OBJS $o"
done
i586-pc-msdosdjgpp-gcc $CFLAGS "$@" -c "$HERE/bench_vga.c" -o "$B/bench_vga.o"
i586-pc-msdosdjgpp-g++ -Wl,--wrap=malloc,--wrap=realloc "$B/bench_vga.o" $OBJS -o "$OUT"
i586-pc-msdosdjgpp-size "$OUT"

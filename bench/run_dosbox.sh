#!/bin/bash
# Runs every *.EXE of a folder, one after the other, inside a headless DOSBox set to a Pentium class CPU.
# The output of FOO.EXE is written to FOO.TXT in the same folder.
#   bench/run_dosbox.sh <folder with the .EXE files and CWSDPMI.EXE> [cycles, default 60000]
set -e
D=$(cd "$1" && pwd); CYCLES=${2:-60000}
[ -f "$D/CWSDPMI.EXE" ] || { echo "CWSDPMI.EXE (the DJGPP DPMI host) must be in $D, see bench/README.md" >&2; exit 1; }
CONF=$(mktemp); trap 'rm -f "$CONF"' EXIT
{
  echo "[sdl]"; echo "fullscreen=false"; echo "output=surface"
  echo "[dosbox]"; echo "memsize=31"
  echo "[cpu]"; echo "core=normal"; echo "cputype=pentium_slow"; echo "cycles=fixed $CYCLES"
  echo "[autoexec]"; echo "@echo off"; echo "mount c $D"; echo "c:"
  for f in "$D"/*.EXE; do n=$(basename "$f"); [ "$n" = CWSDPMI.EXE ] && continue; echo "$n > ${n%.EXE}.TXT"; done
  echo "exit"
} > "$CONF"
rm -f "$D"/*.TXT
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy dosbox -conf "$CONF" >/dev/null 2>&1
for f in "$D"/*.TXT; do echo "== $(basename "$f" .TXT)"; tr -d '\r' < "$f"; done

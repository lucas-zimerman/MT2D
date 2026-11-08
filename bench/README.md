# Benchmarks

Micro benchmarks for the var / object core (`MT2D_VAR`, `Object_Create`, `Cscript_*` math, ...). They exist to compare two
checkouts of the engine, e.g. `master` against a branch, not to give absolute numbers.

Both programs print `RESULT` lines: the time of each workload (best of N) and the number of `malloc` calls (counted with the
linker's `--wrap`). `bench_linux.cpp` also reports hardware instruction counts (`perf_event_open`) and heap bytes.
`bench_dos.cpp` is the same workloads at a smaller scale (DOS memory) timed with `uclock()`.

Checkouts that still use the old `MT2D_VAR` layout (`*(int*)var->Data`) need `-DBENCH_OLD`.

## Linux (x86 or `-m32`)

```sh
git worktree add /tmp/mt2d-old master          # the tree to compare against
bench/build_linux.sh /tmp/mt2d-old  /tmp/bench_old -DBENCH_OLD -m32
bench/build_linux.sh .              /tmp/bench_new -m32
taskset -c 3 /tmp/bench_old; taskset -c 3 /tmp/bench_new    # pin to a core for stable numbers
```

## MS-DOS (DJGPP in an emulated Pentium)

Needs the DJGPP toolchain the CI uses (see `.github/workflows/djgpp.yml`, `DJGPP_URL` and `DJGPP_SHA256`), DOSBox, and
`CWSDPMI.EXE` (the DPMI host DJGPP programs need, from `csdpmi7b.zip` on <https://www.delorie.com/pub/djgpp/current/v2misc/>;
it is not stored in this repo).

```sh
mkdir /tmp/dosrun && cp CWSDPMI.EXE /tmp/dosrun/
DJGPP=~/djgpp bench/build_dos.sh /tmp/mt2d-old /tmp/dosrun/OLD.EXE -DBENCH_OLD
DJGPP=~/djgpp bench/build_dos.sh .             /tmp/dosrun/NEW.EXE
bench/run_dosbox.sh /tmp/dosrun            # prints OLD then NEW, also saved as OLD.TXT / NEW.TXT
```

`run_dosbox.sh` runs DOSBox headless with `cputype=pentium_slow`, `core=normal` and a fixed cycle count (60000 by default, pass
another number as the second argument). Results are deterministic: the same binary gives the same numbers every run.

DOSBox executes a flat instruction rate and does not model caches or memory timing. It is good for comparing how much work two
builds do on a Pentium class CPU (DJGPP's real `malloc`/`free` run), not for predicting absolute times, and it understates
gains that come from fewer memory accesses.

## VGA renderer (MS-DOS, mode 13h)

`bench_vga.c` draws scripted scenes (idle text, changing text, aligned / unaligned / many sprites) through the real
renderer in VGA mode and prints, per scene, the time of one frame, a checksum of the 320x200 screen, `badpixels` (pixels
that differ from an independent per-pixel reference painter, 0 = the picture is right) and the `malloc`/`realloc` calls per
frame. The last line is the heap growth (`sbrk`) and the number of allocation calls. `size` of the `.exe` shows the static data.

```sh
git worktree add /tmp/mt2d-old master
DJGPP=~/djgpp bench/build_vga.sh /tmp/mt2d-old /tmp/dosrun/OLD.EXE            # the renderer of that checkout
DJGPP=~/djgpp bench/build_vga.sh .             /tmp/dosrun/NEW.EXE
bench/run_dosbox.sh /tmp/dosrun
```

Pass `-DMSDOS_USE_DOUBLE_BUFFER` to build an old checkout with its double buffer. DOSBox does not charge the slow video
memory bus, so removing writes to the video memory is understated.

### Spaceship game (`bench/ship`)

`ship/ship.c` is the main loop of `examples/graphics` (two full screen scrolling layers, planets, a rock, coins and the ship)
without the 30 ms delay, the beep and the keyboard: the ship follows a fixed path for 400 frames. It prints the time of a
frame, the allocation calls per frame, the heap growth, the screen checksum and `badpixels` (the last frame compared with the
reference painter). It is built like an example, then the exe is run with the `d` folder of `examples/graphics` next to it:

```sh
cmake -S bench/ship -B /tmp/ship-new -DCMAKE_TOOLCHAIN_FILE=$PWD/examples/cmake/djgpp.cmake -DMT2D_DOS_VGA=ON
cmake --build /tmp/ship-new          # writes bench/ship/bin/shipbench.exe (git ignored)
mkdir /tmp/shiprun && cp -r examples/graphics/d CWSDPMI.EXE /tmp/shiprun/ && cp bench/ship/bin/shipbench.exe /tmp/shiprun/NEW.EXE
bench/run_dosbox.sh /tmp/shiprun
```

For the old renderer copy `bench/ship` into a `master` worktree and build it there (`-DCMAKE_CXX_FLAGS=-DMSDOS_USE_DOUBLE_BUFFER` for the double buffer variant).

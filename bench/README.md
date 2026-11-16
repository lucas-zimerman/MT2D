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

Pass `-DMSDOS_USE_DOUBLE_BUFFER` to build an old checkout with its double buffer (it does nothing in a checkout with the new renderer). DOSBox does not charge the slow video
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

## Frame pacing (MS-DOS, `MT2D/MT2D_Frame.h`)

`bench_frame.c` measures what the DJGPP frame clock is built on, in DOSBox: `uclock()` cost and step, how long `delay(1/5/16)` really
takes, the cost of the `INT 2Fh/1680h` idle call and of a read of port `0x3DA`, the vertical retrace period (70.085 Hz in text mode
and in mode 13h), how well a "sleep until 3 ms before, then spin" wait lands on the retrace, and what `uclock()` reports once PIT
channel 0 is moved to a song rate (it is wrong while music plays). `bench_clock.c` compares the clocks that could replace it: a tick-counting timer ISR at 140 to 2800 Hz (accuracy and CPU cost of the interrupt,
both measured against the VGA retrace, 70.085 Hz and independent of the PIT) against `uclock()` with the PIT moved (it goes backwards), and it
probes how long `delay(n)` really takes. DOSBox does not charge the real cost of an interrupt, run it in 86Box (a 486 machine) or on hardware.

`bench_delay.c` measures the distribution of `delay(1/2/5/10)` and replays the frame wait; `bench_fwait.c` runs the real `MT2D_Frame_Wait_Next`
(build `MT2D/_MSDOS/M2Frame.c` with `-DMT2D_FRAME_FPS=35` and link `M2Music.c`, `M2Opl.c`, `Audio/MT2D_Music.c` and the container sources). Both use
integer math only, so they run on a 386 without an FPU (build with `-march=i386`).

`bench_fmusic.c` runs the real `MT2D_Frame_Wait_Next` while a looping silent song plays (needs an OPL chip, the DOSBox and 86Box SB16 have one):
without music, with music, and with the song started, replaced and stopped. The reference is the BIOS tick counter, because `uclock()` cannot time
a frame while a song plays. Link `M2Music.c` for the real run, or compile it with `-DMT2D_Dos_Music_Clock_Us=x` next to a stub that returns 0 to see the old
`uclock()` behaviour.

`bench_musicdelay.c` plays a silent looping song and, for 10 seconds of the RTC (hardware, independent of the PIT), spins, calls `delay(30)` or calls `delay(1)`,
and reports what the song timer counted. On the simulated 486 `delay()` made it lose 23 to 29% of the ticks (the BIOS wait loses timer interrupts), so the song ran slow.

86Box without touching your VM: copy the VM folder, convert its `.vhd` with `qemu-img convert -O raw`, `mcopy` (mtools, partition offset
`@@8704` for a disk made by 86Box) the `.EXE` files and `CWSDPMI.EXE` into it, add `CD \BENCH`, `CLOCK.EXE > CLOCK.TXT` and `ECHO DONE > DONE.TXT`
to its `AUTOEXEC.BAT`, point the copy's `hdd_01_fn` at the raw image and start `86Box -P <copy>`. Read `CLOCK.TXT` back with `mtype` once `DONE.TXT` exists.

The pacing math (drift, late frames) is tested on the host in `tests/test_frame.cpp` (MS-DOS) and `tests/test_frame_sdl.cpp` (SDL).

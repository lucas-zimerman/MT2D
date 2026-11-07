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

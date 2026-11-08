#!/usr/bin/env python3
"""Renders the songs of the playmusic example to IMF files for MS-DOS (OPL2: Adlib / Sound Blaster).

    python3 tools/make_imf.py

Same notes as make_assets.py (which renders the .ogg files of the SDL build). IMF is a list of
OPL register writes: 4 bytes each (register, value, 16 bit delay in ticks), 700 ticks per second,
preceded by the 16 bit length of the data. The melody uses one FM voice, the chords three more.
The files go to d/ (MS-DOS looks for d/<name>.imf when the program plays <name>.ogg).
The OPL helpers are shared with midi2imf.py. Only the standard library is needed.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_assets import ODE_TO_JOY, FUR_ELISE, CHORDS  # noqa: E402
from midi2imf import TICKS, fnum_block, hz, setup  # noqa: E402

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "d")


def note(events, ch, start, length, midi):
    fnum, block = fnum_block(hz(midi))
    on = max(0, int(round(start * TICKS)))
    off = max(on + 1, int(round((start + length) * TICKS)))
    events.append((on, 1, 0xA0 + ch, fnum & 0xFF))
    events.append((on, 2, 0xB0 + ch, 0x20 | (block << 2) | (fnum >> 8)))  # key on
    events.append((off, 0, 0xB0 + ch, (block << 2) | (fnum >> 8)))        # key off


def render(notes, unit_s, name):
    events = []
    setup(events, 4)  # voice 0 is the melody, 1..3 the chords
    bar_len = unit_s * (16 if name == "ode" else 24)
    reps = 4 if name == "ode" else 3
    t, bars = 0.0, {}
    for midi, length in notes:
        if midi:
            note(events, 0, t, length * unit_s * 0.92, midi)
            bars.setdefault(int(t / bar_len), []).append(midi % 12)
        t += length * unit_s
    for bar, pcs in bars.items():
        root, ivs = max(CHORDS, key=lambda c: sum(((m - c[0]) % 12) in c[1] for m in pcs))
        for k, iv in enumerate(ivs):
            for rep in range(reps):
                note(events, 1 + k, bar * bar_len + rep * bar_len / reps, bar_len / reps * 0.9, root + iv)
    events.sort()
    end = events[-1][0] + int(0.4 * TICKS)  # a short pause before the loop restarts
    data = bytearray()
    for i, (tick, _, reg, val) in enumerate(events):
        nxt = events[i + 1][0] if i + 1 < len(events) else end
        data += struct.pack("<BBH", reg, val, nxt - tick)
    assert len(data) < 65536
    path = os.path.join(OUT, name + ".imf")
    with open(path, "wb") as f:
        f.write(struct.pack("<H", len(data)))
        f.write(data)
    print("%s: %d bytes, %.1f s" % (path, len(data) + 2, end / float(TICKS)))


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    render(ODE_TO_JOY, 0.12, "ode")
    render(FUR_ELISE, 0.075, "elise")

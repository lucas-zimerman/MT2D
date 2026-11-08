#!/usr/bin/env python3
"""Renders d/desert.imf, the chase theme of the desert example for MS-DOS (OPL2: Adlib / Sound Blaster).

    python3 tools/make_imf.py

The same tune as assets/desert.ogg (make_music.py, an original tune, CC0): A minor, 150 BPM, 16 bars that
loop. Lead, bass and arpeggio use one FM voice each, the drums use the OPL2 rhythm mode (bass drum, snare,
hi-hat). The MS-DOS build plays it when title.txt says PlayMusic("desert.ogg") (the engine looks for
d/desert.imf). The OPL helpers come from playmusic/tools/midi2imf.py. Only the standard library is needed.
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "..", "playmusic", "tools"))
from make_music import BAR, LEAD, PROGRESSION, STEP  # noqa: E402
from midi2imf import DRUM_BITS, OPS, TICKS, drum_setup, fnum_block, hz, setup  # noqa: E402

OUT = os.path.join(HERE, "..", "d", "desert.imf")
LEAD_V, BASS_V, ARP_V = 0, 1, 2
BD, SD, HH = DRUM_BITS[36][0], DRUM_BITS[38][0], DRUM_BITS[42][0]


def main():
    events = []

    def add(tick, order, reg, val):
        events.append((tick, order, len(events), reg, val))  # the counter keeps the order of equal events

    plain = []
    setup(plain, 3)
    for tick, order, reg, val in plain:
        add(tick, order, reg, val)
    add(0, 1, 0x40 + OPS[BASS_V][1], 0x04)  # bass: loud
    add(0, 1, 0x40 + OPS[ARP_V][1], 0x1C)   # arpeggio: quiet
    drum_setup(add)
    rhythm = 0x20
    add(0, 0, 0xBD, rhythm)

    def note(voice, start, length, midi):
        fnum, block = fnum_block(hz(midi))
        on = int(round(start * TICKS))
        off = max(on + 1, int(round((start + length) * TICKS)))
        add(on, 1, 0xA0 + voice, fnum & 0xFF)
        add(on, 2, 0xB0 + voice, 0x20 | (block << 2) | (fnum >> 8))
        add(off, 0, 0xB0 + voice, (block << 2) | (fnum >> 8))

    # the drum register holds all the bits at once: build it from the list of hits sorted by time
    hits = []
    for b, (root, chord) in enumerate(PROGRESSION):
        t0 = b * BAR
        for s in range(16):
            t = t0 + s * STEP
            if s in (0, 6, 8, 10):
                hits.append((t, BD, 28))
            if s in (4, 12):
                hits.append((t, SD, 28))
            if s % 2 == 0 or s == 15:
                hits.append((t, HH, 100 if s in (14, 15) else 28))
            if s % 2 == 0:
                note(BASS_V, t, STEP * 1.8, root + (12 if s in (6, 14) else 0))
            midi = root + 24 + chord[(s % 3) if s % 4 != 3 else 1] + (12 if s % 8 >= 4 else 0)
            note(ARP_V, t, STEP * 0.9, midi)
        for step, midi, length in LEAD[b]:
            note(LEAD_V, t0 + step * STEP, length * STEP * 0.92, midi)
    # replay the drum hits in time order, keeping the bits that are still on
    changes = []
    for t, bit, ticks in hits:
        on = int(round(t * TICKS))
        changes.append((on, 1, bit))
        changes.append((on + ticks, 0, bit))
    changes.sort()
    for tick, kind, bit in changes:
        if kind == 1:
            if rhythm & bit:  # retrigger
                rhythm &= ~bit
                add(tick, 0, 0xBD, rhythm)
            rhythm |= bit
            add(tick, 2, 0xBD, rhythm)
        else:
            rhythm &= ~bit
            add(tick, 0, 0xBD, rhythm)
    events.sort()
    end = max(events[-1][0], int(round(len(PROGRESSION) * BAR * TICKS)))  # the loop restarts exactly on the bar
    body = bytearray()
    for i, (tick, _, _, reg, val) in enumerate(events):
        nxt = events[i + 1][0] if i + 1 < len(events) else end
        body += struct.pack("<BBH", reg, val, nxt - tick)
    assert len(body) < 65536
    with open(OUT, "wb") as f:
        f.write(struct.pack("<H", len(body)))
        f.write(body)
    print("%s: %d bytes, %.1f s, %d events" % (OUT, len(body) + 2, end / float(TICKS), len(events)))


if __name__ == "__main__":
    main()

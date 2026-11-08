#!/usr/bin/env python3
"""Converts standard MIDI files to one IMF song (OPL2: Adlib / Sound Blaster FM chip) for MS-DOS.

    python3 tools/midi2imf.py [--no-drums] out.imf in1.mid in2.mid ...

IMF is a list of OPL register writes: 4 bytes each (register, value, 16 bit delay in ticks), 700 ticks
per second. Songs over 64 KB are written as type 0 IMF (no length header), the others as type 1.
The songs play one after the other with a short pause. Notes get a free voice (the oldest one is
stolen when all are busy), the velocity sets the volume. Percussion (channel 10) uses the OPL2 rhythm
mode (bass drum, snare, tom, cymbal, hi-hat), which takes 3 voices away: 6 melodic voices + drums.
Songs without drums keep 9 voices (--no-drums turns the percussion off). Every other MIDI event is
ignored, all the notes use the same simple FM instrument. Only the standard library is needed.

d/badapple.imf was made from a MIDI file of "Bad Apple!!" (see ../NOTICE.md for the credits):
    python3 tools/midi2imf.py d/badapple.imf badapple.mid
"""
import os
import struct
import sys

TICKS = 700  # per second, MT2D_MUSIC_DEFAULT_RATE
GAP_S = 0.8
# operator offsets (modulator, carrier) of the 9 OPL2 voices
OPS = [(0x00, 0x03), (0x01, 0x04), (0x02, 0x05), (0x08, 0x0B), (0x09, 0x0C), (0x0A, 0x0D),
       (0x10, 0x13), (0x11, 0x14), (0x12, 0x15)]


def hz(midi):
    return 440.0 * 2 ** ((midi - 69) / 12.0)


def setup(events, voices):
    """One FM instrument on every voice, a bit softer on all but the first one."""
    events.append((0, 0, 0x01, 0x20))  # waveform select on
    events.append((0, 0, 0xBD, 0x00))
    for ch, (mod, car) in enumerate(OPS[:voices]):
        soft = ch > 0
        events.append((0, 0, 0x20 + mod, 0x01))
        events.append((0, 0, 0x40 + mod, 0x28 if soft else 0x1A))
        events.append((0, 0, 0x60 + mod, 0xF4))
        events.append((0, 0, 0x80 + mod, 0x74))
        events.append((0, 0, 0xE0 + mod, 0x00))
        events.append((0, 0, 0x20 + car, 0x01))
        events.append((0, 0, 0x40 + car, 0x14 if soft else 0x00))
        events.append((0, 0, 0x60 + car, 0xF4))
        events.append((0, 0, 0x80 + car, 0x74))
        events.append((0, 0, 0xE0 + car, 0x00))
        events.append((0, 0, 0xC0 + ch, 0x06))  # FM, some feedback


def fnum_block(freq):
    for block in range(8):
        fnum = int(round(freq * (1 << (20 - block)) / 49716.0))
        if fnum < 1024:
            return fnum, block
    return 1023, 7


def read_midi(path):
    """Returns ([(seconds, kind, channel, note, velocity)], total seconds). kind is 'on' or 'off'."""
    data = open(path, "rb").read()
    if data[:4] != b"MThd":
        raise ValueError(path + " is not a MIDI file")
    _fmt, ntracks, division = struct.unpack(">HHH", data[8:14])
    if division & 0x8000:
        raise ValueError("SMPTE time division is not supported")
    pos = 14
    raw, tempos = [], [(0, 500000)]
    for _ in range(ntracks):
        if data[pos:pos + 4] != b"MTrk":
            break
        length = struct.unpack(">I", data[pos + 4:pos + 8])[0]
        i, end, tick, status = pos + 8, pos + 8 + length, 0, 0
        pos = end
        while i < end:
            delta = 0
            while True:
                b = data[i]; i += 1
                delta = (delta << 7) | (b & 0x7F)
                if not b & 0x80:
                    break
            tick += delta
            if data[i] & 0x80:
                status = data[i]; i += 1
            if status == 0xFF:  # meta
                mtype = data[i]; i += 1
                mlen = 0
                while True:
                    b = data[i]; i += 1
                    mlen = (mlen << 7) | (b & 0x7F)
                    if not b & 0x80:
                        break
                if mtype == 0x51 and mlen == 3:
                    tempos.append((tick, int.from_bytes(data[i:i + 3], "big")))
                i += mlen
            elif status in (0xF0, 0xF7):  # sysex
                mlen = 0
                while True:
                    b = data[i]; i += 1
                    mlen = (mlen << 7) | (b & 0x7F)
                    if not b & 0x80:
                        break
                i += mlen
            else:
                kind, ch = status & 0xF0, status & 0x0F
                if kind in (0xC0, 0xD0):
                    i += 1
                else:
                    a, b = data[i], data[i + 1]
                    i += 2
                    if kind == 0x90 and b > 0:
                        raw.append((tick, "on", ch, a, b))
                    elif kind in (0x80, 0x90):
                        raw.append((tick, "off", ch, a, 0))
    tempos.sort(key=lambda t: t[0])  # stable: the file's tempo at tick 0 must stay after the default one

    def seconds(t):
        s, last_tick, tempo = 0.0, 0, 500000
        for tk, tp in tempos:
            if tk >= t:
                break
            s += (tk - last_tick) * tempo / 1e6 / division
            last_tick, tempo = tk, tp
        return s + (t - last_tick) * tempo / 1e6 / division

    notes = [(seconds(t), k, ch, n, v) for t, k, ch, n, v in raw]
    notes.sort(key=lambda e: (e[0], 0 if e[1] == "off" else 1))
    return notes, (notes[-1][0] if notes else 0.0)


# General MIDI percussion -> OPL2 rhythm mode: (bit of register 0xBD, operator whose level is the volume)
DRUM_BITS = {}
for _n in (35, 36):
    DRUM_BITS[_n] = (0x10, 0x53)   # bass drum (carrier of voice 6)
for _n in (37, 38, 39, 40):
    DRUM_BITS[_n] = (0x08, 0x54)   # snare
for _n in (41, 43, 45, 47, 48, 50):
    DRUM_BITS[_n] = (0x04, 0x52)   # tom
for _n in (49, 51, 52, 53, 55, 57, 59):
    DRUM_BITS[_n] = (0x02, 0x55)   # cymbal
for _n in (42, 44, 46):
    DRUM_BITS[_n] = (0x01, 0x51)   # hi-hat
DRUM_MIN_TICKS = 28  # a drum hit lasts at least 40 ms


def drum_setup(add):
    """Voices 6, 7 and 8 become the five rhythm instruments (tuned by ear, tweak freely)."""
    def operator(off, level, ar_dr, sl_rr):
        add(0, 0, 0x20 + off, 0x01)   # multiplier 1
        add(0, 0, 0x40 + off, level)
        add(0, 0, 0x60 + off, ar_dr)
        add(0, 0, 0x80 + off, sl_rr)
        add(0, 0, 0xE0 + off, 0x00)
    def voice(ch, fnum_low, block_fnum_high):
        add(0, 0, 0xC0 + ch, 0x00)
        add(0, 0, 0xA0 + ch, fnum_low)
        add(0, 0, 0xB0 + ch, block_fnum_high)  # no key on bit: the rhythm register plays them
    operator(0x10, 0x0D, 0xF9, 0x6A)  # bass drum, both operators of voice 6
    operator(0x13, 0x00, 0xF9, 0x6A)
    voice(6, 0x40, 0x06)
    operator(0x11, 0x08, 0xF8, 0x0F)  # hi-hat: modulator of voice 7
    operator(0x14, 0x04, 0xF8, 0x2A)  # snare: carrier of voice 7
    voice(7, 0xC0, 0x0A)
    operator(0x12, 0x08, 0xF8, 0x4A)  # tom: modulator of voice 8
    operator(0x15, 0x08, 0xF5, 0x57)  # cymbal: carrier of voice 8
    voice(8, 0x16, 0x0B)


def convert(paths, out, drums=True):
    songs = [read_midi(path) for path in paths]
    has_drums = drums and any(ch == 9 and kind == "on" for notes, _ in songs for _, kind, ch, _, _ in notes)
    voices = 6 if has_drums else 9   # rhythm mode takes the voices 6, 7 and 8
    events = []

    def add(tick, order, reg, val):
        events.append((tick, order, len(events), reg, val))  # the counter keeps the order of equal events

    plain = []
    setup(plain, voices)
    for tick, order, reg, val in plain:
        add(tick, order, reg, val)
    rhythm = 0x00
    if has_drums:
        drum_setup(add)
        rhythm = 0x20
        add(0, 0, 0xBD, rhythm)
    offset = 0.0
    last_tl = {}
    busy = [None] * voices   # (channel, note, started) of the note playing on each voice
    drum_on = {}             # drum bit -> tick it was started
    for notes, length in songs:
        for sec, kind, ch, note, vel in notes:
            tick = int(round((offset + sec) * TICKS))
            if ch == 9:
                if not has_drums or note not in DRUM_BITS:
                    continue
                bit, level_reg = DRUM_BITS[note]
                if kind == "on":
                    if rhythm & bit:  # retrigger: key off first
                        rhythm &= ~bit
                        add(tick, 0, 0xBD, rhythm)
                    tl = int((127 - vel) * 0x1F / 127)
                    if last_tl.get(level_reg) != tl:
                        add(tick, 1, level_reg, tl)
                        last_tl[level_reg] = tl
                    rhythm |= bit
                    add(tick, 2, 0xBD, rhythm)
                    drum_on[bit] = tick
                elif rhythm & bit:
                    rhythm &= ~bit
                    add(max(tick, drum_on.get(bit, tick) + DRUM_MIN_TICKS), 0, 0xBD, rhythm)
                continue
            if kind == "on":
                free = [v for v in range(voices) if busy[v] is None]
                v = free[0] if free else min(range(voices), key=lambda x: busy[x][2])
                if busy[v] is not None:  # steal: key off the old note first
                    fn, bl = fnum_block(hz(busy[v][1]))
                    add(tick, 0, 0xB0 + v, (bl << 2) | (fn >> 8))
                busy[v] = (ch, note, tick)
                fn, bl = fnum_block(hz(note))
                tl = int((127 - vel) * 0x1F / 127)
                if last_tl.get(v) != tl:
                    add(tick, 1, 0x40 + OPS[v][1], tl)
                    last_tl[v] = tl
                add(tick, 1, 0xA0 + v, fn & 0xFF)
                add(tick, 2, 0xB0 + v, 0x20 | (bl << 2) | (fn >> 8))
            else:
                for v in range(voices):
                    if busy[v] is not None and busy[v][0] == ch and busy[v][1] == note:
                        fn, bl = fnum_block(hz(note))
                        add(max(tick, busy[v][2] + 1), 0, 0xB0 + v, (bl << 2) | (fn >> 8))
                        busy[v] = None
                        break
        end_tick = int(round((offset + length) * TICKS)) + 1
        for v in range(voices):  # the file ended with notes still on
            if busy[v] is not None:
                fn, bl = fnum_block(hz(busy[v][1]))
                add(end_tick, 0, 0xB0 + v, (bl << 2) | (fn >> 8))
                busy[v] = None
        if has_drums and rhythm & 0x1F:
            rhythm &= ~0x1F
            add(end_tick, 0, 0xBD, rhythm)
        offset += length + GAP_S
    events.sort()
    end = events[-1][0] + int(0.4 * TICKS)
    body = bytearray()
    for i, (tick, _, _, reg, val) in enumerate(events):
        nxt = events[i + 1][0] if i + 1 < len(events) else end
        body += struct.pack("<BBH", reg, val, nxt - tick)
    if len(body) <= 65535:  # type 1: a 16 bit length, then the events
        data = struct.pack("<H", len(body)) + bytes(body)
        kind = "type 1"
    else:  # type 0: only events, the first word is zero (a harmless write of 0 to register 0)
        data = struct.pack("<BBH", 0, 0, 0) + bytes(body)
        kind = "type 0"
    with open(out, "wb") as f:
        f.write(data)
    print("%s: %s, %d bytes, %.1f s, %d events, %s" % (out, kind, len(data), end / float(TICKS), len(events),
                                                      "6 voices + drums" if has_drums else "9 voices"))


if __name__ == "__main__":
    args = sys.argv[1:]
    drums = "--no-drums" not in args
    args = [a for a in args if a != "--no-drums"]
    if len(args) < 2:
        sys.exit(__doc__)
    convert(args[1:], args[0], drums)

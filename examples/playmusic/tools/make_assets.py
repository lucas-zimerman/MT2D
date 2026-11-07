#!/usr/bin/env python3
"""Renders the songs of the playmusic example to .ogg files (original recordings of public domain music).

    python3 tools/make_assets.py

Notes are typed in as tables and played by a tiny additive synthesizer with a chord accompaniment.
Needs ffmpeg (libvorbis) to encode the .ogg files, the rest uses the standard library.
"""
import math
import os
import struct
import subprocess
import tempfile
import wave

RATE = 44100
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")

# (midi note, length in units), midi 0 = rest, 60 = middle C
ODE_TO_JOY = [
    (64,4),(64,4),(65,4),(67,4),(67,4),(65,4),(64,4),(62,4),(60,4),(60,4),(62,4),(64,4),(64,6),(62,2),(62,8),
    (64,4),(64,4),(65,4),(67,4),(67,4),(65,4),(64,4),(62,4),(60,4),(60,4),(62,4),(64,4),(62,6),(60,2),(60,8),
    (62,4),(62,4),(64,4),(60,4),(62,4),(64,2),(65,2),(64,4),(60,4),(62,4),(64,2),(65,2),(64,4),(62,4),(60,4),(62,4),(55,8),
    (64,4),(64,4),(65,4),(67,4),(67,4),(65,4),(64,4),(62,4),(60,4),(60,4),(62,4),(64,4),(62,6),(60,2),(60,12),
]
FUR_ELISE = [
    (76,2),(75,2),(76,2),(75,2),(76,2),(71,2),(74,2),(72,2),(69,6),(0,2),
    (60,2),(64,2),(69,2),(71,6),(0,2),(64,2),(68,2),(71,2),(72,6),(0,2),
    (64,2),(76,2),(75,2),(76,2),(75,2),(76,2),(71,2),(74,2),(72,2),(69,6),(0,2),
    (60,2),(64,2),(69,2),(71,6),(0,2),(64,2),(72,2),(71,2),(69,8),
]
# chords the accompaniment can pick from: root midi note + intervals
CHORDS = [(48, (0, 4, 7)), (43, (0, 4, 7)), (45, (0, 3, 7)), (40, (0, 4, 7)), (50, (0, 3, 7)), (41, (0, 4, 7))]


def hz(midi):
    return 440.0 * 2 ** ((midi - 69) / 12.0)


def add_note(buf, start, length, freq, vol, bright=1.0):
    """Additive 'organ/piano' like note: a few harmonics, quick attack, decaying body."""
    n = int(length * RATE)
    s0 = int(start * RATE)
    for i in range(n):
        if s0 + i >= len(buf):
            break
        t = i / RATE
        env = min(1.0, i / (0.01 * RATE)) * math.exp(-1.6 * t / max(length, 0.1)) * min(1.0, (n - i) / (0.04 * RATE))
        v = (math.sin(2 * math.pi * freq * t)
             + 0.45 * bright * math.sin(4 * math.pi * freq * t)
             + 0.2 * bright * math.sin(6 * math.pi * freq * t))
        buf[s0 + i] += vol * env * v


def render(notes, unit_s, name):
    total = sum(l for _, l in notes) * unit_s + 1.5
    buf = [0.0] * int(total * RATE)
    # melody
    t, bars = 0.0, {}
    for midi, length in notes:
        if midi:
            add_note(buf, t, length * unit_s * 1.05, hz(midi), 0.32)
            bars.setdefault(int(t / (unit_s * 16 if name == "ode" else unit_s * 24)), []).append(midi % 12)
        t += length * unit_s
    # accompaniment: one chord per bar, the one that matches the melody notes best
    bar_len = unit_s * (16 if name == "ode" else 24)
    for bar, pcs in bars.items():
        best = max(CHORDS, key=lambda c: sum(((m - c[0]) % 12) in c[1] for m in pcs))
        root, ivs = best
        for k, iv in enumerate(ivs):
            for rep in range(4 if name == "ode" else 3):
                add_note(buf, bar * bar_len + rep * bar_len / (4 if name == "ode" else 3) + k * 0.01,
                         bar_len / 4, hz(root + iv), 0.10 if k else 0.16, bright=0.4)
    peak = max(abs(x) for x in buf) or 1.0
    wav_path = tempfile.mktemp(suffix=".wav")
    with wave.open(wav_path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(x / peak * 28000)) for x in buf))
    out = os.path.join(OUT, name + ".ogg")
    subprocess.check_call(["ffmpeg", "-y", "-loglevel", "error", "-i", wav_path, "-c:a", "libvorbis", "-q:a", "3", out])
    os.remove(wav_path)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    render(ODE_TO_JOY, 0.12, "ode")
    render(FUR_ELISE, 0.075, "elise")
    print("done")

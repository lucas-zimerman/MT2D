#!/usr/bin/env python3
"""Renders assets/desert.ogg, the chase theme of the desert example (an original tune, CC0).

    python3 tools/make_music.py

A minor, 150 BPM, 16 bars that loop: drums, bass, an arpeggio and a lead made by a tiny synthesizer.
Needs ffmpeg (libvorbis) to encode the .ogg, the rest uses the standard library.
"""
import math
import os
import random
import struct
import subprocess
import tempfile
import wave

RATE = 22050
BPM = 150
STEP = 60.0 / BPM / 4          # one 16th note
BAR = STEP * 16
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "desert.ogg")

# one chord per bar: (bass root midi, chord tones above the root)
MINOR, MAJOR = (0, 3, 7), (0, 4, 7)
PROGRESSION = [(33, MINOR), (33, MINOR), (29, MAJOR), (31, MAJOR),
               (33, MINOR), (33, MINOR), (29, MAJOR), (28, MAJOR)] * 2

# lead riff, one list per bar of 8 bars: (step 0..15, midi note, length in steps)
RIFF_A = [(0, 81, 3), (4, 84, 2), (6, 83, 2), (8, 81, 4), (12, 79, 2), (14, 81, 2)]
RIFF_B = [(0, 81, 2), (2, 79, 2), (4, 76, 4), (8, 79, 3), (12, 76, 2), (14, 74, 2)]
RIFF_C = [(0, 77, 3), (4, 81, 2), (6, 79, 2), (8, 77, 4), (12, 76, 2), (14, 77, 2)]
RIFF_D = [(0, 79, 3), (4, 83, 2), (6, 81, 2), (8, 79, 4), (12, 78, 2), (14, 79, 2)]
RIFF_E = [(0, 76, 3), (4, 80, 2), (6, 83, 2), (8, 88, 6), (14, 83, 2)]
LEAD = [RIFF_A, RIFF_B, RIFF_C, RIFF_D, RIFF_A, RIFF_B, RIFF_C, RIFF_E] * 2


def hz(m):
    return 440.0 * 2 ** ((m - 69) / 12.0)


def tone(buf, start, length, freq, vol, harmonics, decay):
    """Adds a note made of a few harmonics (saw like), a short attack and an exponential decay."""
    s0 = int(start * RATE)
    n = int(length * RATE)
    for i in range(n):
        j = s0 + i
        if j >= len(buf):
            break
        t = i / RATE
        env = min(1.0, i / (0.004 * RATE)) * math.exp(-decay * t) * min(1.0, (n - i) / (0.012 * RATE))
        w = 2 * math.pi * freq * t
        v = sum(math.sin(w * k) / k for k in range(1, harmonics + 1))
        buf[j] += vol * env * v


def kick(buf, start):
    for i in range(int(0.22 * RATE)):
        j = int(start * RATE) + i
        if j >= len(buf):
            break
        t = i / RATE
        f = 45 + 110 * math.exp(-t * 28)
        buf[j] += 0.9 * math.exp(-t * 14) * math.sin(2 * math.pi * f * t + 0.5 * math.sin(t * 40))


def snare(buf, start, rnd):
    for i in range(int(0.16 * RATE)):
        j = int(start * RATE) + i
        if j >= len(buf):
            break
        t = i / RATE
        buf[j] += 0.45 * math.exp(-t * 26) * rnd.uniform(-1, 1) + 0.3 * math.exp(-t * 30) * math.sin(2 * math.pi * 190 * t)


def hat(buf, start, rnd, open_hat=False):
    last = 0.0
    for i in range(int((0.14 if open_hat else 0.04) * RATE)):
        j = int(start * RATE) + i
        if j >= len(buf):
            break
        t = i / RATE
        x = rnd.uniform(-1, 1)
        buf[j] += 0.16 * math.exp(-t * (14 if open_hat else 70)) * (x - last)  # differentiating = bright noise
        last = x


def main():
    rnd = random.Random(5)
    bars = len(PROGRESSION)
    buf = [0.0] * int(bars * BAR * RATE)
    for b, (root, chord) in enumerate(PROGRESSION):
        t0 = b * BAR
        for s in range(16):
            t = t0 + s * STEP
            if s in (0, 6, 8, 10):
                kick(buf, t)
            if s in (4, 12):
                snare(buf, t, rnd)
            if s % 2 == 0:
                hat(buf, t, rnd, open_hat=(s == 14))
            if s == 15:
                hat(buf, t, rnd, True)
            # bass: eighth notes, jumping an octave now and then
            if s % 2 == 0:
                m = root + (12 if s in (6, 14) else 0)
                tone(buf, t, STEP * 1.8, hz(m), 0.20, 5, 7)
            # arpeggio on the chord, two octaves up
            m = root + 24 + chord[(s % 3) if s % 4 != 3 else 1] + (12 if s % 8 >= 4 else 0)
            tone(buf, t, STEP * 0.9, hz(m), 0.045, 3, 9)
        for step, midi, length in LEAD[b]:
            tone(buf, t0 + step * STEP, length * STEP, hz(midi), 0.085, 6, 2.5)
            tone(buf, t0 + step * STEP, length * STEP, hz(midi) * 1.004, 0.05, 4, 2.5)  # a little detune
    peak = max(abs(x) for x in buf) or 1.0
    fade = int(0.01 * RATE)
    for i in range(fade):
        buf[-1 - i] *= i / fade
    wav = tempfile.mktemp(suffix=".wav")
    with wave.open(wav, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, x / peak * 0.92)) * 32000)) for x in buf))
    subprocess.check_call(["ffmpeg", "-y", "-loglevel", "error", "-i", wav, "-c:a", "libvorbis", "-q:a", "4", OUT])
    os.remove(wav)
    print("desert.ogg: %.1f seconds" % (bars * BAR))


if __name__ == "__main__":
    main()

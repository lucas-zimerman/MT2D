#!/usr/bin/env python3
"""Renders the sound effects of the desert example to assets/*.wav (original sounds, CC0).

    python3 tools/make_sfx.py

roar (the worm rises), whoosh (the leap), thud (the landing), screech (the drift), alert (the banner).
Only the Python standard library is needed.
"""
import math
import os
import random
import struct
import wave

RATE = 22050
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")
rnd = random.Random(11)


def save(name, samples, gain=0.9):
    peak = max(abs(x) for x in samples) or 1.0
    fade = int(0.01 * RATE)
    for i in range(min(fade, len(samples))):
        samples[-1 - i] *= i / fade
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(x / peak * gain * 32000)) for x in samples))


def lowpass(xs, k):
    out, y = [], 0.0
    for x in xs:
        y += k * (x - y)
        out.append(y)
    return out


def roar():
    n = int(1.0 * RATE)
    out, phase = [], 0.0
    for i in range(n):
        t = i / RATE
        f = 85 - 38 * t + 6 * math.sin(t * 40)               # a falling growl
        phase += 2 * math.pi * f / RATE
        saw = sum(math.sin(phase * k) / k for k in range(1, 9))
        grit = 0.55 + 0.45 * math.sin(2 * math.pi * 31 * t)    # the rattle in the throat
        env = min(1.0, t / 0.08) * math.exp(-1.4 * t)
        out.append((saw * grit + 0.5 * rnd.uniform(-1, 1)) * env)
    save("roar", lowpass(out, 0.35))


def whoosh():
    n = int(0.6 * RATE)
    noise = [rnd.uniform(-1, 1) for _ in range(n)]
    out = []
    y = 0.0
    for i in range(n):
        t = i / n
        k = 0.04 + 0.5 * math.sin(math.pi * t) ** 2           # the filter opens and closes
        y += k * (noise[i] - y)
        out.append(y * math.sin(math.pi * t) ** 1.5)
    save("whoosh", out)


def thud():
    n = int(0.7 * RATE)
    out = []
    for i in range(n):
        t = i / RATE
        f = 38 + 70 * math.exp(-t * 18)
        out.append(math.sin(2 * math.pi * f * t) * math.exp(-t * 7)
                   + 0.6 * rnd.uniform(-1, 1) * math.exp(-t * 22))
    save("thud", lowpass(out, 0.5))


def screech():
    n = int(0.7 * RATE)
    out, phase = [], 0.0
    for i in range(n):
        t = i / RATE
        f = 780 + 260 * math.sin(t * 9) + 140 * math.sin(t * 31) - 250 * t
        phase += 2 * math.pi * f / RATE
        v = math.sin(phase) + 0.5 * math.sin(phase * 2.01) + 0.6 * rnd.uniform(-1, 1)
        env = min(1.0, t / 0.05) * min(1.0, (0.7 - t) / 0.2)
        out.append(v * env * (0.7 + 0.3 * math.sin(t * 70)))
    save("screech", out, 0.6)


def alert():
    out = []
    for f in (880, 660, 880, 660):
        n = int(0.1 * RATE)
        for i in range(n):
            t = i / RATE
            sq = 1.0 if math.sin(2 * math.pi * f * t) > 0 else -1.0
            out.append(sq * 0.5 * min(1.0, (n - i) / (0.02 * RATE)))
        out.extend([0.0] * int(0.03 * RATE))
    save("alert", out, 0.6)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    roar()
    whoosh()
    thud()
    screech()
    alert()
    print("done")

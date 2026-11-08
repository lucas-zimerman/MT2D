#!/usr/bin/env python3
"""Generates the sound effects used by the playsound example (original sounds, CC0).

    python3 tools/make_assets.py

.wav files are written with the standard library only. The .ogg files are encoded with ffmpeg
(libvorbis), install it or delete the OGG lines to only get wav files.
"""
import math
import os
import random
import shutil
import struct
import subprocess
import tempfile
import wave

RATE = 22050
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")
# MS-DOS cannot play .ogg: the engine plays d/<name cut to 8 characters>.wav (jump.ogg -> d/jump.wav)
DOS_OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "d")


def square(phase):
    return 1.0 if (phase % 1.0) < 0.5 else -1.0


def tone(freq_from, freq_to, ms, vol=0.5, decay=3.0, wave_fn=square):
    """A tone that slides from freq_from to freq_to with a decaying volume."""
    n = int(RATE * ms / 1000)
    out, phase = [], 0.0
    for i in range(n):
        t = i / n
        phase += (freq_from + (freq_to - freq_from) * t) / RATE
        env = math.exp(-decay * t) * min(1.0, i / 60.0)
        out.append(vol * env * wave_fn(phase))
    return out


def noise(ms, vol=0.4):
    """Explosion: noise through a low pass filter that closes while the sound fades."""
    rnd = random.Random(5)
    n = int(RATE * ms / 1000)
    out, y = [], 0.0
    for i in range(n):
        t = i / n
        k = 0.45 * (1.0 - t) + 0.02
        y += k * (rnd.uniform(-1, 1) - y)
        out.append(vol * (1.0 - t) ** 1.5 * y * 2.2)
    return out


def save(name, samples):
    path = os.path.join(OUT, name)
    ext = os.path.splitext(name)[1]
    wav_path = path if ext == ".wav" else tempfile.mktemp(suffix=".wav")
    with wave.open(wav_path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, s)) * 32000)) for s in samples))
    os.makedirs(DOS_OUT, exist_ok=True)
    shutil.copyfile(wav_path, os.path.join(DOS_OUT, os.path.splitext(name)[0][:8] + ".wav"))
    if ext == ".ogg":
        subprocess.check_call(["ffmpeg", "-y", "-loglevel", "error", "-i", wav_path, "-c:a", "libvorbis", "-q:a", "3", path])
        os.remove(wav_path)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    save("coin.wav", tone(988, 988, 80, decay=0.5) + tone(1319, 1319, 320, decay=4.0))
    save("laser.wav", tone(2400, 300, 260, decay=1.5))
    save("powerup.wav", sum([tone(300 + i * 150, 300 + i * 150 + 150, 45, decay=0.8) for i in range(10)], []))
    save("blip.wav", tone(1000, 1000, 40, decay=1.0))
    save("jump.ogg", tone(200, 700, 180, decay=2.0))
    save("explosion.ogg", noise(900))
    print("done")

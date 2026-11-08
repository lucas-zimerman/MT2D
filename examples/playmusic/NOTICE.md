# Credits and notice: `d/badapple.imf`

The MS-DOS build of this example has a third song, **"Bad Apple!!"**. It is not original MT2D material and
**the MT2D license does not cover it**. (The other two, Beethoven's "Ode to Joy" and "Fur Elise", are public domain
compositions typed in as note tables by `tools/make_assets.py` and rendered to IMF by `tools/make_imf.py`.)

| | |
|---|---|
| Original composition | **ZUN** (Team Shanghai Alice), *Touhou Project*, "Lotus Land Story" (1998) |
| Arrangement | **Masayoshi Minoshima** / **Alstroemeria Records**, album *Lovelight* (2007) |
| MIDI file | made by **PGFLIMSXD / nlexsctex / handhule90** (names taken from the track names of the file, which says "Artist: Touhou" and "Midi made in 22 NOV 2025") |
| IMF file | converted from that MIDI by `tools/midi2imf.py` (one FM instrument, 6 melodic voices + OPL2 rhythm-mode drums), so it only sounds like a rough chip-music version |

*Touhou Project* and its music belong to ZUN / Team Shanghai Alice. This is an **unofficial, non-commercial fan
work**: it is not made, endorsed or supported by them or by Alstroemeria Records. It is included under the
spirit of the Touhou Project fan-work guidelines (non-commercial use, a clear mention that it is based on
*Touhou Project*, never presented as official), which are also why the credits above are shown on screen
while the song plays.

Things you should know before reusing it:

* The terms of the MIDI file (its authors did not include a license text in the file) were **not verified**.
  If you ship a build that is commercial, or you are not sure, delete `d/badapple.imf` and use your own song:
  `python3 tools/midi2imf.py d/yourSong.imf yourSong.mid`, then `playmus.exe d/yourSong.imf`.
* The Beethoven songs (the `.ogg` files of the SDL build and `d/ode.imf`, `d/elise.imf`) are not affected by
  this notice.

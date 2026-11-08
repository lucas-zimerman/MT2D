# MT2D examples

Small programs that show how to use MT2D. Every folder has its own `CMakeLists.txt` and the code is in
`<example>/src`. The executables are written to `<example>/bin` (git ignored).

| Example | Backend | What it shows |
| --- | --- | --- |
| `helloworld` | terminal | Start the engine, write text on the screen buffer, wait for a key |
| `allchars` | terminal | Show the 256 CP850 characters of the screen buffer in a 16x16 table |
| `inputpopup` | terminal | Message box, text input, number input and a two option question |
| `menu` | terminal | `MT2D_Short_Menu` / `MT2D_Menu` |
| `playpcspeaker` | terminal | PC speaker: sound effects and two public domain songs (Beethoven) made of beeps |
| `playsound` | SDL + SDL2_mixer, MS-DOS | `MT2D_Play_Audio`: plays `.wav` / `.ogg` sound effects, many at the same time (on MS-DOS the `.wav` files of `d` through the Sound Blaster) |
| `playmusic` | SDL + SDL2_mixer, MS-DOS | `MT2D_Play_Music`: streams and loops a song with a visualizer (`.ogg` with SDL2_mixer; on MS-DOS an IMF song on the OPL2 chip, see [`playmusic/NOTICE.md`](playmusic/NOTICE.md) for its credits) |
| `asciigraphics` | terminal | Plasma, starfield and spiral drawn with the MS-DOS block characters |
| `graphics` | SDL | PNG images drawn on top of the text screen, parallax scrolling, smooth movement with the joystick API |
| `cscript` | SDL | Scripts (Cscript) attached to object states |
| `sprites` | SDL | Animated PNG sprites (walk cycle, spinning coins, ghosts) with `Object`, `ObjectState` and `ObjectScene` |
| `desert` | SDL, MS-DOS (VGA) | Object scripts (`ObjectCore_LoadScript`, globals, `CreateObject`, `GotoStateIfHitObject`, `random()`): a monster truck drifts away from a leaping sand worm over parallax scenery, with a looping soundtrack and sound effects played by the scripts (`PlayMusic`, `PlaySound`, need SDL2_mixer; MS-DOS only plays the soundtrack, as IMF music on the OPL2 chip) |

The terminal examples use the engine's native text backend, the SDL ones open a window that emulates the
text screen and can also draw images (see `MT2D/MT2D_Terminal_Define.h`). SDL examples are only built for DJGPP when the experimental VGA mode is on (see [MS-DOS](#ms-dos-djgpp)), and the music of `playmusic` and `desert` plays as IMF on the OPL2 chip (`playsound` plays its effects through the Sound Blaster).

## Building

You need CMake 3.12+ and a C++ compiler.

```bash
# one example
cmake -S examples/helloworld -B examples/helloworld/build
cmake --build examples/helloworld/build
./examples/helloworld/bin/helloworld

# all of them
cmake -S examples -B build/examples
cmake --build build/examples
```

SDL examples (`graphics`, `cscript`, `sprites`, `desert`, `playsound`, `playmusic`) run from their `bin` folder, which also receives the font
(`assets/msdos_fnt.png`) and the example's images.

### Linux

```bash
sudo apt install build-essential cmake libncurses-dev libsdl2-dev libsdl2-image-dev
# plus libsdl2-mixer-dev to hear the playsound and playmusic examples
```

The terminal examples use ncurses (wide character support, `libncursesw`) and need a UTF-8 terminal: the screen buffers hold CP850 text (like on MS-DOS) that is converted to Unicode when drawn, and typed characters are converted back to CP850. There is no PC speaker on modern machines, so `MT2D_System_Beep` tries the
Linux console speaker first and otherwise writes a short tone to a temporary `.wav` and plays it with `paplay`,
`pw-play` or `aplay`. If none of them exists you only get silence (the timing of the songs stays the same).

### macOS

```bash
brew install cmake sdl2 sdl2_image
```

ncurses ships with macOS, beeps are played with `afplay`.

### Windows

Terminal examples only need a compiler (Visual Studio or MinGW). For the SDL examples install
`sdl2` and `sdl2-image` (for example with vcpkg) and pass the vcpkg toolchain file:

```bat
cmake -S examples -B build\examples -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build\examples --config Release
```

SDL2.dll must be next to the executable (vcpkg copies it for you).

### MS-DOS (DJGPP)

Cross compile with a DJGPP toolchain (for example [build-djgpp](https://github.com/andrewwutw/build-djgpp))
and the toolchain file of this folder. The executable names are shortened to 8 characters (`hello.exe`,
`inputpop.exe`, `pcspeak.exe`, ...).

```bash
export PATH=/path/to/djgpp/bin:$PATH
cmake -S examples -B build/dos -DCMAKE_TOOLCHAIN_FILE=$PWD/examples/cmake/djgpp.cmake
cmake --build build/dos
```

This builds the terminal examples in the normal text mode. The programs need a DPMI host: put
[`CWSDPMI.EXE`](https://www.delorie.com/djgpp/) next to them (DOSBox, 86Box or real hardware).

#### VGA mode (experimental)

`-DMT2D_DOS_VGA=ON` builds the VGA renderer (mode 13h: 320x200, 256 colors, the text screen is drawn with a bitmap
font) which can also draw image sprites. It defines `MSDOS_USE_GRAPHICS` (you can also define it yourself in
`MT2D_Terminal_Define.h`) and adds the image examples (`graphics`, `cscript`, `sprites`, `desert`) to the build,
the music of `desert` plays on the OPL2 chip (`desert/d/desert.imf`), there are no sound effects. The script compiler is turned on for `desert`.

```bash
cmake -S examples -B build/dos -DCMAKE_TOOLCHAIN_FILE=$PWD/examples/cmake/djgpp.cmake -DMT2D_DOS_VGA=ON
cmake --build build/dos
```

Each example copies its `d` folder and scripts next to the executable (`<example>/bin`). Run it from there,
with `CWSDPMI.EXE` in the same folder.

How images work in this mode:

* MS-DOS cannot read PNG files. `Load_Sprite_Image("name.png", ...)` loads `d/name.bmp` instead (the name is cut to
  8 characters, e.g. `planet_blue.png` -> `d/planet_b.bmp`), so the code and the object scripts keep using the PNG names.
  A name that already ends with `.bmp` is loaded as it is.
* The BMP files must be uncompressed 8 bit images. Their palette is sent to the VGA when they are loaded, so all the
  images of a program should share one palette. Palette index 0 is transparent and the indexes 16 and 31 are the
  colors of the text (black and white).
* `tools/png2dos.py` makes the `d` folders from the PNGs of `assets` (Python 3 and Pillow):
  `python3 examples/tools/png2dos.py examples/sprites examples/graphics examples/desert`. Cyan pixels (the
  color key of the SDL backend) become transparent and the other colors are quantized to one palette per example.
* The screen is 320x200, the SDL one is 320x240: images placed below y = 200 are not visible.
* Sprites are drawn over the text screen, in tiles of 4x8 pixels (`MSDOS_USE_SPRITE_TILE_RENDER`).

## Audio

`playsound` and `playmusic` need SDL2_mixer (with OGG support) to play files: when it is found, CMake defines
`SDL_USE_AUDIO` and the engine's `MT2D_Audio_Init`, `MT2D_Play_Audio`, `MT2D_Play_Music` and `MT2D_Audio_Close`
work. Without it the engine ignores those calls, the examples still build and only print a message.
The terminal examples never need audio files, `playpcspeaker` uses `MT2D_System_Beep`.
Use `-DMT2D_EXAMPLES_AUDIO=OFF` to ignore SDL2_mixer even when it is installed.

On MS-DOS (DJGPP) the engine plays **IMF** songs on the OPL2 chip (Adlib / Sound Blaster, DOSBox emulates one):
`MT2D_Audio_Init` returns 1 when the chip is found, `MT2D_Play_Music("song.ogg")` loops `d/song.imf` (the name is cut
to 8 characters and the extension replaced, like the images; a name that ends with `.imf` is used as it is).
Songs that fit the 32 KB song buffer are loaded whole, bigger ones are streamed from the disk. `playmusic` is
the example: `playmus.exe` shows its menu (Ode to Joy, Fur Elise and "Bad Apple!!") and `playmus.exe d/other.imf` plays any IMF file. Make IMF files from MIDI
files with `playmusic/tools/midi2imf.py`. "Bad Apple!!", the third song, is a fan work with its own credits, see
[`playmusic/NOTICE.md`](playmusic/NOTICE.md).

Sound effects (`MT2D_Play_Audio("coin.ogg")`) play `d/coin.wav` (same naming rule) through the **Sound Blaster** (2.0 or newer,
found with the `BLASTER` environment variable, DOSBox sets it): PCM mono `.wav` files, 8 or 16 bit, up to 3 at the same time (playing the same sound again restarts its voice instead of using another one),
mixed in software and sent by auto-init DMA at 22050 Hz, 8 bit (a file with another rate is converted when it is loaded).
`playsound` is the example (`playsnd.exe`); `playsound/tools/make_assets.py` also writes its `d` folder.

## Assets

* `assets/msdos_fnt.png` and `assets/msdos.ico`: font and window icon used by the SDL backend (from the MT2D / SuperShibe3K projects).
* `graphics/assets`, `sprites/assets`, `desert/assets`, `playsound/assets` and `playmusic/assets`: original art and sounds generated by
  `<example>/tools/make_assets.py` (CC0; `desert` also has `tools/make_music.py` and `tools/make_sfx.py`, and `tools/make_imf.py` that renders its tune as `d/desert.imf` for MS-DOS). Run the scripts again to regenerate them (Python 3, the `.ogg` files also
  need `ffmpeg` with libvorbis).
* `graphics/d`, `sprites/d` and `desert/d`: the same images as 8 bit BMP files for the MS-DOS VGA mode, made by `tools/png2dos.py`.
* The songs in `playpcspeaker` and `playmusic` are Beethoven compositions (public domain), typed in as note tables,
  the recordings in `playmusic/assets` are rendered by a small synthesizer.

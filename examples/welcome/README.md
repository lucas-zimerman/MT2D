# welcome

The home page of MT2D ([GitHub Pages](https://lucas-zimerman.github.io/MT2D/)), and an MT2D program: the menu, the effects, the
sprites, the sound and the music in the screen of the page are drawn by the engine itself. It is built **only** with Emscripten.

| Screen | What it shows | Calls |
| --- | --- | --- |
| Menu | an 80x25 text menu: box drawing, a block letter banner, selection with the keyboard or the page buttons | `insert_string_on_display`, `WINDOW1[y][x]` |
| Text mode | plasma, fire, starfield and spiral drawn with the shade characters | `WINDOW1[y][x]` |
| Graphics | space flight (held keys, parallax, coins), a **desert game** (drive the truck in every direction, a sand worm chases you under the sand, rears up and leaps at where you are heading, a hit is game over, no warning), a sprite zoo; the text screen stays behind the images | `Load_Sprite_Image`, `Sprite_Render_on_Window`, `MT2D_Joystick_*` |
| Sound lab | six sample effects and a piano whose notes are WAV files made in memory at start-up | `MT2D_Play_Audio`, `MT2D_PlayAudioFromMemory` |
| Music player | three songs, pause, volume, a visualizer fed by the mixer output | `MT2D_Play_Music`, `Mix_SetPostMix` (SDL2_mixer) |
| About | what MT2D is and where it runs | |

The desert uses the sprites of the [`desert`](../desert) example (there the worm and the truck are object scripts and the truck drives
itself), the game here is plain C. Text is drawn under the images, so the game over message is an image
(`assets/gameover.png`, made from the engine font by [`tools/make_banner.py`](tools/make_banner.py)) and the score is on the first text row,
which the scene leaves uncovered.

The piano does not use `MT2D_System_Beep`: with the SDL backend the PC speaker emulation and audio files cannot be used in the same run (see
[`Documentation/general/audio.md`](../../Documentation/general/audio.md)), and this program plays files.

The program runs at 60 steps a second whatever the refresh rate of the screen ([`../common/web_timing.h`](../common/web_timing.h)).

Keys: arrows and Enter in the menu, `1`-`5` open a screen, `Esc` goes back, `M` mutes. The page has buttons for the same keys.
The page starts muted (`M` or the Unmute button turns the sound on): the browser keeps the sound locked until the first key press anyway, the song starts then.

## Files

* [`src/main.c`](src/main.c): the program (ANSI C). One function, `frame()`, is called once per screen refresh with
  `emscripten_set_main_loop`, like [`web-tui`](../web-tui/README.md).
* [`page.html`](page.html): the page (the Emscripten shell file): hero, the canvas with its controls, features, platforms, getting started, showcase.
  The buttons dispatch the same keyboard events the engine reads. The engine calls `window.mt2dScene(n)` (through `EM_ASM`) so the page can
  mark the active screen. SDL writes the window size on the canvas style, the page overrides it with `!important` CSS.
* [`CMakeLists.txt`](CMakeLists.txt): the images and sounds are not copied, they come from the `graphics`, `sprites`, `desert`,
  `playsound` and `playmusic` examples and are preloaded into the browser's virtual file system. `SHELL page.html` selects the page and
  `Logo.png` is copied next to it.

## Building

You need the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) and CMake 3.12+:

```bash
emcmake cmake -S examples/welcome -B build/welcome -DCMAKE_BUILD_TYPE=Release
cmake --build build/welcome
python3 -m http.server 8000 --directory examples/welcome/bin
```

Browsers do not load `.wasm` from `file://`, serve the folder. [`.github/workflows/pages.yml`](../../.github/workflows/pages.yml)
builds it on every pull request and publishes `examples/welcome/bin` on GitHub Pages when `master` changes.

Known limit: with the phone in portrait the engine draws its rotated layout, the page asks for landscape.

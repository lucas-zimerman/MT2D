# Platforms, backends and defines

The portable headers (`MT2D.h`, `MT2D_Display.h`, `MT2D_Keyboard.h`, `MT2D_System_Calls.h`, `Cursor/Cursor.h`,
`Joystick/MT2D_Joystick.h`) pick a backend at compile time with `#if`/`#elif`, in this order. The first
match wins, so `SDL_USE` overrides the native backend of the OS:

| Order | Condition | Backend | `MT2D_Init` is |
| --- | --- | --- | --- |
| 1 | `SDL_USE` | SDL2 window that emulates the text screen, can draw images | `MT2D_SDL_Init` |
| 2 | `_WIN32` | Windows console | `MT2D_Win_Init` |
| 3 | `MSDOS_USE_GRAPHICS` | MS-DOS VGA 320x200x256 (experimental, C++ in `_MSDOS/`) | `MT2D_Dos_VGA_Init` |
| 4 | `__MSDOS__` / `__DOS__` | MS-DOS text mode | `MT2D_Dos_Init` |
| 5 | `linux` (Linux and macOS) | ncurses terminal | `MT2D_Linux_Init` |
| - | `__EMSCRIPTEN__` without SDL | `#error`, Emscripten needs `SDL_USE` | - |

Other OS: `#error "MT2D doesn't have code to support this operational system"`.

`MT2D_Set_Settings` and `MT2D_Set_Settings_Argv` ([settings.md](settings.md)) use the same order: each is a macro for the backend
of the first matching row (`MT2D_SDL_Set_Settings`, `MT2D_Win_Set_Settings`, ...).

## Defines (`MT2D/MT2D_Terminal_Define.h`)

Off unless you uncomment them there or pass `-D...` to the compiler.

| Define | Effect |
| --- | --- |
| `SDL_USE` | SDL2 backend (needs SDL2 and SDL2_image) |
| `SDL_USE_AUDIO` | audio through SDL2_mixer, see [audio.md](audio.md) |
| `SDL_stbimage` | `SDL_stbimage` instead of SDL_image (UWP and similar) |
| `MT2D_SDL_GPU` | SDL_GPU instead of SDL render calls |
| `MSDOS_USE_GRAPHICS` | MS-DOS VGA mode, see [gui/limitations.md](../gui/limitations.md) |
| `MSDOS_USE_DOUBLE_BUFFER` | **no longer works** (accepted, but does nothing): the VGA renderer writes each changed tile once, no flicker and no 64 KB buffer. Remove it from your build, see [MIGRATION.md](../../MIGRATION.md) |
| `MT2D_TERMINAL_USE_GRAPHICS` | Linux terminal mode: draw the screen as a Sixel image (see below) |
| `MT2D_TERMINAL_USE_AUDIO` | Linux terminal mode: play `.wav` / `.ogg` with command line players (see below) |
| `MSDOS_VGA_WAIT_RETRACE` | only with `MSDOS_USE_GRAPHICS`: wait the vertical retrace before drawing a frame |
| `MT2D_SCREEN_RESIZE`, `MT2D_SDL_USING_DIRECTX_RENDER`, `DEBUG_OBJECTCORE` | see the comments in the file |

`MT2D_USING_CONTAINER` is always on. `MT2D_WINDOWED_MODE` is defined by `MT2D.h`. Do not edit the
"settings that you should not touch" part of the header (it derives `WINDOWS_TARGET`, `LINUX_TARGET`,
`ANDROID_TARGET`, `BROWSER_TARGET`, `MT2D_RENDERING_PRINTF`).

## Building

There is no single build system: compile all `MT2D/**/*.c` with your
project. Per target details are in [`../CONTRIBUTING.md`](../../CONTRIBUTING.md) and
[`../examples/README.md`](../../examples/README.md) (CMake files for every example, DJGPP toolchain file in
`examples/cmake/djgpp.cmake`). SDL links: `-lSDL2 -lSDL2_image` (+ `-lSDL2_mixer` for audio).

Linux native terminal: ncurses with wide characters, UTF-8 terminal, CP850 is converted when drawn.
With `MT2D_TERMINAL_USE_GRAPHICS` (CMake examples: `-DMT2D_TERMINAL_GRAPHICS=ON`) the screen is drawn as a Sixel image
instead: every cell is an 8x16 bitmap font glyph (640x384 pixels, white on black), a frame equal to the previous one is
not sent again. curses is still used for the keyboard. It needs a terminal with Sixel support (xterm started with
`-ti vt340`, foot, mlterm, wezterm, ...). Graphics only: the sound is a separate flag (`MT2D_TERMINAL_USE_AUDIO`, below).
`MT2D_Init` asks the terminal for its device attributes (`ESC [ c`, waits up to 2 s) and, when Sixel (attribute 4) is not
listed, prints an error to stderr and exits with code 1. `MT2D_SIXEL=1` in the environment skips the check, for a terminal
that draws Sixel but does not say so.
Like the MS-DOS VGA mode, the screen has **one palette of 256 colors** and everything is drawn with palette indexes (nothing is
built on the fly). `MT2D_Init` sets the default VGA palette through the `MT2D_SET_PALETTE` setting (see
[settings.md](settings.md)), a program replaces it with that same setting, and loading a BMP sets the palette of the file. The
text is drawn with index 16 (black) and 31 (white) and index 0 is the transparent color of the sprites, so all the images of a
program should share one palette that keeps those. Image sprites (`Load_Sprite_Image`) load the 8 bit BMP files of the MS-DOS VGA mode
(see `examples/tools/png2dos.py`): `"ship.png"` is read from `d/ship.bmp`, and a name ending in `.bmp` is used as it is. They are placed
on a 320x240 virtual screen like with SDL and drawn over the text (up to 256 images per frame). The container variant
(`Load_Sprite_Image_From_Container`) still gives the `<!>` placeholder.
Ignored with `SDL_USE` and on other platforms.

With `MT2D_TERMINAL_USE_AUDIO` (CMake examples: `-DMT2D_TERMINAL_AUDIO=ON`) `MT2D_Play_Audio` / `MT2D_Play_Music` (and the
`FromMemory` / container versions) play `.wav` and `.ogg` files with command line players, nothing is linked: a sound effect
runs `paplay || pw-play || aplay`, the music (one song at a time, looped) `paplay || pw-play || ffplay || aplay`. `aplay` only
plays `.wav`; for `.ogg` you need `paplay`/`pw-play` with libsndfile, or `ffplay`. The file is looked for as given and then in
`d/`. IMF songs (MS-DOS) are not played, and without a player the sounds are silent. The players run with their input and output
on `/dev/null` so they never draw on the terminal. The sound plays on the machine that runs the game, not through an ssh
connection. The music stops when the game ends, however it ends (exit, Ctrl+C, a closed terminal, `kill -9`). On Linux the kernel tells the music shell at once (`PR_SET_PDEATHSIG`), on macOS a small watcher in that shell checks once a second that the game still exists, so after a `kill -9` the music can play up to 1 second more. Sound effects that are
already playing finish by themselves.
It works with or without the Sixel graphics.
Android: `Android.mk`. MS-DOS: DJGPP or Open Watcom, 8.3 file names.

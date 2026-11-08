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

## Defines (`MT2D/MT2D_Terminal_Define.h`)

Off unless you uncomment them there or pass `-D...` to the compiler.

| Define | Effect |
| --- | --- |
| `SDL_USE` | SDL2 backend (needs SDL2 and SDL2_image) |
| `SDL_USE_AUDIO` | audio through SDL2_mixer, see [audio.md](audio.md) |
| `SDL_stbimage` | `SDL_stbimage` instead of SDL_image (UWP and similar) |
| `MT2D_SDL_GPU` | SDL_GPU instead of SDL render calls |
| `MSDOS_USE_GRAPHICS` | MS-DOS VGA mode, see [gui/limitations.md](../gui/limitations.md) |
| `MSDOS_USE_DOUBLE_BUFFER` | only with `MSDOS_USE_GRAPHICS`: no flicker, costs 64 KB of RAM |
| `MT2D_SCREEN_RESIZE`, `MT2D_SDL_USING_DIRECTX_RENDER`, `DEBUG_OBJECTCORE` | see the comments in the file |

`MT2D_USING_CONTAINER` is always on. `MT2D_WINDOWED_MODE` is defined by `MT2D.h`. Do not edit the
"settings that you should not touch" part of the header (it derives `WINDOWS_TARGET`, `LINUX_TARGET`,
`ANDROID_TARGET`, `BROWSER_TARGET`, `MT2D_RENDERING_PRINTF`).

## Building

There is no single build system: compile all `MT2D/**/*.c` plus the `ObjectCore/*.cpp` files with your
project. Per target details are in [`../CONTRIBUTING.md`](../../CONTRIBUTING.md) and
[`../examples/README.md`](../../examples/README.md) (CMake files for every example, DJGPP toolchain file in
`examples/cmake/djgpp.cmake`). SDL links: `-lSDL2 -lSDL2_image` (+ `-lSDL2_mixer` for audio).

Linux native terminal: ncurses with wide characters, UTF-8 terminal, CP850 is converted when drawn.
Android: `Android.mk`. MS-DOS: DJGPP or Open Watcom, 8.3 file names.

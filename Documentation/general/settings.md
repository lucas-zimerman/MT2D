# Settings (`MT2D_Set_Settings`)

Header: `MT2D/MT2D_Settings.h`. One way to configure the engine, the same call on every platform.

```c
#include <MT2D/MT2D_Settings.h>

MT2D_Set_Settings(MT2D_SDL_SET_WINDOWED, 800, 600);        /* up to two params */

int cfg[] = { MT2D_DOS_SET_AUDIO, 0x220, 5, 1, 22050 };    /* a list: port, irq, dma, rate */
MT2D_Set_Settings_Argv(5, cfg);
```

| Call | For |
| --- | --- |
| `MT2D_Set_Settings(operation, param1, param2)` | operations with up to two params |
| `MT2D_Set_Settings_Argv(argc, argv)` | operations that take a list (`argv[0]` = the operation, `argv[1..]` = its params), or values that come from an array |

Each operation below says which call handles it, the other call answers `MT2D_UNSUPPORTED` for it.
`MT2D_Set_Settings_Argv` returns `MT2D_INVALID` when `argc < 1` or `argv == NULL`.

## Result (`MT2D_Result`)

| Value | | Meaning |
| --- | --- | --- |
| `MT2D_OK` | 1 | done |
| `MT2D_INVALID` | 0 | bad `argc`/`argv`, or the operation could not be done (e.g. no sound card answered) |
| `MT2D_BEFORE_INIT_ONLY` | -1 | can only be set before `MT2D_Init()` |
| `MT2D_BAD_PARAMETER` | -2 | a param is out of range (nothing is changed) |
| `MT2D_UNSUPPORTED` | -3 | this platform / build has no such setting (also an operation of another platform) |

Nothing is changed unless `MT2D_OK` is returned.

## Operations

Grouped by platform: 1xxx Windows, 2xxx Linux, 3xxx macOS, 4xxx MS-DOS, 5xxx SDL, 6xxx shared (1xxx..3xxx and 6xxx
have no operation yet). A backend answers `MT2D_UNSUPPORTED` to the operations of the other ranges.

| Operation | Call | Params | Notes |
| --- | --- | --- | --- |
| `MT2D_SDL_SET_WINDOW_SIZE` (5000) | `MT2D_Set_Settings` | width, height in pixels | size of the window to be created, **before** `MT2D_Init()` only (`MT2D_BEFORE_INIT_ONLY` after) |
| `MT2D_SDL_SET_WINDOWED` (5001) | `MT2D_Set_Settings` | width, height | leaves fullscreen. `0, 0` keeps the size, one of them 0 is `MT2D_BAD_PARAMETER`. Before or after init |
| `MT2D_SDL_SET_FULLSCREEN` (5002) | `MT2D_Set_Settings` | none | always at the monitor resolution (borderless, the video mode is not changed). Before or after init |
| `MT2D_SDL_ALT_ENTER_TOGGLE_FULLSCREEN` (5003) | `MT2D_Set_Settings` | 1 on, 0 off | default 0. When on, Alt+Enter (also keypad Enter) switches fullscreen / windowed and the program does not see that Enter |
| `MT2D_DOS_SET_AUDIO` (4000) | `MT2D_Set_Settings_Argv` | `port, irq, dma, rate` | Sound Blaster setup, see below |
| `MT2D_SET_PALETTE` (6000) | `MT2D_Set_Settings_Argv` | 256 colors `0xRRGGBB` | the whole palette of the graphics (MS-DOS VGA, Linux Sixel), see below |

The SDL operations exist when `SDL_USE` is defined (Android, Windows UWP and Emscripten builds need it anyway).
Without `SDL_USE` they return `MT2D_UNSUPPORTED`.

**`MT2D_OK` does not always mean the window changed.** SDL cannot tell if the platform honors a window size or a
windowed request. On mobile (the OS owns the window) and with window managers that refuse it, the window can stay as it
is. Do not assume the size you asked for: the engine follows the real size through the window resize events.
Fullscreen (and Alt+Enter) are expected to work wherever SDL supports a fullscreen window, they were not tested on mobile.

## MS-DOS audio: `MT2D_DOS_SET_AUDIO`

`{ MT2D_DOS_SET_AUDIO, port, irq, dma, rate }` (DJGPP only, `MT2D_UNSUPPORTED` otherwise). Without it the engine reads the
`BLASTER` variable (`SET BLASTER=A220 I5 D1 T3`) and uses `MT2D_DOS_SOUND_RATE` (22050).

* A param that is missing or negative is not overridden (port, irq and rate also 0). So `{ op, -1, 7 }` only sets the irq.
* Ranges: port `0x210`-`0x280`, irq 3-15 (2 is the cascade, mapped to 9), dma 0, 1 or 3, rate 4000-44100 Hz.
  Out of range: `MT2D_BAD_PARAMETER`, nothing changes.
* Call it **before** `MT2D_Audio_Init()`. If the card is already running it is restarted with the new setup: playing sounds
  are cut and the sound cache is dropped (loaded sounds are converted to the rate). A restart that fails is `MT2D_INVALID`.
* The engine only resets the DSP at that port: a **wrong irq is not detected**, it starts and stays silent.
  DOSBox's default card is `A220 I7 D1`. `MT2D_Audio_Init()` is also non-zero when only the OPL music chip answered, use
  `MT2D_Dos_Sound_Init()` (`_MSDOS/M2Sound.h`) to know about the sound card alone.

Example: [`examples/dos/SetBlaster`](../../examples/dos/SetBlaster/src/main.c).

## Palette: `MT2D_SET_PALETTE`

`{ MT2D_SET_PALETTE, c0, c1, ... c255 }`: `argc` must be exactly 257, else `MT2D_BAD_PARAMETER`. Only the colors of the graphics have a
palette: the MS-DOS VGA mode (`MSDOS_USE_GRAPHICS`) and the Linux Sixel graphics (`MT2D_TERMINAL_USE_GRAPHICS`). Every other
build (the text modes, SDL, Windows) answers `MT2D_UNSUPPORTED`. Only the low 24 bits of a color are used, the VGA keeps 6 bits per
component (the Sixel graphics keep 8). Index 0 is the transparent color of the sprites and 16 / 31 are the text colors (black and white
in the default palette).
Loading a BMP also sets the palette (with its own), so set yours after loading the images. On Linux `MT2D_Init` calls this same setting
with the default VGA palette, and the next frame is sent again with the new colors.
Changing only the palette moves colors without redrawing: [`examples/dos/DiscoPallete`](../../examples/dos/DiscoPallete/src/main.c)
draws a spiral once and animates it with a new palette every frame.

## How it works

Like `MT2D_Init`, both calls are macros for the function of the platform, picked in the same order as
[platforms.md](platforms.md): `MT2D_SDL_Set_Settings(_Argv)`, `MT2D_Win_...`, `MT2D_Dos_VGA_...`, `MT2D_Dos_...`,
`MT2D_Linux_...` (declared in each platform's init header). Nothing is stored in the shared file: `MT2D_Settings.c` only has the
`MT2D_UNSUPPORTED` fallbacks for a platform without backend. A backend applies a setting to its own state (for SDL, the
`SCREEN_WIDTH` / `SCREEN_HEIGHT` globals and the window).

To add a setting: add an entry to `MT2D_Setting_Operation` inside the range of its platform, document the params there, handle it in
the backend(s) that support it (the others already answer `MT2D_UNSUPPORTED`), and say in the table above which call carries it.

Tests: `tests/test_settings.cpp` builds with `MT2D_SETTINGS_NO_PLATFORM`, which skips the platform selection so the fallbacks are used.
The backends are not covered by the automated tests.

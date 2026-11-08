# Limitations

What each target can and cannot do.

## Everywhere

* The screen is a grid of 80x25 cells, one byte each (CP850). No colour or attribute data in the buffers.
* Text screen limits: see [../tui/display.md](../tui/display.md) (no row check in `insert_string_on_display`,
  `transfer_window*` copies 24 rows, `MT2D_Line_Draw_Vertical` fills a row and `MT2D_Line_Draw_Horizontal` fills a column).
* Popups, menus and input boxes **block** (own loop). Do not call them from a game loop that must keep running.
* Menus have no cancel key ([../tui/menus.md](../tui/menus.md)). Input boxes return `""` on `ESC` and the string must be freed.
* `MT2D_Joystick_*` and `MT2D_Keyboard_*` should not be used in the same program, and the joystick is the keyboard
  (keyboard on every backend, plus a real controller with SDL: stick axes 0/1 and buttons 0..3 only, no d-pad or right stick),
  see [../general/joystick.md](../general/joystick.md).
* One global container ([../general/container.md](../general/container.md)); avoid duplicate names. Image sprites from the container are SDL only and never decrypted.
* The headers have no thread safety; `ObjectCore` states are read-only data shared between objects.

## Image sprites by target

| Target | Images | Notes |
| --- | --- | --- |
| SDL (Windows, Linux, macOS, Android, Emscripten) | yes | PNG and others through SDL_image. UWP: use `SDL_stbimage` |
| Native Windows console / Linux ncurses / MS-DOS text | **no** | ASCII sprites, objects and Cscript work normally ([../scripting/](../scripting/README.md)) |
| MS-DOS VGA (`MSDOS_USE_GRAPHICS`, experimental) | yes | see below |

`struct Sprite` type 1 (`Image`) is "platform specific image (SDL only for now)"; type 0 (`Text`) works everywhere.

## SDL

* Images are always drawn over the text, there is no "text over image" layer.
* Image coordinates are 320x240 virtual pixels, not window pixels.
* Rotating on smartphones is not supported: lock the screen to landscape.
* Audio: either the beeper or audio files per run (`SDL_SOUND_TYPE`). On Emscripten the audio starts after the first click or key press (browser rule), and `MT2D_System_Beep` does not wait for the tone to end.
* The SDL key list (`Keyboard_Buffer`) is shared by the keyboard and joystick code; key releases are applied one frame late.
* The SDL font/size values live in globals (`SCREEN_WIDTH`, `FONT_SIZEX`, ...), not in a config struct.
* The engine has no hotkeys of its own: F1..F12 are returned to your program as key codes ([../general/keyboard.md](../general/keyboard.md)).

## MS-DOS VGA mode

* 320x200, 256 colours, text drawn with a bitmap font; the SDL screen is 320x240, so images at `y >= 200` are not visible.
* No PNG: `Load_Sprite_Image("planet_blue.png", ...)` loads `d/planet_b.bmp` (8.3 style: name cut to 8 characters,
  `.bmp`, folder `d/`). A name that ends in `.bmp` is used as is.
* BMP must be **uncompressed 8 bit**. The palette of each loaded image is sent to the VGA, so all images of a
  program should share one palette. Index 0 = transparent, indexes 16 and 31 = text colours (black, white).
* Use `examples/tools/png2dos.py` to convert (cyan pixels become transparent).
* Sprites are drawn in tiles of 4x8 pixels: a tile is built in a small buffer and written once, only when its char or
  its sprites changed, so there is no flicker and no 64 KB double buffer (`MSDOS_USE_DOUBLE_BUFFER` is ignored).
  The renderer uses about 18 KB of static memory plus the sprite list, which grows when needed and is reused every frame.
* Audio: music as IMF on OPL2, effects as 8/16 bit PCM mono `.wav` through a Sound Blaster, max 3 at once
  ([../general/audio.md](../general/audio.md)). The `desert` object scripts only play the soundtrack on DOS.
* `MT2D/_MSDOS/` stays C++ (DJGPP style) for now.
* DJGPP programs need a DPMI host (`CWSDPMI.EXE`) next to the `.exe`. Everything DOS visible needs 8.3 names.

## Platform status

Backends in the source: SDL, Windows console, MS-DOS text and VGA, Linux/macOS ncurses, Android (`Android.mk`, needs SDL),
Emscripten (needs SDL). See [../general/platforms.md](../general/platforms.md).

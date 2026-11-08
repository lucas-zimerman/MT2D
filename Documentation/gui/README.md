# GUI mode (images on top of the text screen)

"GUI" in MT2D = the **SDL2 backend** (or the experimental MS-DOS VGA mode). The program is the same as a
text one, but the text screen is drawn in a window with a bitmap font, and **image sprites** can be drawn over it.
Read [limitations.md](limitations.md) before promising a feature on a platform.

## Initialize (SDL)

1. Enable SDL: `#define SDL_USE` in `MT2D/MT2D_Terminal_Define.h` or pass `-DSDL_USE`. For sound also `SDL_USE_AUDIO`
   ([../general/audio.md](../general/audio.md)).
2. Link SDL2 and SDL2_image (+ SDL2_mixer for audio): `-lSDL2 -lSDL2_image -lSDL2_mixer`.
   On Windows MSVC the pragmas in `MT2D.h` link them for you and `main` is un-defined from SDL.
   Debian/Ubuntu packages: `libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev`.
3. Put the font `assets/msdos_fnt.png` (from `examples/assets`) in the working directory of the program
   (the examples copy it to `<example>/bin`) and run the program from there. So do your image files.
4. Call `MT2D_Init()` once (it is `MT2D_SDL_Init`). Free with `MT2D_SDL_Close()` when you finish.
5. Draw with the normal text API ([../tui/display.md](../tui/display.md)) and add images:

```c
#include <MT2D/MT2D.h>
#include <MT2D/MT2D_Display.h>
#include <MT2D/MT2D_Keyboard.h>
#include <MT2D/ObjectCore/Sprites.h>

int main(int argc, char **argv)        /* same form of main as the examples */
{
    Sprite *ship;
    (void)argc; (void)argv;
    MT2D_Init();
    ship = Load_Sprite_Image((char *)"ship.png", 16, 16); /* file, size on the 320x240 screen */
    if (!ship) return 1;                                  /* missing file, wrong working directory */
    for (;;) {
        MT2D_Clear_Main_Window();                         /* also clears last frame's images */
        Sprite_Render_on_Window(ship, DISPLAY_WINDOW1, 100, 80);   /* x, y in 320x240 pixels (returns false on SDL, ignore it) */
        insert_string_on_display((char *)"ESC quits", 0, 0, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);                /* text + images appear here */
        if (MT2D_Keyboard_touched() && MT2D_Keyboard_keytouched() == escape_pressed) break;
    }
    Sprite_Delete(ship);
    return 0;
}
```

## Facts to rely on

* The text screen is still 80x25 cells (`WINDOW1`); images use a separate **320x240 virtual screen** that is
  scaled to the window. Window default is 1280x720 (`SCREEN_WIDTH/HEIGHT` in `MT2D_SDL_Defines.c`);
  `MT2D.h` defines `MT2D_WINDOWED_MODE`, so it starts windowed.
* Images are always drawn **on top of** the text: a text cell under an image is hidden.
* Typical frame: `MT2D_Clear_Main_Window` -> render sprites/objects/text -> `MT2D_Draw_Window(DISPLAY_WINDOW1)`
  -> read input -> update -> `MT2D_System_Delay(~30)`.
* Input: [../general/keyboard.md](../general/keyboard.md). Use the joystick API ([../general/joystick.md](../general/joystick.md)) for smooth movement, several keys and a game controller (SDL only; MS-DOS is keyboard only).
* Touch (SDL finger down) is reported as the space key (32).
* Sprites, objects and Cscript are not GUI only (ASCII ones work in text mode too): see
  [../scripting/README.md](../scripting/README.md), [../scripting/sprites.md](../scripting/sprites.md).
* Working code to copy: `examples/graphics` (parallax + joystick), `examples/sprites` (animated objects),
  `examples/cscript`, `examples/desert` (object scripts + sound).

## MS-DOS VGA mode

Same program, other backend: build with `-DMSDOS_USE_GRAPHICS` (CMake `-DMT2D_DOS_VGA=ON`), `MT2D_Init`
becomes `MT2D_Dos_VGA_Init`. Rules for images (BMP, 8 characters, palette) are in [limitations.md](limitations.md).

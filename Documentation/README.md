# MT2D documentation (for agents)

Short notes written from the source code, one topic per file.

MT2D = "Monochrome Text 2D Engine". Everything draws into an **80x25 text screen buffer**
(`WINDOW1` / `WINDOW2`, one byte per cell, CP850 characters). On some backends images (sprites) can be
drawn on top of that text screen.

## Minimal program

```c
#include <MT2D/MT2D.h>
#include <MT2D/MT2D_Display.h>
#include <MT2D/MT2D_Keyboard.h>

int main(void)
{
    MT2D_Init();                       /* once, picks the backend of the platform */
    MT2D_Clear_Main_Window();          /* clears WINDOW1 */
    insert_string_on_display((char *)"Hello, MT2D!", 10, 34, DISPLAY_WINDOW1); /* row, column */
    MT2D_Draw_Window(DISPLAY_WINDOW1); /* nothing shows until you draw */
    MT2D_Keyboard_keytouched();        /* wait for a key */
    return 0;
}
```

More: [`examples/`](../examples/README.md) (one small program per feature, copy the closest one).

## Map

| I want to... | Read |
| --- | --- |
| write text, draw boxes/lines, move the cursor | [tui/display.md](tui/display.md) |
| read the keyboard | [general/keyboard.md](general/keyboard.md) |
| held keys, several keys at once ("joystick") | [general/joystick.md](general/joystick.md) |
| show a menu | [tui/menus.md](tui/menus.md) |
| message boxes, text/number/password input, delay, beep | [tui/io.md](tui/io.md) |
| draw images, use SDL | [gui/README.md](gui/README.md) |
| know what does not work | [gui/limitations.md](gui/limitations.md) |
| sprites (ASCII works everywhere, images need SDL/VGA) | [scripting/sprites.md](scripting/sprites.md) |
| animated objects, states, scenes | [scripting/objects.md](scripting/objects.md) |
| scripts that run on frames (Cscript), built in functions | [scripting/cscript.md](scripting/cscript.md) |
| `MT2D_VAR` vars, globals | [scripting/vars.md](scripting/vars.md) |
| the text script language, `cscriptc`, `.mto` files | [Object-Scripts.md](Object-Scripts.md) (overview in [scripting/README.md](scripting/README.md)) |
| play sound or music | [general/audio.md](general/audio.md) |
| open/read files (`MT2D_FILE`) | [general/files.md](general/files.md) |
| pack or load named assets, encrypted data (container) | [general/container.md](general/container.md) |
| defines, backends, per-OS build notes | [general/platforms.md](general/platforms.md) |

Folders: `general/` = works the same in text mode and with SDL (input, audio, files, platforms), `tui/` = the text
screen and the widgets drawn on it, `gui/` = what only SDL / DOS VGA add (images), `scripting/` = sprites, objects, Cscript, vars.

## Source layout (`MT2D/`)

| Folder / file | What |
| --- | --- |
| `MT2D.h/.c` | `WINDOW1/2`, `MT2D_Init`, `insert_string_on_display`, buffer copies |
| `MT2D_Display.h`, `MT2D_Keyboard.h`, `MT2D_System_Calls.h`, `Cursor/`, `Joystick/` | one portable header each, it maps the `MT2D_*` names to the backend of the OS |
| `MT2D_Layout.*`, `MessageBox/`, `InputBox/`, `Tools/UserInterface/`, `Tools/Video/` | text UI built on the buffers |
| `ObjectCore/` | sprites, objects, states, scenes, Cscript, vars (still C++ except the loader and globals), see [scripting/](scripting/README.md) |
| `Audio/`, `Container/`, `File/` | audio front end, packed asset container, file wrapper |
| `SDL/` | SDL2 backend (window, images, audio, keyboard, joystick) |
| `_WINDOWS/`, `_LINUX/`, `_MSDOS/`, `_ANDROID/`, `_EMNISCRIPT/` | OS specific code (keep it here) |
| `MT2D_Terminal_Define.h` | feature switches, see [general/platforms.md](general/platforms.md) |

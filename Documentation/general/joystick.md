# Joystick (held keys, several at once)

Header: `MT2D/Joystick/MT2D_Joystick.h`. Code: `Joystick/MT2D_Joystick.c` (the getters) + one backend per platform
(`SDL/IO/SDL_MT2D_Joystick.c`, `_WINDOWS/IO`, `_MSDOS/IO`, `_ANDROID/IO`, `_EMNISCRIPT/IO`). Example: `examples/graphics`.

**Two input sources feed one `MT2D_Joystick` state:**

* **The keyboard** (every backend): the arrow keys become the left stick (full deflection) and keys you map with `Map_Button0N`
  become buttons. Held keys stay held and several keys work at once, which `MT2D_Keyboard_*` cannot give you.
* **A real controller, only with SDL** (`SDL_USE`, also Android and Emscripten with SDL): `Init` calls `SDL_Init(SDL_INIT_JOYSTICK)`
  and opens controller 0; the SDL event handler (`MT2D_SDL_Event_Handler.c`) then copies `SDL_JOYAXISMOTION` axis 0/1 into the left
  stick (analog value) and `SDL_JOYBUTTONDOWN` buttons 0..3 into `Button_Pressed`. Axes 2+ (right stick), hats (d-pad) and button
  releases are not handled. No controller connected: no joystick events, the keyboard still works.

**MS-DOS is keyboard only** (arrows and mapped keys read from the keyboard queue; no game port code).
The UWP accelerometer start/stop exists in `MT2D_Win_Uwp.cpp`; I did not check what it feeds.

## Availability

`MT2D_Joystick.h` chooses the backend in this order, otherwise `#error "No code was done for this platform"`:

| Condition | Backend | Works? |
| --- | --- | --- |
| `SDL_USE` (not Android, not Emscripten) | SDL | yes: controller 0 (axes 0/1, buttons 0..3) + held arrows and mapped keys |
| `WINDOWS_TARGET` | Windows | **no input without SDL**: `Update` only clears the state |
| MS-DOS | DOS | keyboard only: arrows + 3 mapped buttons, read from the keyboard queue |
| Android / Emscripten | with `SDL_USE`: same as SDL; without: no input | |
| Linux / macOS without SDL | none | **compile error**, enable `SDL_USE` |

## Use

```c
#include <MT2D/Joystick/MT2D_Joystick.h>

MT2D_Joystick_Init();                               /* once; the handler is NULL before this and every call crashes */
MT2D_Joystick_Map_Button01(escape_pressed);         /* map the buttons you use right after Init */
MT2D_Joystick_Map_Button02(' ');
for (;;) {
    MT2D_Joystick_Update();                         /* once per frame, BEFORE reading: pumps the events */
    if (MT2D_Joystick_Button_Touched(1)) break;     /* IDs are 1..4 */
    int dx = MT2D_Joystick_Get_LeftAxisX();         /* -32768 / 0 / 32767 */
    int dy = MT2D_Joystick_Get_LeftAxisY();         /* read BOTH axes every frame, see below */
    /* ...move, draw (Clear, render, MT2D_Draw_Window), MT2D_System_Delay(30)... */
}
MT2D_Joystick_Close();
```

* `MT2D_Joystick_Update()` replaces `MT2D_Keyboard_touched/keytouched` in the loop. It reads the same event/key queue: on DOS it
  **drains `MT2D_Keyboard_*`** (any key that is not an arrow or a mapped button is thrown away); on SDL it pumps the SDL events. Do not
  mix the two APIs in one loop.
* Axis range is `-32768..32767` (up and left negative, down and right positive). Keys give full deflection only (SDL
  `-32768` / `32767`, DOS `-32767` / `32767`); a real controller gives analog values, including small noise around 0.
  `DeadZoneAxis` (8000) is set at init and **never applied by the engine**: apply your own dead zone, as `examples/graphics` does.
  If a controller and the arrow keys are used together, the last writer wins within a frame.
* **Reading clears.** `Get_LeftAxisX/Y` clear `Left_Modified` and `Touched`; `Button_Touched(id)` clears that button and `Touched`.
  The SDL backend resets the left axes to 0 only while `Left_Modified` is false, so if you never call a `Get_Left*` function a released
  arrow keeps its old value. Call both `Get_LeftAxisX` and `Get_LeftAxisY` every frame.
* Buttons are true for every `Update` while the mapped key is held (SDL) or when it is read from the queue (DOS key repeat).
  Test `Button_Touched` once per frame; it returns the flag and resets it.

## Mapping buttons

`MT2D_Joystick_Map_Button01..04(int key)` store a key code in `Mapped_Button[0..3]`; `Button_Touched(1..4)` reports it.

* SDL: use the lowercase SDL keysym value of the key: `'a'`..`'z'` are 97..122 (an uppercase `'A'` never matches), `' '` is 32, ESC is 27,
  Enter 13, Backspace 8, Tab 9. Keys with a keysym above 122 (F keys, arrows, keypad) cannot be mapped.
* DOS: codes as returned by `MT2D_Keyboard_keytouched` (uppercase/lowercase as typed). Only buttons **1 to 3** are read. The
  `Mapped_Button` array is **not initialised** by `MT2D_Dos_Joystick_Init` (plain `malloc`), so map buttons 1-3 right after `Init`
  or a stray value can match a key.
* An unmapped button holds 0 on SDL (never matches).
* Physical controller buttons need no mapping: SDL button 0 sets `Button_Pressed[0]`, i.e. `Button_Touched(1)`, up to button 3
  (`Button_Touched(4)`). It is a one frame pulse on the press event (no release event), cleared at the next `Update`.

## Functions

| Call | Notes |
| --- | --- |
| `Init()` / `Close()` / `Update()` | macros for the backend: `MT2D_Joystick_Init` etc. |
| `bool Touched()` | something happened since the last read |
| `bool ArrowKey_Touched()`, `LeftArrow_Touched()` | both return `Left_Modified` (same thing) |
| `bool RightArrow_Touched()` | `Right_Modified`, never true: no backend ever sets the right stick |
| `int Get_LeftAxisX/Y()` | see above; clears the flags |
| `int Get_RightAxisX()` | always 0 |
| `int Get_RightAxisY()` | returns `Right_X_axis` (always 0) |
| `bool Button_Touched(int ID)` | `ID` 1..4. `0` or `5+` index outside the array |
| `Map_Button01..04(int key)` | see above |

Struct `MT2D_Joystick` (public in the header): `Left_X_axis`, `Left_Y_axis`, `Left_Modified`, `Right_*`, `Button_Pressed[4]`,
`Mapped_Button[4]`, `Touched`, `DeadZoneAxis`, and with SDL `gGameController`. The single instance is the global
`GlobalJoystickHandler`; there is one joystick.

## SDL details worth knowing

* Key releases are applied one frame late (`MT2D_SDL_Flush_Key_Releases` at the start of `Update`) so a tap that begins and ends inside
  one frame is still seen.
* On Android a touch is stored in the key list as the space key (32), so `Map_Button0N(' ')` turns a screen touch into a button.
* The key list is shared with `MT2D_Keyboard_*`; see [keyboard.md](keyboard.md).

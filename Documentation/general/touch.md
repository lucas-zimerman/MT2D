# Touch controls (virtual buttons and sticks)

Header: `MT2D/Touch/MT2D_Touch.h`. Code: `Touch/MT2D_Touch.c` + the finger events in `SDL/MT2D_SDL_Event_Handler.c`.
Examples: `examples/graphics` (a stick and an ESC button, ASCII), `examples/welcome` (the same as images, bound per screen). Tests: `tests/test_touch.cpp`.

A touch control is a normal ObjectCore object ([../scripting/objects.md](../scripting/objects.md)) that is bound to
`MT2D_Touch`. While a finger is on it, it writes into the **joystick state** ([joystick.md](joystick.md)), so the game reads
touch, keys and a real controller with the same calls:

* a **Button** presses joystick button 1..4 (`MT2D_Joystick_Button_Touched(id)`). The arrow keys are not buttons, they are the left stick.
* an **Axis** moves the left or the right stick (`MT2D_Joystick_Get_LeftAxisX/Y`, `Get_RightAxisX/Y`), with analog values.

No Cscript is needed: `MT2D_Touch` switches the object between its "Select" and "Deselect" states and moves it.

## Availability

| Platform | |
| --- | --- |
| SDL (`SDL_USE`: desktop, Android, Emscripten) | works. Fingers come from `SDL_FINGERDOWN/MOTION/UP`, a portrait window (height >= width) is rotated like the screen |
| MS-DOS, Linux / Windows terminal | every call is an empty inline function (`Bind` returns -1): the same game code builds, nothing is shown |

Desktop SDL only gets finger events from a touch screen. The SDL hint `SDL_MOUSE_TOUCH_EVENTS=1` turns mouse clicks into fingers on
some SDL versions (not with sdl2-compat 2.32 on SDL3).

**Compatibility:** before `MT2D_Touch`, any touch was the space key on SDL. That still happens while **no control is bound**
(`MT2D_Touch_Count() == 0`); with at least one control bound, touches only go to the controls.

## Use

```c
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/Touch/MT2D_Touch.h>

MT2D_Joystick_Init();
MT2D_Touch_Init();                 /* after the joystick: it writes into GlobalJoystickHandler */
MT2D_Touch_Bind(stick);            /* reads the vars once, returns the slot or -1 */
MT2D_Touch_Bind(fire);
for (;;) {
    MT2D_Joystick_Update();        /* pumps the events (also the fingers) */
    MT2D_Touch_Update();           /* AFTER the joystick update: fingers -> buttons, sticks, states */
    /* ...read MT2D_Joystick_* as usual, draw the game... */
    MT2D_Touch_Render();           /* last, before drawing the window */
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}
MT2D_Touch_Close();                /* unbinds everything, the objects are not deleted */
MT2D_Joystick_Close();
```

* `Update` must come **after** `MT2D_Joystick_Update`: that one clears the buttons of the last frame. Touch input written
  after it is seen in the same frame.
* `Render` draws the controls (the area of a Reactive stick, then the object) and then calls `Object_Goto_NextStep` on each one,
  so their states animate and loop. Do not also add the objects to a scene of the game: they would be drawn and stepped twice.
* The controls are drawn last, so they cover the text under them. Image sprites of the game are drawn on top of the text
  screen (SDL), so they can cover ASCII controls.
* Blocking calls (`MT2D_Keyboard_keytouched`, menus, message boxes, input boxes) do not call `MT2D_Touch_Update` and do not
  see the controls, like they do not see the joystick.
* Without `MT2D_Joystick_Init` (`GlobalJoystickHandler` is NULL) the controls are drawn and change state but write nothing.

## Configuring a control

`MT2D_Touch_Bind(obj)` reads these vars of the object. Each one can be a **string** var (from C,
`MT2D_Object_Create_Var_String`) or an **int** var (object files only have `int` vars). All are optional.

| Var | String | Int | Default |
| --- | --- | --- | --- |
| `Btn_Type` | `"Button"` / `"Axis"` (any case) | 0 / 1 | Button |
| `Btn_Id` | Axis: `"Left"` / `"Right"` | Button: 1..4. Axis: 1 (left) / 2 (right) | 1 / Left |
| `Select` | state name | state index | state 1 (state 0 if there is only one) |
| `Deselect` | state name | state index | state 0 |
| `Ax_Style` | `"Fixed"` / `"Reactive"` | 0 / 1 | Fixed (Axis only) |
| `Ax_Area` | state name | state index | none, required by Reactive |

`Bind` returns -1 and binds nothing when a var has an invalid value, a state does not exist, Reactive has no `Ax_Area` sprite, the
16 slots (`MT2D_TOUCH_MAX_CONTROLS`) are used, or the Select / Deselect state has **no frame with a wait of at least 1**
(`Object_Goto_NextStep` loops forever on such a state). Binding the same object again reads its vars again. Changing the vars later
does nothing until it is bound again.

### Sizes and units

* The touch area of a Button or a Fixed stick is the object's rect: `Pos_X`, `Pos_Y`, `Size_X`, `Size_Y`. It is read every
  update, so the game can move it. `Size_X` / `Size_Y` 0 means the size of the first sprite of the Deselect state.
* Units are the ones of the sprites ([../scripting/sprites.md](../scripting/sprites.md)): **cells** (80x25) when that sprite is ASCII,
  the **320x240** screen when it is an image (its `ScaleX/ScaleY` is the size).

### Buttons

Pressed while any finger that no stick holds is on it. Fingers can slide from one button to another, several buttons can be held at
once, and a tap that starts and ends between two updates is still seen once. `Button_Pressed[Btn_Id - 1]` is set on every update
while held (the joystick update clears it every frame).

### Sticks

A finger that touches a stick's area grabs it and keeps it until it is lifted, even outside of the area (it does not press
buttons meanwhile). Values go from -32767 to 32767, `Left_Modified` / `Right_Modified` and `Touched` are set while held. On release
the stick goes back to 0, unless something else (an arrow key, a controller) wrote a different value in that frame.

* **Fixed**: the value is the offset of the finger from the center of the object's rect, the object does not move. A d-pad image works
  this way.
* **Reactive**: the first sprite of the `Ax_Area` state is the area, drawn at the object's position at bind time. The object is the
  thumb: it is centered in the area, follows the finger (clamped inside the area) and goes back to the center when released. The
  value is the thumb's offset from the center. If the area is not bigger than the thumb on an axis, that axis stays at 0: a 1 line high
  area is a horizontal slider. The area state is never stepped, its waits do not matter. `Unbind` puts the object back where it was
  bound.

The thumb moves over the whole area sprite, its border included.

## Other calls

| Call | |
| --- | --- |
| `MT2D_Touch_Show(MT2D_TOUCH_AUTO)` | the default: hidden until the first finger touches the screen, so desktop players never see the controls |
| `MT2D_Touch_Show(MT2D_TOUCH_SHOW)` / `(MT2D_TOUCH_HIDE)` | always shown / hidden. Hidden releases every control, ignores the fingers and draws nothing |
| `bool MT2D_Touch_Visible()` | the result of the mode above |
| `bool MT2D_Touch_Pressed(Object*)` | a finger holds that control |
| `int MT2D_Touch_Count()` | bound controls |
| `MT2D_Touch_Unbind(Object*)` | releases and unbinds one control |
| `MT2D_Touch_Finger_Down/Move/Up(id, x, y)` | input from the backend, x and y 0..`MT2D_TOUCH_RANGE` (32768) in the game's orientation. Tests and new backends call them |

Up to `MT2D_TOUCH_MAX_FINGERS` (10) fingers are tracked, the next ones are ignored. No heap is used.

## Example (C, ASCII sprites)

```c
static ObjectState *one(const char *name, int w, int h, const char *cells)
{
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)name);
    MT2D_ObjectState_ADD(st, Sprite_Create_Ascii_From_Cells(w, h, cells), 1, 0); /* wait 1: it can loop */
    return st;
}

ObjectState **states = malloc(3 * sizeof(ObjectState *));
states[0] = one("up", 3, 1, "(o)");          /* Deselect: state 0 */
states[1] = one("down", 3, 1, "(O)");        /* Select: state 1 */
states[2] = one("area", 11, 5, "...55 cells...");
Object *stick = Object_Create(false, false, 0, 0, 1, 18, states, 3); /* area at column 1, row 18 */
Object_Add_Variable(stick, MT2D_Object_Create_Var_String("Btn_Type", "Axis"));
Object_Add_Variable(stick, MT2D_Object_Create_Var_String("Ax_Style", "Reactive"));
Object_Add_Variable(stick, MT2D_Object_Create_Var_String("Ax_Area", "area"));
MT2D_Touch_Bind(stick);
```

The same in an object file uses ints: `int Btn_Type = 1; int Ax_Style = 1; int Ax_Area = 2;` (state index 2).

/*
        MT2D_Touch: touch screen buttons and sticks made of ObjectCore objects.

        A bound object is a virtual control that feeds the joystick state (MT2D_Joystick.h):
          * a "Button" presses joystick button 1..4 (MT2D_Joystick_Button_Touched) while a finger is on it
          * an "Axis" moves the left or the right stick (MT2D_Joystick_Get_LeftAxisX ...)

        The object is configured with its vars, read once by MT2D_Touch_Bind. Each one can be a string var
        (from C) or an int var (object files only have int vars):

          Btn_Type  "Button" / "Axis"                 or 0 / 1           (default Button)
          Btn_Id    Button: 1..4. Axis: "Left"/"Right" or 1 / 2          (default 1 / Left)
          Select    state while touched, name or index                   (default state 1, or 0 with one state)
          Deselect  state while not touched, name or index               (default state 0)
          Ax_Style  "Fixed" / "Reactive"              or 0 / 1           (Axis only, default Fixed)
          Ax_Area   Reactive only: the state whose first sprite is the area the thumb moves in, name or index

        Fixed:    the object's rect (Pos_X, Pos_Y, Size_X, Size_Y) is the touch area, the axis is the finger
                  offset from its center. The object does not move.
        Reactive: the Ax_Area sprite, drawn at the object's position at bind time, is the area. The object is
                  the thumb: it follows the finger inside the area and goes back to the center when released.
                  An axis where the area is not bigger than the thumb does not move (a 1 line area is a slider).

        Size_X / Size_Y 0 means the size of the object's sprite. Units are the ones of the sprites: cells for ASCII
        sprites, the 320x240 screen for image sprites (see Sprites.h).

        Use it like the joystick, once per frame:

                MT2D_Joystick_Update();
                MT2D_Touch_Update();      after the joystick update, it writes into the joystick state
                ...read the joystick, draw the game...
                MT2D_Touch_Render();      draws the controls, then steps their states (Object_Goto_NextStep)
                MT2D_Draw_Window(DISPLAY_WINDOW1);

        It only works with SDL (SDL_USE: desktop, Android, Emscripten). On the other platforms every call is
        an empty inline function, so the same game code builds everywhere.
        With no control bound, the SDL backend keeps the old behaviour: a touch is the space key.
*/
#ifndef MT2D_TOUCH_H
#define MT2D_TOUCH_H

#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/MT2D_CDecl.h>

#if defined(SDL_USE) && !defined(MT2D_USE_TOUCH)
#define MT2D_USE_TOUCH
#endif

#define MT2D_TOUCH_MAX_CONTROLS 16
#define MT2D_TOUCH_MAX_FINGERS 10
/* finger positions given to MT2D_Touch_Finger_* go from 0 to MT2D_TOUCH_RANGE on both axes */
#define MT2D_TOUCH_RANGE 32768

/* MT2D_Touch_Show */
#define MT2D_TOUCH_HIDE 0
#define MT2D_TOUCH_SHOW 1
#define MT2D_TOUCH_AUTO 2 /* the default: hidden until the first touch */

#if defined(MT2D_USE_TOUCH) || defined(MT2D_TOUCH_IMPLEMENTATION)

MT2D_C_BEGIN
void MT2D_Touch_Init(void);
void MT2D_Touch_Close(void); /* unbinds everything, the objects are not deleted */
/* Returns the slot (0..MT2D_TOUCH_MAX_CONTROLS-1) or -1: no free slot, invalid vars, or a Select/Deselect
   state that has no frame with a wait of at least 1 (Object_Goto_NextStep would never return). */
int MT2D_Touch_Bind(Object *obj);
void MT2D_Touch_Unbind(Object *obj);
void MT2D_Touch_Update(void);
void MT2D_Touch_Render(void);
void MT2D_Touch_Show(int mode);
bool MT2D_Touch_Visible(void);
/* true while a finger holds the control of that object */
bool MT2D_Touch_Pressed(Object *obj);
/* how many controls are bound. With none, the SDL backend keeps the old behaviour: a touch is the space key */
int MT2D_Touch_Count(void);

/* Input from the backend (the SDL event handler). x and y are 0..MT2D_TOUCH_RANGE in screen orientation
   of the game (left to right, top to bottom), already rotated for portrait screens. */
void MT2D_Touch_Finger_Down(long id, int x, int y);
void MT2D_Touch_Finger_Move(long id, int x, int y);
void MT2D_Touch_Finger_Up(long id, int x, int y);
MT2D_C_END

#else

/* no touch screen: empty functions (not macros, so the arguments are still evaluated and checked) */
MT2D_INLINE void MT2D_Touch_Init(void) {}
MT2D_INLINE void MT2D_Touch_Close(void) {}
MT2D_INLINE int MT2D_Touch_Bind(Object *obj) { (void)obj; return -1; }
MT2D_INLINE void MT2D_Touch_Unbind(Object *obj) { (void)obj; }
MT2D_INLINE void MT2D_Touch_Update(void) {}
MT2D_INLINE void MT2D_Touch_Render(void) {}
MT2D_INLINE void MT2D_Touch_Show(int mode) { (void)mode; }
MT2D_INLINE bool MT2D_Touch_Visible(void) { return false; }
MT2D_INLINE bool MT2D_Touch_Pressed(Object *obj) { (void)obj; return false; }
MT2D_INLINE int MT2D_Touch_Count(void) { return 0; }

#endif

#endif

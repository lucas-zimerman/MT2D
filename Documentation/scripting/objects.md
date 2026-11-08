# Objects, states and scenes

Headers: `MT2D/ObjectCore/Object.h`, `State.h`. Examples: `examples/sprites` (images), `examples/cscript`.

* **ObjectState** = named list of frames. Frame *i* = `Sprites[i]`, `WaitSprites[i]` (ticks) and `Functions[i]` (a `Cscript*` or
  `NULL`). Shared by every object that uses it and treated as read-only; keep it alive while any object uses it.
* **Object** = flags (`Solid`, `RenderOnly`), six built in vars, your own vars, an array of states, the current state/frame.
* **ObjectScene** = a group of objects.

## Building them

```c
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Object.h>   /* before State.h, they include each other */
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/STDFunctions.h>

ObjectState *walk = MT2D_OBJECT_CREATE_STATE((char *)"Walk");   /* the name must stay valid: it is not copied */
MT2D_ObjectState_ADD(walk, spriteA, 4, Cscript_New(Cscript_Move)); /* sprite, wait, script (0 or NULL = none) */
MT2D_ObjectState_ADD(walk, spriteB, 4, Cscript_New(Cscript_Move));

ObjectState **states = (ObjectState **)malloc(sizeof(ObjectState *));
states[0] = walk;                      /* states[0] is the state the object starts in; it needs at least one frame */
Object *o = Object_Create(false /*solid*/, false /*renderOnly*/, 3, 1 /*size*/, 10, 5 /*x, y*/, states, 1);
ObjectAcelX(o) = 1;

ObjectScene *scene = ObjectScene_CreateEmpty();
ObjectScene_Add(scene, o);             /* sets o->MyScene; CreateObject needs it */
```

`MT2D_OBJECT_ADD_STATE(state, sprite, wait, script)` is the older twin of `MT2D_ObjectState_ADD`; use the second one.

## One tick

Per frame of your game loop, in this order (see the examples):

```c
MT2D_Clear_Main_Window();
ObjectScene_Render(scene);            /* Object_Render for each: draws the CURRENT frame's sprite at Pos_X, Pos_Y */
/* ...other drawing, input... */
MT2D_Draw_Window(DISPLAY_WINDOW1);
ObjectScene_Goto_NextSteps(scene);    /* advance every object one tick, delete the ones marked CanDelete */
MT2D_System_Delay(30);
```

`Object_Goto_NextStep(o)` (from `Object.cpp`):

1. If `ActualFrameWait > 0`: decrement it and stop.
2. Otherwise loop while `ActualFrameWait == 0`:
   run the **current frame's Cscript**, `ActualFrame++`, then load that frame's wait (wrap to frame 0 and its wait
   after the last frame, so a state loops).

So a frame's function runs **when that frame's wait has finished** (after its sprite was shown `wait` ticks). Frames with wait `0`
run in the same tick, one after another. A state whose every frame has wait 0 never returns control (infinite loop).

Entering a state by script (`SetState`, `JumpToState...`) sets `ActualFrame = -1` so the `++` lands on frame 0; its function is
therefore not run until frame 0's wait is over. Exception: `JumpToStateIfRandom` sets `ActualFrame = 0`, so frame 0 is skipped.

Other facts:
* `Object_Render` draws only if `Enabled`. Position is `Pos_X`/`Pos_Y` (column,row for ASCII sprites; pixels for images).
* `RenderOnly` / `Solid`: only `Solid` objects matter for hit tests (`GotoStateIfHitObject`).
* Deleting: a script's `RemoveObject` / `Stop` only sets `CanDelete = true`. `ObjectScene_Goto_NextSteps` then calls
  `Object_Delete(obj, false)` and drops it from the scene. Outside a scene you delete it yourself.
* `Object_Delete(o, deleteStates)`: frees the vars and the object. Pass `true` only for the last user of the states; it also
  deletes the frames' sprites (refcount) and Cscripts.
* `ObjectScene_Add` grows the array with `realloc`; do not keep pointers into `ObjectGroup`.
* `ObjectScene_Add` is not checked against duplicates.

## Built in vars and your own

First six `User_Vars`: `Pos_X, Pos_Y, Size_X, Size_Y, Acel_X, Acel_Y` (`Object_PosX_Var_Index` ... `Object_AcelY_Var_Index`
= 0..5). Macros `ObjectPosX(o)`, `ObjectPosY`, `ObjectAcelX`, `ObjectAcelY`, `ObjectSizeX`, `ObjectSizeY` read/write the
int; the `...Ref(o)` forms give an `int*`. They are lvalues. Your own vars: `Object_Add_Variable(o, &var)` / `Object_Add_Variables(o, array, n)`
(the `MT2D_VAR` is copied by value, so build it with `MT2D_VAR_Init_*` or copy a `MT2D_Object_Create_Var_*` then `free` only the shell).
Details: [vars.md](vars.md). Size is only used for hit tests.

Hit test (`Object_Hit_Object`): inclusive axis aligned boxes `[Pos, Pos + Size]` on both axes, so touching edges count as a hit.

## A text mode example

A ship that flies right and wraps back to column 2 (C++, uses the generic `Cscript_New`):

```c
static MT2D_VAR *name(const char *n) { return MT2D_Object_Create_Var_Pointer(n); } /* state name, or var of the running object */
static MT2D_VAR *num(int v)          { return MT2D_Object_Create_Var_Int("n", v); }

Sprite *ship = Sprite_Create_Ascii_From_Cells(3, 1, "<A>");

ObjectState *fly = MT2D_OBJECT_CREATE_STATE((char *)"Fly");
MT2D_ObjectState_ADD(fly, ship, 1, Cscript_New(Cscript_Move));          /* shown 1 tick, then Pos += Acel */
MT2D_ObjectState_ADD(fly, 0, 0, Cscript_New(Cscript_JumpToStateIfVarHigherEq,
                                            name("Reset"), name("Pos_X"), num(70))); /* wait 0: same tick; if Pos_X >= 70 goto Reset */

ObjectState *reset = MT2D_OBJECT_CREATE_STATE((char *)"Reset");
MT2D_ObjectState_ADD(reset, 0, 0, Cscript_New(Cscript_TeleportToCoord, num(2), num(5)));
MT2D_ObjectState_ADD(reset, 0, 0, Cscript_New(Cscript_Object_SetState, name("Fly")));

ObjectState **states = (ObjectState **)malloc(2 * sizeof(ObjectState *));
states[0] = fly; states[1] = reset;
Object *o = Object_Create(false, false, 3, 1, 2, 5, states, 2);
ObjectAcelX(o) = 1;
```

Each tick: `ObjectScene_Render` draws `<A>` at (Pos_X, Pos_Y); `ObjectScene_Goto_NextSteps` runs `Move` after the 1 tick wait.
In C replace `Cscript_New(fn)` by `Cscript_New2(fn)` and `Cscript_New(fn, a, b)` by `Cscript_New3(MT2D_VAR_Create_Matrix2(a, b), fn)`
([cscript.md](cscript.md)).

## Loading from a file

```c
Object *model = ObjectCore_Load("movingdot.mto");     /* NULL: see ObjectCore_GetLastError() */
Object *copy  = ObjectCore_Clone(model, 10, 20);      /* shares the states; Object_Delete(copy, false) */
ObjectCore_Unload(model);                              /* last, when no clone is left */
```

Text scripts (`ObjectCore_LoadScript`), the globals registry and the binary format are in
[../Object-Scripts.md](../Object-Scripts.md) and `ObjectLoader.h`. `ColisionSystem.*` is an empty placeholder (0 bytes in the
`.cpp`): collision is `Object_Hit_Object` and the two `...IfHit...` functions.

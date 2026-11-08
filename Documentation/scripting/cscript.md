# Cscript

Headers: `MT2D/ObjectCore/Cscript.h` (the struct and constructors), `STDFunctions.h` (built in functions),
`CscriptFunctions.h` (the list the text compiler and loader know). Example: `examples/cscript`.

A **Cscript** is a function pointer plus the `MT2D_VAR` arguments it will receive. It is stored in one frame of an
`ObjectState` and executed by `Object_Goto_NextStep` when that frame's wait ends ([objects.md](objects.md)).
It never returns a value: a function writes results into the vars it was given.

The text language (`Move`, `x++`, `if`, `while`, `global ...`) is compiled into these same structures, see
[../Object-Scripts.md](../Object-Scripts.md). This page is the C side.

## The four function shapes

```c
void f(void);                                         /* constructed, but Object_Goto_NextStep never calls it, see below */
void f(Object *self);                                 /* Cscript_New2 */
void f(Object *self, MT2D_VAR **args);                /* Cscript_New3 */
void f(Object *self, Object *other, MT2D_VAR **args); /* Cscript_New4 (other = a model/target object) */
```

`self` is the object running the script. **`void f(void)` is stored but never executed by the engine**
(`Object_Private_Run_Function` and `Cscript_Execute` only call the three shapes that take an `Object*`). Use the other shapes.

## Making one

| Call | Result |
| --- | --- |
| `Cscript_New2(fn)` | `void f(Object*)` |
| `Cscript_New3(MT2D_VAR **args, fn)` | args is a `malloc`ed array of `MT2D_VAR*` (see `MT2D_VAR_Create_Matrix1..4/N` in [vars.md](vars.md)) |
| `Cscript_New4(Object *other, MT2D_VAR **args, fn)` | |
| `Cscript_New(fn, a, b, ...)` | **C++ only** (variadic macro + overloads). Chooses the shape from the type of `fn`; up to 16 args. For the `Object*, Object*, MT2D_VAR**` shape the first extra argument is the `Object*` |
| `Cscript_Empty()`, `Cscript_New1(fn)` | an empty/`void` one; not useful |

The Cscript only stores **pointers**: the vars, the array of pointers and the `other` object must outlive it. Nothing is copied.
One Cscript can be attached to several frames/states (share it to save memory).

Attach it: `MT2D_ObjectState_ADD(state, sprite, wait, cscript)`. Run it by hand: `Cscript_Execute(caller, cscript)`.

`Cscript_RemoveFromMemory(c)` frees the Cscript, its `args` pointer array and, if `other` is set, **`free()`s the `other` object**
(for unpacked ones) but not the vars. `MT2D_ObjectState_Delete` calls it for every frame, so deleting a state with a `CreateObject`
frame frees the model object struct: do not use the model afterwards, and do not delete states that share a Cscript twice.
`Packed` Cscripts (made by `ObjectCore_Load`) live in one block and are released with a single free.

## Writing your own

```c
static int turns = 0;
static void count_turn(Object *self) { (void)self; turns++; }        /* shape 2 */

static void add_to_score(Object *self, MT2D_VAR **a)                 /* shape 3: a[0] += a[1] */
{
    MT2D_Object_ADD(a[0], a[1]);   /* a[0] must be a normal int/char/float var, see vars.md */
}

MT2D_ObjectState_ADD(state, sprite, 10, Cscript_New(count_turn));
```

For the text compiler register it by name: `ObjectCore_AddGlobalCScript("count_turn", count_turn)`,
`ObjectCore_AddGlobalCScriptVars(name, fn, argCount)`, `ObjectCore_AddGlobalCScriptObjVars(name, fn, argCount)` (see
[../Object-Scripts.md](../Object-Scripts.md#globals)). Register before loading the objects that use it.

## How arguments resolve (important)

Built in functions read args with `Cscript_VAR_Get_Integer(self, var)` and write with `Cscript_VAR_Set_Integer`:

| Arg is | Meaning |
| --- | --- |
| `VAR_INT` / `VAR_CHAR` / `VAR_FLOAT` | a value (float is truncated). If a function *writes* to it (`VarInc`, `VarAdd`, `SetVar` ...) it changes **that var itself**, so it behaves like a shared/global variable |
| `VAR_POINTER` (made with `MT2D_Object_Create_Var_Pointer(name)`) | "the var called `name` of the object running the script", looked up by `strcmp` on `self->User_Vars` every call (slow with many vars). `Pos_X`, `Acel_Y` ... are such names. Not found: reads as 0, writes are ignored (no error) |
| the **name** of a `VAR_POINTER` | also used as plain text: a **state name** (`SetState`, `JumpToState...`) or a **file name** (`PlaySound`, `PlayMusic`). A `VAR_STRING` var supplies its text instead |

The functions overwrite `Data.p` of a pointer var with `self` while they run and reset it to `0` afterwards, so a `VAR_POINTER`
arg always means the running object; do not share the same pointer var between objects that run on different threads.
The only function that writes into another object is `SetVar` when the first arg is a pointer var whose `Data.p` is already set
to that object.

## Built in functions (`STDFunctions.h`)

Order of `args` as the C function receives it. `a op b` jumps use the integer value of each arg. Names in the first column
are the text-script names (`CscriptFunctions.h`).

| Name | Shape | args | Does |
| --- | --- | --- | --- |
| `Move` | 2 | | `Pos_X += Acel_X; Pos_Y += Acel_Y` |
| `TeleportToCoord` | 3 | x, y | set `Pos_X`, `Pos_Y` |
| `SetState` | 3 | state name | go to the state (no effect if no state has that name) |
| `SetVar` | 3 | dst, src | `dst = src` (int/char/float, converted to dst's type) |
| `VarInc`, `VarDec` | 3 | var | `var += 1` / `-= 1` |
| `VarAdd`, `VarSub` | 3 | var, value | `var += value` / `-= value` |
| `RemoveObject` | 2 | | `CanDelete = true` |
| `JumpToStateIfVarEq` / `NotEq` / `Lower` / `Higher` / `LowerEq` / `HigherEq` | 3 | state, a, b | go to state if `a op b` (Eq compares `a == b`; the argument roles are `[1]`,`[2]`) |
| `JumpToStateIfRandom` | 3 | state, chance, max | if `rand() % max <= chance`. `max` 0 divides by zero. Lands on frame 1, skipping frame 0 |
| `JumpNextFrameIfVar...` | 3 | a, b | skip the next frame if `a op b`. Not for the last frame of a state. The compiler refuses them |
| `JumpFrames`, `JumpFramesIfVar`, `JumpFramesIfRandom` | 3 | jump (, a, op, b) / (jump, chance, max) | move the frame index; used by the compiler for `if`/`while`. `op` is a char: `<` `>` `=` `!`, 242 `>=`, 243 `<=` |
| `PlaySound`, `PlayMusic` | 3 | file | `MT2D_Play_Audio` / `MT2D_Play_Music` ([../general/audio.md](../general/audio.md)); silent when audio is not built |
| `GotoStateIfHitObject` | 4 | `other` = target object, args: state | go to state if `self` hits `other` (`other->Solid` must be true) |
| `CreateObject` | 4 | `other` = model, args: x, y, accelX, accelY | clone the model (shares its states, copies user vars from index 6) into `self->MyScene` |
| `Cscript_While`, `Cscript_While_End` | 3 | loop id, left, op, right / loop id | a loop over the frames between them, usable from C, see [While loops](#while-loops) |
| `Object_GotoState_IfHit` | 3 | state, damage, hp var name | old: hit any solid object of the scene, subtract damage from its var. Not in the compiler list |
| `Cscript_AddVar_ToObject` | 3 | var | add a copy of a var to `self`. Its duplicate check compares `args[i]` with user var `i`, so it is only valid while the object has at most one user var |

`CreateObject` requires `self->MyScene` (the caller must be in a scene) and uses the model's `Solid`, `RenderOnly` and `Size`.

## While loops

`Cscript_While` and `Cscript_While_End` are two frames of one state; the frames between them are the loop body.
The text compiler emits them for `while`, and C code can add them the same way (Super Shibe 3000's `Boss.cpp` does).

| Frame | args |
| --- | --- |
| `Cscript_While` | `[0]` loop id var, `[1]` left, `[2]` operator (a `VAR_CHAR`), `[3]` right |
| `Cscript_While_End` | `[0]` the **same** loop id var pointer |

```c
/* for (i = 0; i < 9; i++) { ... }   i is a pointer var to the object's user var "i" */
MT2D_VAR *loop = MT2D_Object_Create_Var_Int("", 0);        /* loop id: one per loop, value 0 = not started */
MT2D_ObjectState_ADD(st, NULL, 0, Cscript_New3(MT2D_VAR_Create_Matrix4(loop, iPointer, lowerOp, nine), &Cscript_While));
    /* ...body frames... */
    MT2D_ObjectState_ADD(st, NULL, 0, Cscript_New3(MT2D_VAR_Create_Matrix1(iPointer), &Cscript_VAR_INC));
MT2D_ObjectState_ADD(st, NULL, 0, Cscript_New3(MT2D_VAR_Create_Matrix1(loop), &Cscript_While_End));
```

* The loop id var is how the two frames find each other: the first run of `Cscript_While` stores its own frame index in it, and the
  matching end is the frame whose `args[0]` is that same pointer. Nested loops each need their own loop id var.
* The operator is a char var: `'<'`, `'>'`, `'='` or `'!'` (not equal). While the condition holds the body runs. When it fails the
  frame index moves to the `Cscript_While_End` frame. Any other operator value enters the body.
* `Cscript_While_End` jumps back to the `Cscript_While` frame, so the condition is checked again.
* Nothing in the loop changes the counter: add a `Cscript_VAR_INC` (or similar) frame in the body. Reset an inner counter (`SetVar`) before the
  inner `Cscript_While`, as `Boss.cpp` does.
* Each frame still waits its own wait value before the next one runs, so a body of `n` frames takes at least the sum of their waits per pass.

The text names and arities are the `X(Name, Symbol, Kind, Arity)` table in `CscriptFunctions.h`; the position in that list is the
id stored in `.mto` files, **only append at the end**.

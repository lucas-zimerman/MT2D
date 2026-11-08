# Object scripts

Object scripts describe an ObjectCore object (its vars, states, sprites and behaviour) in a
small text file instead of code:

```
object movingdot
{
    int Pos_X = 10;
    int speed = 2;

    states
    {
    Spawn:
        Show "SPRITEX"
        Wait 10
        Pos_X += speed
        if (Pos_X > 300) { Goto Done }
        Goto Spawn
    Done:
        Stop
    }
}
```

There are two ways to use a script:

* **Compile it on your machine** with `cscriptc` into a small binary `.mto` file, ship that and load
  it with `ObjectCore_Load`. The game only contains the small loader, no parser.
* **Load the text directly** with `ObjectCore_LoadScript` while developing.

The text compiler is optional (CMake option `MT2D_CSCRIPT_PARSER`, off by default for DJGPP, see
[Building](#building)).

## Contents

* [Quick start](#quick-start)
* [The language](#the-language)
* [How an object runs](#how-an-object-runs)
* [Loading objects](#loading-objects)
* [Globals](#globals)
* [Built in functions](#built-in-functions)
* [Building](#building)
* [Error messages](#error-messages)
* [Limits](#limits)

## Quick start

```
cscriptc movingdot.txt movingdot.mto
```

```c
#include <MT2D/ObjectCore/ObjectLoader.h>

Object *dot = ObjectCore_Load("movingdot.mto");
if (!dot) {
    puts(ObjectCore_GetLastError());   // why it failed
}
ObjectScene_Add(scene, dot);
// ... ObjectScene_Goto_NextSteps(scene); ObjectScene_Render(scene); ...
```

## The language

Comments are `// to the end of the line` and `/* block */`. Keywords (`object`, `states`, `Show`,
`Wait`, `while`, `if` ...) and function names are not case sensitive. Names of vars and states are,
and can have up to 18 characters.

### An object

```
object name
{
    solid;                  // optional flags
    renderonly;
    int Size_X = 32;        // vars
    int hp = 3;
    states { ... }
}
```

* `name` documents the file, the object is loaded by file name.
* `int` declares a var with a start value (default 0). The built in vars `Pos_X`, `Pos_Y`,
  `Size_X`, `Size_Y`, `Acel_X` and `Acel_Y` always exist, declaring one only sets its value.
* `solid;` and `renderonly;` set the matching flags of the object (hit tests only look at solid
  objects).

### States and frames

A state is a label followed by statements. **The first state is where the object starts.** When the
last frame of a state ends, the state starts again, so a state is a loop. Use `Goto` to change state.

```
Idle:
    Show "idle1.txt"
    Wait 5
    Show "idle2.txt"
    Wait 5
Dead:
    Stop
```

### Showing sprites and waiting

* `Show "file"` + `Wait n`: shows the sprite for `n` ticks (0 to 65535).
* `Show` belongs to the `Wait` **right after it**. A `Wait` that does not come right after a `Show`
  shows no sprite, and a `Show` that is not directly followed by `Wait` is an error.
* `Show none` shows nothing.
* Other kinds of sprite:
  * `Show "file"`: an ASCII sprite file
  * `Show image "file" 32 24`: an image file scaled to 32 x 24
  * `Show container "file" 32 24`: an image from the container, scaled to 32 x 24

The same file (and size) used many times is loaded once.

### Statements

Statements run in order, one after the other. They take no time by themselves, only `Wait` does.

```
x++              x--
x += 5           x -= y
x = 0            x = y
Move             // a function with no arguments, the parentheses are optional
VarAdd(x, 5)     // a function call
Goto Dead        // go to a state
Stop             // remove the object
```

An action runs **when the `Wait` before it ends**:

```
Show "A"
Wait 4          // A is shown for 4 ticks
x++             // then x++ runs
Show "B"
Wait 5          // then B is shown for 5 ticks
```

`;` after a statement is optional.

### while and if

Blocks always use braces.

```
while (y < 300 && !paused)
{
    Show "S" Wait 1
    y++
}

if (hp <= 0) { Goto Dead }
else if (hp < 10) { Show "hurt" Wait 5 }
else { Show "ok" Wait 5 }
```

A condition is made of comparisons (`<` `>` `==` `=` `!=` `>=` `<=`) between a var and a var or a
number, combined with `&&`, `||`, `!` and parentheses. A lone var or number is true when it is not
0. `&&` and `||` stop at the first answer, like in C.

`random(p)` is true `p` percent of the time (a number from 0 to 100):

```
if (random(5)) { global SpawnProjectile() }
```

A state label cannot be inside a block, use `Goto` to leave one.

### Globals

`global` marks a var, an object or a function that **the game** registers, see [Globals](#globals).

```
Pos_X -= global GameSpeed
global DogHealth--
if (global JumpRequest == 1) { Goto Jump }
global SpawnSmoke()
GotoStateIfHitObject(global Doggo, hit)
```

## How an object runs

`Object_Goto_NextStep` (or `ObjectScene_Goto_NextSteps`) is called once per tick.

* The object counts down the wait of its current frame. When it reaches 0 the frame's action runs
  and the object moves to the next frame.
* Frames with `Wait 0` (and everything the compiler makes for `if`/`while`/`Goto`) are run in the
  same tick, one after the other.
* A loop where **no** frame waits never gives control back to the game, that is up to the script.
  This includes a `Stop` (or `Goto`) that comes right after a `while` and is not preceded by a `Wait`: the state
  restarts through frames that do not wait. Put a `Wait 1` before it (`Show none Wait 1 Stop`).
* To keep the memory small the compiler puts an action in the frame of the `Wait` before it when
  nothing jumps between them. This does not change what a script does.

## Loading objects

```c
#include <MT2D/ObjectCore/ObjectLoader.h>

Object *ObjectCore_Load(const char *file);                       // NULL on error
Object *ObjectCore_LoadFromMemory(const unsigned char *data, unsigned long size);
Object *ObjectCore_Clone(Object *model, int x, int y);           // a new object, same behaviour
void    ObjectCore_Unload(Object *model);
const char *ObjectCore_GetLastError(void);                       // why a load failed
```

A loaded object is a **model**: its states are shared data. Make the objects you put in the scene
with `ObjectCore_Clone`; a clone shares the states of the model and has its own copy of the flags,
size, acceleration and vars (with the values the model has when you clone it).

```c
Object *model = ObjectCore_Load("tile.mto");
Object *a = ObjectCore_Clone(model, 0, 0);
Object *b = ObjectCore_Clone(model, 102, 0);
...
Object_Delete(a, false);       // clones do not own the states
Object_Delete(b, false);
ObjectCore_Unload(model);      // last: frees the states, the model object and its sprites
```

Do not call `ObjectCore_Unload` while clones are still in a scene, they use its states.

With the text compiler built in, `ObjectCore_LoadScript("tile.txt", error, sizeof(error))` compiles and
loads a text file in one call (`error` is a `char` buffer, it receives the line and the reason, it can
be `NULL`). `CscriptCompiler_Compile` gives you the `.mto` bytes in memory instead.

## Globals

Scripts cannot know about your game's own vars, objects and functions, so you register them by
name **before** loading the objects that use them. The registry does not copy anything.

```c
#include <MT2D/ObjectCore/ObjectGlobals.h>

ObjectCore_AddGlobalVar(GameSpeed);                       // the name is GameSpeed->Name
ObjectCore_AddGlobalObject("Doggo", doggo);               // used as global Doggo
ObjectCore_AddGlobalCScript("SpawnSmoke", SpawnSmoke);    // void f(Object *caller)
ObjectCore_AddGlobalCScriptVars("GetCash", GetCash, 1);   // void f(Object *caller, MT2D_VAR **args), 1 argument
ObjectCore_AddGlobalCScriptObjVars("Spawn", Spawn, 3);    // void f(Object *caller, Object *other, MT2D_VAR **args)
```

* The signatures a function can have are the ones of Cscript: `void f(Object*)`
  (`ObjectCore_AddGlobalCScript`), `void f(Object*, MT2D_VAR**)` (`...Vars`) and
  `void f(Object*, Object*, MT2D_VAR**)` (`...ObjVars`). The last
  number is how many arguments a script has to write (-1 accepts any amount) (the object counts as the first one in the
  last signature); the load fails if a script does not match.
* Add returns `false` for a name that exists or is longer than 18 characters.
* Each kind (vars, objects, functions) has its own names.
* A name that is not registered when an object is loaded makes the load fail:
  `ObjectCore_GetLastError()` returns for example `global var is not registered: 'GameSpeed'`.
* Loading looks the name up once. The object keeps the pointer, nothing is searched while it runs,
  and it sees the **current** value of a global var every time.
* `ObjectCore_RemoveGlobalVar`, `...RemoveGlobalObject` and `...RemoveGlobalCScript` always work.
  Removing something that a loaded object still uses (or freeing what it points to) is up to you:
  unload or stop those objects first.
* **Sprites** need no word in the script. Register a sprite under the path a script writes in `Show`
  and the loader uses it instead of loading the file (any kind: `Show "a.txt"`, `Show image "a.png" 2 2`,
  `Show container "a.png" 2 2`):

  ```c
  ObjectCore_AddGlobalSprite("smoke.txt", Load_Sprite("smoke.txt"));   // every Show "smoke.txt" now shares it
  ...
  ObjectCore_RemoveGlobalSprite("smoke.txt");   // or ObjectCore_ClearGlobalSprites() on shutdown
  ```

  The registry holds a reference of its own, so unloading the objects never frees the sprite and
  you must not call `Sprite_Delete` on it. Removing it is always safe: the sprite is freed when the
  last state that uses it is gone. A registered image sprite keeps its own scale (the `w h` in the
  script are ignored). A path is up to 255 characters, and a sprite registered after an object was
  loaded is not seen by that object.
* An object keeps the pointer it found when it was loaded. If you create a new `Doggo` later, load
  the objects that refer to it again.

## Built in functions

Call them by name (`VarAdd(x, 5)`). Most of them have a nicer statement form.

| Function | Arguments | Does | Statement form |
| --- | --- | --- | --- |
| `Move` | | `Pos += Acel` | |
| `TeleportToCoord` | `x, y` | sets the position | |
| `SetState` | `state` | goes to a state | `Goto state` |
| `SetVar` | `var, value` | sets a var | `var = value` |
| `VarInc`, `VarDec` | `var` | adds / subtracts 1 | `var++`, `var--` |
| `VarAdd`, `VarSub` | `var, value` | adds / subtracts | `var += v`, `var -= v` |
| `RemoveObject` | | marks the object to be deleted | `Stop` |
| `PlaySound`, `PlayMusic` | `"file"` | plays a file | |
| `JumpToStateIfVarEq`, `...NotEq`, `...Lower`, `...Higher`, `...LowerEq`, `...HigherEq` | `state, a, b` | goes to the state if `a` op `b` | `if (a op b) { Goto state }` |
| `JumpToStateIfRandom` | `state, chance, max` | goes to the state if `rand() % max <= chance` | `if (random(p)) { Goto state }` is the same idea in percent |
| `GotoStateIfHitObject` | `global Object, state` | goes to the state if this object hits the other one (the other object has to be `solid`) | |
| `CreateObject` | `global Model, x, y, accelX, accelY` | adds a clone of the model to the scene of the caller | |

`JumpFrames…` and `JumpNextFrame…` exist in the engine but count frames, which the compiler moves
around: write `if` and `while` instead. The compiler refuses to compile them.

The exact list (the position of each is the number stored in `.mto` files) is
`MT2D/ObjectCore/CscriptFunctions.h`.

## Building

| What | How |
| --- | --- |
| Loader and global registry | Always built, plain C. Files `ObjectCore/ObjectLoader.*`, `ObjectCore/ObjectGlobals.*`. |
| Text compiler | CMake option `MT2D_CSCRIPT_PARSER` (ON by default, OFF for DJGPP). Plain C, files in `ObjectCore/Parser/`. Defines `MT2D_USING_CSCRIPT_PARSER`. |
| `cscriptc` tool | Built by `tests/CMakeLists.txt` when the option is ON: `cscriptc in.txt out.mto`. |
| Android | `Android.mk` builds the loader and the registry; it does not build `Parser/`. |
| Your build system | Compile `.c` files too, with C99 (`-std=gnu99`). |

The tests (`tests/test_objectloader.cpp`, `test_objectglobals.cpp`, `test_cscriptcompiler.cpp`)
double as examples of every feature.

## Error messages

The compiler says where the problem is: `movingdot.txt:line 7: unknown function 'Bogus'`.
The most common ones:

| Message | Meaning |
| --- | --- |
| `unknown function 'X'` | not a built in function, use `global X()` for your own |
| `X needs N argument(s), got M` | wrong number of arguments |
| `'x' is not a var or a state` | a name that was never declared |
| `Show must be directly followed by Wait` | see [Showing sprites](#showing-sprites-and-waiting) |
| `'X' counts frames, use if / while instead` | `JumpFrames…` or `JumpNextFrame…` called by hand |
| `X needs a global object as its first argument` | write `global Name` |
| `a state label cannot be inside a block` | use `Goto` |
| `name 'x' is longer than 18 characters` | shorten it |

Loading problems (`ObjectCore_GetLastError`): `global var is not registered: 'X'`,
`global object is not registered: 'X'`, `global function is not registered: 'X'`,
`sprite could not be loaded: 'path'`, `wrong argument count for the function in state 'S'`.

## Limits

| | |
| --- | --- |
| Names of vars, states and globals | 18 characters |
| `Wait` | 0 to 65535 ticks |
| States per object / frames per state | 65535 |
| `int` declarations per object | 255 |
| Sprites per object | 65535 |
| Text arguments and sprite paths | 255 characters |
| Numbers | 32 bit integers |

The binary file format is described in `MT2D/ObjectCore/ObjectLoader.h`.

# Scripting and game objects (ObjectCore)

Sprites, objects, states, scenes, Cscript functions and `MT2D_VAR` vars. **All of it works in text mode
too**, not only with SDL: ASCII sprites are blitted into `WINDOW1` and an object's `Pos_X` / `Pos_Y` are then
text cells (column, row). With SDL or the DOS VGA mode the same objects can also show image sprites, and
then the position is in 320x240 pixels.

Code: `MT2D/ObjectCore/` (the loader and the global registry are C, the rest is C++ with `extern "C"` headers).

## How the pieces fit

```
Sprite  ----------------------+
(ASCII or image)              |
                              v
ObjectState = frames: [ sprite, wait (ticks), Cscript or none ] ...    (shared, read-only)
                              |
Object = position/size/accel + MT2D_VAR list + states + current state/frame
                              |
ObjectScene = group of objects: ObjectScene_Goto_NextSteps() then ObjectScene_Render()
```

* **Cscript** (`cscript.md`) = a C function pointer + the `MT2D_VAR`s it receives. It runs between frames.
* **Vars** (`vars.md`) = the one small typed value used for object properties, script arguments and globals.
* There are two ways to describe objects:
  1. **In C**: build states and `Cscript`s by hand (this folder). Example: `examples/sprites`, `examples/cscript`.
  2. **In a text script** compiled to `.mto`: [../Object-Scripts.md](../Object-Scripts.md) (the language,
     `cscriptc`, `ObjectCore_Load`, globals). Example: `examples/desert`. The script compiler produces the same
     structures as option 1.

## Files here

| File | Content |
| --- | --- |
| [sprites.md](sprites.md) | ASCII and image sprites, the text sprite file format, transparency |
| [objects.md](objects.md) | objects, states, scenes: the per tick algorithm, lifecycle, a text mode example |
| [cscript.md](cscript.md) | making and writing Cscript functions, the built in function table with argument order |
| [vars.md](vars.md) | `MT2D_VAR`: types, creation, conversion, errors, how script arguments resolve |

## Include order

`Object.h` and `State.h` include each other: include `Object.h` first (as the examples do). Include
letter case must match the file names.

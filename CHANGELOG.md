# Changelog

## Unreleased

### Breaking

- `MT2D_VAR.Data` is now an inline union (`Data.i`, `Data.c`, `Data.f`, `Data.p`) and `MT2D_VAR.Name` an inline `char[MT2D_VAR_NAME_MAX]`: one allocation per var instead of three, see [MIGRATION.md](MIGRATION.md).
- Var names are limited to `MT2D_VAR_NAME_MAX - 1` (18) characters. Longer names are rejected: the constructors return `NULL` and set `Var_Name_Too_Long`. Store long texts (e.g. sound paths) in a `VAR_STRING` var.
- `MT2D_Object_Set_Var(Var, void*, Type)` copies the value instead of taking ownership of the pointer, and the `int`/`float`/`bool` overloads now store their value. The `char*`, `unsigned` and the never implemented `Create_Var_UInt/UChar` were removed.
- `MT2D_Object_Create_Var` returns `NULL` (with `Var_Null_Data`/`Var_Null_Name`) on bad input.
- `Cscript_Play*FromContainer` no longer cache the loaded file inside the var.
- `bool` and `unsigned` var types were removed from MT2D_Var.
- `Sprite` no longer has `Data`/`RotatedTexture`, ASCII sprites use a compact slice image and SDL images use `MT2D_Internal_Image`, see [MIGRATION.md](MIGRATION.md) ([#4](https://github.com/lucas-zimerman/MT2D/issues/4)).
- Engine `FILE` references were replaced by `MT2D_FILE`.
- File seek now accepts `long` values, and container coordinates are now `long`.
- MS-DOS code was refactored (files renamed to the 8.3 format, e.g. `M2Render.cpp`, `M2Init.cpp`).
- User_Vars is always initialized and you should add additional variables using Object_Add_Variable or Object_Add_Variables.
- If you replace the content of User_Vars, ObjectCore will not work when dealing with primary variables.
- To Capture the primitive parameters you can use one of the following methods:

method1:
```c
//Set 12 to the posX coord
Object.User_Vars[Object_PosX_Var_Index].Data.i = 12;
```
method2:
```c
ObjectPosX(Object) = 12;
```

### Features

- Object files: `ObjectCore_Load("movingdot.mto")` ([#3](https://github.com/lucas-zimerman/MT2D/issues/3)) loads an object (vars, states, frames, sprites and Cscript calls) from a compact binary file. The loader has no parser and no function name table, reads the file in one pass and allocates every array with its final size, one allocation per frame. Release it with `ObjectCore_Unload`.
- Text compiler for those files, `MT2D/ObjectCore/Parser` (CMake option `MT2D_CSCRIPT_PARSER`, off by default for DJGPP): a DECORATE like format with `int` vars, states, `"sprite" wait Action(args)` frames, `Goto` and `Stop`, `while`, `if`/`else if`/`else` with `&&`, `||`, `!` and parentheses (compiled to the relative frame jumps, so the `.mto` format and the loader did not change), see `MT2D/ObjectCore/Parser/CscriptCompiler.h`. Use the `cscriptc` tool (built by `tests/`) or `ObjectCore_LoadScript` to compile it.
- `CscriptFunctions.h` lists the built in Cscript calls available to compiled objects (the position in the list is the id stored in files, only append).
- `Cscript.Packed` marks a Cscript stored in one block with its vars, `Cscript_RemoveFromMemory` frees it with a single `free`.
- New relative frame jumps `Cscript_JumpFrames` and `Cscript_JumpFramesIfVar` (any of `<`, `>`, `=`, `!`, `>=`, `<=`), the building blocks for if/else and loops inside a state.
- New Cscript conditions `Cscript_JumpToStateIfVar{Lower,Higher,NotEq}` and `Cscript_JumpNextFrameIfVar{Lower,Higher,NotEq}` (`<`, `>`, `!=`), completing the set next to the existing `==`, `<=` and `>=`.
- `VAR_STRING` vars (`MT2D_Object_Create_Var_String`, `Set_Var_String`, `Get_Var_String`) and `MT2D_Object_VAR_GetPath`; `Cscript_PlaySound`/`PlayMusic` read the file from them
- `MT2D_Object_Create_Var_Float`, float support in `MT2D_Object_ADD/SUB` and `MT2D_VAR_CLONE`; `MT2D_Object_VAR_GetInt/GetBool` are now inline
- Add `MT2D_Types.h` (fixed width types), `Sprite_Create_Ascii_From_Cells` and `Sprite_Get_Char`, ASCII sprites no longer store transparent cells (saves RAM on MS-DOS)
- Add common object parameters as variables (Acceleration, Position, Size) ([#7](https://github.com/lucas-zimerman/MT2D/pull/7))
- Add functions Object_Add_Variable and Object_Add_Variables ([#7](https://github.com/lucas-zimerman/MT2D/pull/7))
- Add Emscripten support (no audio yet) and a joystick implementation for it ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add a custom CMakeLists to help building MT2D projects ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add new STDFunctions: JumpFramesIfRandom, While and WhileEnd (beta) ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add Object_GotoState_IfHitObject ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add Cscript functions for playing sounds/music from a container ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add support for audio playback from memory ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add option to get a MT2D Var as a bool ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add an extra MT2D_FILE layer; SDL handles the files when SDL is in use ([#5](https://github.com/lucas-zimerman/MT2D/pull/5))
- Add Android joystick logic ([#5](https://github.com/lucas-zimerman/MT2D/pull/5))
- Add initial Open Watcom support for MS-DOS, plus MS-DOS joystick, keyboard, render and system call implementations ([#8](https://github.com/lucas-zimerman/MT2D/pull/8))
- Add Container test case (`TestCase/ContainerTest`) ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add VSYNC with SDL to avoid unstable framerates ([#8](https://github.com/lucas-zimerman/MT2D/pull/8))
- Add a specific debug tag for ObjectCore, and notes to help find out why sprite rendering or adding fails ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add logs to SDL_Audio ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- SDL now finds fonts inside the assets folder by default ([#8](https://github.com/lucas-zimerman/MT2D/pull/8))
- Container refactor: data is no longer fully stored in memory, and it supports temporary files, updating, inserting and removing files, getting where the data is located, and clearing data/containers ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add functions to create a matrix of vars of any size ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Add MS-DOS tooling notes (`Tools/msdos/readme.txt`) ([#8](https://github.com/lucas-zimerman/MT2D/pull/8))
- Add `examples/` with ten CMake based examples (`helloworld`, `inputpopup`, `menu`, `playpcspeaker`, `playsound`, `playmusic`, `asciigraphics`, `graphics`, `cscript`, `sprites`), a DJGPP toolchain file and a CI workflow ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Add `Cscript_New(fn, ...)`, a single constructor for every kind of Cscript function that accepts any number of vars; `Cscript_New1`..`4` still work, see [MIGRATION.md](MIGRATION.md) ([#22](https://github.com/lucas-zimerman/MT2D/pull/22))

### Fixes

- `Cscript_Empty` now initialises `_void`.
- Better memory management in MT2D_Vars and ObjectCore; objects are properly cleared when removed from the object scene ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Allow the use of NULL as functions ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Set SDL Audio to 44KHz ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Fix SDL Log and internal SDL paths ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Fix const warnings and a state crash ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Fix lower/upper case typos in includes ([#8](https://github.com/lucas-zimerman/MT2D/pull/8))
- Fix memory leaks and unfreed memory in the container code ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Fix a problem in the encryption part that removed data from the decrypted content ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Fix file reading/writing typo in containers and an include overflow ([#6](https://github.com/lucas-zimerman/MT2D/pull/6))
- Fix SDL issues for MS-DOS/SDL builds and the full path handling ([#9](https://github.com/lucas-zimerman/MT2D/pull/9))
- Code cleanup (tabs replaced by spaces, unused code removed) ([#8](https://github.com/lucas-zimerman/MT2D/pull/8))
- Fix memory leaks in the container, ObjectCore, input boxes and SDL code, plus a double free and a heap overflow in containers ([#16](https://github.com/lucas-zimerman/MT2D/pull/16))
- Fix the Linux/macOS terminal (curses) backend: it never refreshed the screen, the clear/draw macros were swapped, `kbhit` ate keys, delays were 1000x too short and macOS did not build ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Fix a missing `#` in `MT2D_Win_Debug.h` that broke every Windows terminal build ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Fix backslashes in include paths and `Objectcore` letter case so the engine builds on Linux and macOS ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Add `MT2D_SDL_Joystick_Init`/`Close`/`Update`, so the SDL joystick API works on desktop (arrow keys act as the left stick and held keys are tracked) ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Fix SDL key releases that happened between two frames being lost (quick taps were never seen by the keyboard buffer) ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- SDL builds now start in the executable folder, so the font and images are found when the program is launched from another directory ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Add a silent `SDL_Beep` when SDL audio is disabled, so SDL builds link without SDL2_mixer ([#20](https://github.com/lucas-zimerman/MT2D/pull/20))
- Fix CP850 support in the Linux/macOS terminal (curses) backend: the screen buffers are now converted to Unicode when drawn (accents, box drawing, blocks and DOS symbols were blank or approximated) and typed characters are converted back to CP850. Needs `ncursesw` and a UTF-8 locale ([#21](https://github.com/lucas-zimerman/MT2D/pull/21))
- Remove `#pragma region`/`endregion` from `MT2D_Terminal_Define.h`, which DJGPP's GCC rejects with `-Werror` ([#22](https://github.com/lucas-zimerman/MT2D/pull/22))

## 2.0.1 - 2019-03-24

### Features

- Add ObjectCore update with a simple script language (alpha), including the Cscript function "add var to object".
- Add dynamic data types (Vars).
- Add Container support, integrated with Load Image; it can load data from memory.
- Add an inputbox password.
- Add basic touchscreen, accelerometer (Android/iOS) and joystick support.
- Rework SDL: split into separate files, add a main event handler, dynamic resolution (smartphones in portrait), and optional SDL render and audio.
- SDL now loads `msdos_font` from the software folder, falling back if it fails.
- Move OS specific code into a separate folder for each OS.
- Windows joystick now works with keyboard and accelerometer (needs testing).
- Add a new FILE struct system to help handle files; SDL handles files when in use.
- Add a License file, README and index page.
- Add basic documentation on creating a MT2D Android Studio project.
- Add a new function to create a matrix of vars with any size.
- Small performance improvement when creating vars.

### Breaking

- SDL is disabled by default.
- Unsigned vars were removed.

### Fixes

- Fix include files folder and includes related to SDL.
- Fix bugs in the SDL system.
- Fix UWP detection (`wrl.h` only exists in UWP) without affecting win32.
- Fix compile problems with SDL on Windows and for MS-DOS.
- Fix android.mk for the newest Android Studio and many ANDROID include files.
- Fix SDL_SOUND_TYPE comparison with -1, which prevented the audio system from starting.
- Fix Android joystick support that was using the MS-DOS code.
- Fix temporary objects not being cleaned when creating a new function.
- Fix Android Studio string warnings.
- Add some memory clearing.

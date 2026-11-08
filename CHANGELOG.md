# Changelog

## Unreleased

### Breaking

- The engine is now written in C: every `.cpp` of `MT2D/` outside `ObjectCore/` became a `.c` file (same names, so build lists must be updated), and the headers have C linkage (`extern "C"`), so C++ projects keep working. `ObjectCore` is still C++ for now, see [MIGRATION.md](MIGRATION.md) ([#29](https://github.com/lucas-zimerman/MT2D/pull/29))
- `close()` in `SDL/MT2D_SDL_main` was renamed `MT2D_SDL_Close()` (it clashed with `close()` from `<unistd.h>` in C) ([#29](https://github.com/lucas-zimerman/MT2D/pull/29))
- The Windows UWP accelerometer code (C++/CX) moved from `_WINDOWS/IO/MT2D_Win_Joystick` to `_WINDOWS/IO/MT2D_Win_Uwp.cpp`, the only engine file outside `ObjectCore` that stays C++; UWP projects must add it to the build ([#29](https://github.com/lucas-zimerman/MT2D/pull/29))
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
- MS-DOS VGA: `MSDOS_USE_DOUBLE_BUFFER` no longer works (it is accepted but ignored): the renderer has no double buffer anymore and writes each changed 4x8 tile once, without flicker. Remove the define from your build flags and expect 64 KB less RAM use. `MSDOS_USE_SPRITE_TILE_RENDER` (the tile renderer is the only one now), `MT2D_Dos_Render_Sprites()`, the `DOUBLE_BUFFER`, `SpriteTiles` and `ChangedMasks` variables and `VGAsprTi.h` were removed; code that included or used them must be updated, see [MIGRATION.md](MIGRATION.md) ([#37](https://github.com/lucas-zimerman/MT2D/pull/37))

### Features

- MS-DOS: music with DJGPP, IMF songs (type 0 and 1) played on the OPL2 chip (Adlib / Sound Blaster). `MT2D_Play_Music("x.ogg")` loops `d/x.imf` (8.3 name, like the images), `MT2D_PlayMusicFromMemory` plays a song from memory, and `MT2D_Audio_Init()` returns 1 when an OPL chip is found. A timer interrupt plays the song and keeps the DOS clock at its normal speed. Sound effects (`MT2D_Play_Audio`) are still ignored on MS-DOS. ([#32](https://github.com/lucas-zimerman/MT2D/pull/32))
- `MT2D/Audio/MT2D_Music.{h,c}`: portable IMF decoder in C. Songs that fit the buffer (32 KB, `MT2D_DOS_MUSIC_BUF`) are loaded whole, bigger ones are streamed from the disk or from a container through a ring buffer (`MT2D_Music_Update`, called by `MT2D_System_Delay` on MS-DOS). Songs are not decrypted. ([#32](https://github.com/lucas-zimerman/MT2D/pull/32))
- `MT2D_Container_Read(id, pos, dst, n)` reads a piece of a container file without loading it all (no decryption). ([#32](https://github.com/lucas-zimerman/MT2D/pull/32))
- The CI compiles the music files with DJGPP, and `test_music` covers the decoder. ([#32](https://github.com/lucas-zimerman/MT2D/pull/32))
- MS-DOS: sound effects with DJGPP. `MT2D_Play_Audio("coin.ogg")` plays `d/coin.wav` (8.3 name, PCM mono 8 or 16 bit, a container file wins over a loose file) and `MT2D_PlayAudioFromMemory` plays a WAV from memory, mixed in software (3 voices) and sent to the Sound Blaster by auto-init DMA at 22050 Hz, 8 bit. The card is found with the `BLASTER` variable and needs a DSP 2.00 or newer; `MT2D_Audio_Init()` now returns 1 when a Sound Blaster or an OPL chip is found ([#33](https://github.com/lucas-zimerman/MT2D/pull/33))
- `MT2D/Audio/MT2D_Mixer.{h,c}`: portable software mixer in C for 8 bit and 16 bit voices (clamped sum, volume per voice and master, the same sound played again restarts its voice, a full mixer replaces the voice with the least left to play) ([#33](https://github.com/lucas-zimerman/MT2D/pull/33))
- `MT2D/Audio/MT2D_Wav.{h,c}`: WAV reader (PCM mono, 8/16 bit) and rate converter used to load the sounds ([#33](https://github.com/lucas-zimerman/MT2D/pull/33))
- `playsound` and `desert` play their sound effects on MS-DOS (`d/*.wav`, also written by their asset scripts), and the CI compiles the mixer, the WAV reader and the Sound Blaster code with DJGPP ([#33](https://github.com/lucas-zimerman/MT2D/pull/33))

- MS-DOS: experimental VGA mode (`MSDOS_USE_GRAPHICS`, mode 13h 320x200 with 256 colors) that draws the text screen with a bitmap font and image sprites from 8 bit BMP files (palette index 0 is transparent). It works with DJGPP (near pointer video memory) and is turned on in the examples with `-DMT2D_DOS_VGA=ON`. ([#30](https://github.com/lucas-zimerman/MT2D/pull/30))
- MS-DOS: `Load_Sprite_Image("x.png")` loads `d/x.bmp` (8.3 name), and `examples/tools/png2dos.py` converts the example images. `graphics`, `cscript`, `sprites` and `desert` can be built for DJGPP in this mode. ([#30](https://github.com/lucas-zimerman/MT2D/pull/30))
- The CI compiles the VGA files with DJGPP, and the ANSI C check allows C++ under `MT2D/_MSDOS/` for now. ([#30](https://github.com/lucas-zimerman/MT2D/pull/30))

- New `desert` example: a scripted demoscene (monster truck, sand worm, radar and particles are object scripts) showing globals, `CreateObject`, `GotoStateIfHitObject`, `random()` and `PlayMusic`/`PlaySound` (original chase theme and sound effects). ([#27](https://github.com/lucas-zimerman/MT2D/pull/27))
- Global sprites: `ObjectCore_AddGlobalSprite(path, sprite)` registers a sprite that the object loader uses instead of loading the file a script has in `Show`, see [Object-Scripts.md](Documentation/Object-Scripts.md#globals). The registry holds its own reference, `ObjectCore_RemoveGlobalSprite` and `ObjectCore_ClearGlobalSprites` release it and `ObjectCore_CountGlobalSprites` returns how many are registered. ([#28](https://github.com/lucas-zimerman/MT2D/pull/28))
- New Cscript functions: `Cscript_JumpToStateIfVar{Lower,Higher,NotEq}`, `Cscript_JumpNextFrameIfVar{Lower,Higher,NotEq}` (`<`, `>`, `!=`) and the relative frame jumps `Cscript_JumpFrames` and `Cscript_JumpFramesIfVar` (any of `<`, `>`, `=`, `!`, `>=`, `<=`). ([#26](https://github.com/lucas-zimerman/MT2D/pull/26))
- Object scripts: describe an object (vars, states, sprites, behaviour) in a text file, compile it with `cscriptc` and load it with `ObjectCore_Load("movingdot.mto")`, see [Object-Scripts.md](https://github.com/lucas-zimerman/MT2D/blob/master/Documentation/Object-Scripts.md) ([#3](https://github.com/lucas-zimerman/MT2D/issues/3)) ([#26](https://github.com/lucas-zimerman/MT2D/pull/26))
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
- MS-DOS VGA: the renderer writes each changed 4x8 tile once to the video memory, so the 64 KB double buffer is not needed anymore, it makes no `malloc`/`free` per frame and uses about 40 KB less static memory. The spaceship game runs about 1.8x faster in DOSBox. Optional `-DMSDOS_VGA_WAIT_RETRACE` waits the vertical retrace before drawing. ([#37](https://github.com/lucas-zimerman/MT2D/pull/37))

### Fixes

- The PC speaker emulation of the SDL backend was almost silent: the sine was cast to an integer before the amplitude was applied. ([#35](https://github.com/lucas-zimerman/MT2D/pull/35))
- Emscripten: `MT2D_System_Beep` no longer waits for the tone to end (it froze the page), the beeps are queued. ([#35](https://github.com/lucas-zimerman/MT2D/pull/35))
- Emscripten: arrow keys stayed pressed forever with the joystick code (key releases were never applied), and `Mapped_Button` was not initialised. ([#35](https://github.com/lucas-zimerman/MT2D/pull/35))
- MS-DOS VGA mode: `MT2D_Dos_Draw_Window`, `MT2D_Dos_Clear_Main_Window` and the sprite/palette functions had C++ linkage (their `.cpp` files did not include their `extern "C"` headers), so VGA programs did not link. ([#32](https://github.com/lucas-zimerman/MT2D/pull/32))

- `Cscript_Empty` now initialises `_void`. ([#26](https://github.com/lucas-zimerman/MT2D/pull/26))
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
- MS-DOS VGA: sprites that were not aligned to the 4x8 tile grid missed their last row/column of tiles, and the tile change detection (a 16 bit hash with a `>5` typo) could skip a redraw, leaving ghost pixels ([#37](https://github.com/lucas-zimerman/MT2D/pull/37))

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

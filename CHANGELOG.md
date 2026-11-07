# Changelog

## Unreleased

### Breaking

- `bool` and `unsigned` var types were removed from MT2D_Var.
- Engine `FILE` references were replaced by `MT2D_FILE`.
- File seek now accepts `long` values, and container coordinates are now `long`.
- MS-DOS code was refactored (files renamed to the 8.3 format, e.g. `M2Render.cpp`, `M2Init.cpp`).
- User_Vars is always initialized and you should add additional variables using Object_Add_Variable or Object_Add_Variables.
- If you replace the content of User_Vars, ObjectCore will not work when dealing with primary variables.
- To Capture the primitive parameters you can use one of the following methods:

method1:
```c
//Set 12 to the posX coord
*(int*)Object.User_Vars[Object_PosX_Var_Index].Data = 12;
```
method2:
```c
ObjectPosX(Object) = 12;
```

### Features

- Add common object parameters as variables (Acceleration, Position, Size).
- Add functions Object_Add_Variable and Object_Add_Variables.
- Add Emscripten support (no audio yet) and a joystick implementation for it.
- Add a custom CMakeLists to help building MT2D projects.
- Add new STDFunctions: JumpFramesIfRandom, While and WhileEnd (beta).
- Add Object_GotoState_IfHitObject.
- Add Cscript functions for playing sounds/music from a container.
- Add support for audio playback from memory.
- Add option to get a MT2D Var as a bool.
- Add an extra MT2D_FILE layer; SDL handles the files when SDL is in use.
- Add Android joystick logic.
- Add initial Open Watcom support for MS-DOS, plus MS-DOS joystick, keyboard, render and system call implementations.
- Add Container test case (`TestCase/ContainerTest`).
- Add VSYNC with SDL to avoid unstable framerates.
- Add a specific debug tag for ObjectCore, and notes to help find out why sprite rendering or adding fails.
- Add logs to SDL_Audio.
- SDL now finds fonts inside the assets folder by default.
- Container refactor: data is no longer fully stored in memory, and it supports temporary files, updating, inserting and removing files, getting where the data is located, and clearing data/containers.
- Add functions to create a matrix of vars of any size.
- Add MS-DOS tooling notes (`Tools/msdos/readme.txt`).

### Fixes

- Better memory management in MT2D_Vars and ObjectCore; objects are properly cleared when removed from the object scene.
- Allow the use of NULL as functions.
- Set SDL Audio to 44KHz.
- Fix SDL Log and internal SDL paths.
- Fix const warnings and a state crash.
- Fix lower/upper case typos in includes.
- Fix memory leaks and unfreed memory in the container code.
- Fix a problem in the encryption part that removed data from the decrypted content.
- Fix file reading/writing typo in containers and an include overflow.
- Fix SDL issues for MS-DOS/SDL builds and the full path handling.
- Code cleanup (tabs replaced by spaces, unused code removed).

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

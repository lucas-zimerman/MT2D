# MT2D tests

Automated tests for the portable part of the engine (terminal mode, no SDL).

```bash
cmake -S tests -B build/tests
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

Needs a C++11 and a C99 compiler and CMake 3.10+ (the tests are C++, the new engine code is C, see AGENTS.md). No external test library is used,
`framework/mt2d_test.h` is a small header-only runner.

| Test file | Engine code covered |
| --- | --- |
| `test_vars.cpp` | `ObjectCore/Vars.c` |
| `test_state.cpp` | `ObjectCore/State.c` |
| `test_spriteblock.cpp` | `ObjectCore/SpriteBlock.c` |
| `test_sprites.cpp` | `ObjectCore/Sprites.c` |
| `test_cscript.cpp` | `ObjectCore/Cscript.c` |
| `test_object.cpp` | `ObjectCore/Object.c` (objects and scenes) |
| `test_stdfunctions.cpp` | `ObjectCore/STDFunctions.c` (built in Cscript functions, audio calls are no-ops) |
| `test_objectloader.cpp` | `ObjectCore/ObjectLoader.c` (compiled `.mto` files, truncated/invalid input) |
| `test_objectglobals.cpp` | `ObjectCore/ObjectGlobals.c` (registry of global vars, objects and functions) |
| `test_cscriptcompiler.cpp` | `ObjectCore/Parser/CscriptCompiler.c` (only with `-DMT2D_CSCRIPT_PARSER=ON`, the default) |
| `test_container.cpp` | `Container/MT2D_Container.c` |
| `test_aes.cpp` | `Container/aes.c` (NIST known answers) |
| `test_display.cpp` | display buffers in `MT2D.c` |
| `test_settings.cpp` | `MT2D_Settings.c` (result codes, operation ranges, the `MT2D_UNSUPPORTED` / `MT2D_INVALID` fallbacks used when a platform has no backend; built with `MT2D_SETTINGS_NO_PLATFORM`) |
| `test_frame.cpp` | `_MSDOS/M2Frame.c` (frame pacing: drift, late frames, work time; the clock and the sleep are simulated with `MT2D_FRAME_EXTERNAL`) |
| `test_frame_sdl.cpp` | `SDL/MT2D_SDL_Frame.c` (the same pacing for SDL, also across the 32 bit clock wrap; simulated with `MT2D_FRAME_SDL_EXTERNAL`, no SDL needed) |
| `test_layout.cpp` | `MT2D_Layout.c`, `Tools/Video/MT2D_Generic_Line.c` |
| `test_touch.cpp` | `Touch/MT2D_Touch.c` (buttons, Fixed / Reactive sticks, taps, show modes, bind errors; the fingers are fed with `MT2D_Touch_Finger_*`, built with `MT2D_USE_TOUCH`) and the getters of `Joystick/MT2D_Joystick.c` |

## Not covered

Code that needs a terminal, a window, a joystick or an OS API cannot run in a
headless test: `SDL/`, `_WINDOWS/`, `_LINUX/`, `_MSDOS/`, `_ANDROID/`,
`_EMNISCRIPT/`, `Audio` playback, `Cursor`, the `Joystick` backends, `InputBox`,
`MessageBox`, `Tools/UserInterface/MT2D_Menu.c` and
`ObjectCore/STDContainer.c` (needs SDL audio). That includes the settings of each backend
(`MT2D_SDL_Set_Settings`, `MT2D_Dos_Sound_Configure`, the VGA palette): their parameter checks are not tested.

## How it builds

* `tests/compat/MT2D/MT2D.h` replaces the real `MT2D.h` for the tests only. The
  real one includes platform headers with backslash paths that do not resolve
  on Linux.
* CMake creates a shadow include tree in the build folder for the includes that
  only work on case insensitive file systems (`MT2D/Objectcore/...`) or use
  backslashes. The engine sources are not modified.

## Known engine bugs

Tests declared with `TEST_XFAIL` describe the intended behaviour of something
that is broken today. They print `XFAIL` and do not fail the run; when the bug
is fixed they print `XPASS`, change them to `TEST`.

Found while writing the tests (no test, to avoid crashing the run):

* `MT2D_ContainerHash` has 15 buckets but names hash with `name[0] & 15`, so a
  file whose first character has a low nibble of `F` (`o`, `O`, `/`, `?`, `_`)
  writes out of bounds.
* `Cscript_AddVar_ToObject` indexes `Var[i]` while looping over the object's vars.

# web-gui

An image (PNG) MT2D game made for the browser, with **sound effects and music**: move the robot, catch the spinning
coins and dodge the ghosts. It is built **only** with Emscripten. The text mode version with PC speaker beeps is
[`web-tui`](../web-tui/README.md).

* Arrow keys or `A` / `D` move (held keys, smooth movement), `Enter` restarts. The page also has on-screen buttons for phones.
* Easy on purpose: 5 lives, only the ghosts cost a life, a missed coin does not.
* Sounds: coin (`coin.wav`), ghost hit (`explosion.ogg`), restart (`powerup.wav`) and a looping song (`ode.ogg`, Beethoven, public domain).
  Browsers keep the audio locked until the page gets a click or a key press, so the song starts at your first key.
* The best score is saved in the browser (`localStorage`) and the score is shown in the page title.

## What it shows

* **Images in a browser**: `Load_Sprite_Image` / `Sprite_Render_on_Window` work like on the desktop, the PNG files are
  preloaded into the browser's virtual file system by CMake (`PRELOAD` in [`CMakeLists.txt`](CMakeLists.txt)), so the
  code loads `"robot0.png"` as usual.
* **Audio**: `MT2D_Audio_Init`, `MT2D_Play_Audio` and `MT2D_Play_Music` (SDL2_mixer from the Emscripten ports).
* **Held keys**: `MT2D_Joystick_Update` / `MT2D_Joystick_Get_LeftAxisX`, the arrow keys act as the left stick.
* **The frame function** (`emscripten_set_main_loop`) and `EM_ASM` for `localStorage` and the page title, like `web-tui`.
* The images and sounds are not copied: they come from the `sprites`, `playsound` and `playmusic` examples.

## Building

Same as [`web-tui`](../web-tui/README.md#building):

```bash
emcmake cmake -S examples/web-gui -B examples/web-gui/build
cmake --build examples/web-gui/build
python3 -m http.server 8000 --directory examples/web-gui/bin
```

Known limit: on a phone held in portrait the engine switches to its rotated layout, lock the screen to landscape.

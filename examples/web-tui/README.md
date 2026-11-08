# web-tui

A text mode MT2D game made for the browser, with **PC speaker** sound effects: catch the falling `$` with the paddle and dodge the `X` bombs.
It is built **only** with Emscripten (the other examples are skipped there, and this one is skipped everywhere else). The image and
music version is [`web-gui`](../web-gui/README.md).

* Arrow keys or `A` / `D` move, `Enter` restarts. The page also has on-screen buttons for phones.
* Easy on purpose: 5 lives, only the bombs cost a life (a missed `$` does not), a wide paddle, and it speeds up slowly.
* PC speaker: `MT2D_System_Beep(frequency, ms)` plays a tone for catching, for a bomb, for the start and the game over.
  The browser stays silent until the page gets a click or a key press.
* The best score is saved in the browser (`localStorage`) and the score is shown in the page title.

## What it shows

* **The frame function.** A web page cannot run `for (;;)`, it would freeze the tab. The game is a function called
  once per frame with `emscripten_set_main_loop` (`src/main.c`).
* **The PC speaker in a browser.** On Emscripten `MT2D_System_Beep` does not wait for the tone to end (the audio runs
  when the page returns to the browser). The beeps go in a queue and play one after the other, so `sfx_game_over` is
  just four calls in a row.
* **Talking to the browser** with `EM_ASM` (`localStorage`, `document.title`).
* **A custom page** ([`../common/web_shell.html`](../common/web_shell.html)): canvas scaled with CSS, touch buttons that send keyboard events to SDL.
* **The CMake side**: `mt2d_add_example(... WEB)` in [`../cmake/MT2D.cmake`](../cmake/MT2D.cmake) enables the SDL2, SDL2_image and
  SDL2_mixer Emscripten ports, preloads the font as `assets/msdos_fnt.png` (the path the engine loads on Emscripten) and writes `bin/index.html`.

## Building

You need the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (`emcc` and `emcmake` in the `PATH`) and CMake 3.12+.

```bash
emcmake cmake -S examples/web-tui -B examples/web-tui/build
cmake --build examples/web-tui/build
```

The files (`index.html`, `index.js`, `index.wasm`, `index.data`) are written to `examples/web-tui/bin`. Browsers do not
load `.wasm` from `file://`, serve the folder:

```bash
python3 -m http.server 8000 --directory examples/web-tui/bin
# open http://localhost:8000
```

`emcmake cmake -S examples -B build/web` builds both web examples (all other examples are skipped).

Known limit: on a phone held in portrait the engine switches to its rotated layout, lock the screen to landscape.

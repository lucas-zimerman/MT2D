# Frame pacing

Header: `MT2D/MT2D_Frame.h` (header only, include it from the game loop).

```c
#include <MT2D/MT2D_Frame.h>

while (running) {
    update_and_draw();
    MT2D_Frame_Wait_Next();   /* sleeps what is left of the frame */
}
```

| Call | Meaning |
| --- | --- |
| `void MT2D_Frame_Wait_Next(void)` | sleeps until the end of the current frame, then starts the next one. The first call only starts the clock |
| `unsigned long MT2D_Frame_Core_Consumed(void)` | milliseconds the last frame spent working (game + render), without the wait. 0 before the second `MT2D_Frame_Wait_Next` |

`MT2D_Frame_Core_Consumed` is useful to see how much of the frame budget is left (at 30 fps a frame is about 33 ms).

## Platforms

| Build | Behavior |
| --- | --- |
| MS-DOS (DJGPP, `MT2D/_MSDOS/M2Frame.c`) | real implementation |
| SDL (`SDL_USE`, not Emscripten, `MT2D/SDL/MT2D_SDL_Frame.c`) | real implementation |
| anything else (terminals, Emscripten, ...) | no-ops: `MT2D_Frame_Wait_Next` returns at once, `MT2D_Frame_Core_Consumed` returns 0. A browser tab cannot be blocked, so Emscripten gets no-ops even with SDL |

`MT2D_FRAME_HAS_CLOCK` (1) is defined when the platform has the real implementation. A loop that needs another pacing
when it is not can test it:

```c
#ifdef MT2D_FRAME_HAS_CLOCK
    MT2D_Frame_Wait_Next();
#else
    MT2D_System_Delay(30);
#endif
```

## How the wait works

* The frame rate is `MT2D_FRAME_FPS` (default 30), define it before building the engine to change it.
* It sleeps (MS-DOS: `delay(1)`, the DOS idle call, a streamed song is refilled; SDL: `SDL_Delay(1)`) until
  `MT2D_FRAME_SPIN_US` before the end of the frame (3 ms on MS-DOS, 2 ms on SDL), then polls the clock so the frame ends on time.
* If one sleep takes longer than `MT2D_FRAME_SLEEP_MAX_US` (4 ms) the platform's wait is too coarse and sleeping stops for good
  (100% CPU, but the frame still ends on time).
* A frame that took longer than its time is not caught up, the next one gets a full period.
* SDL does not handle events while it waits: poll them once per frame as before.

## MS-DOS and music

While a song plays (`MT2D_Dos_Music_*`) the clock is the song timer interrupt (`MT2D_Dos_Music_Clock_Us`), because
`uclock()` reads the PIT the song moved and goes backwards. Its resolution is one song tick (1.4 ms at the default 700 Hz),
so frames end within about that of their time. When a song starts, stops or is replaced the clock changes: that frame is not
paced (it starts a new frame), and right after the song stops the first `uclock()` reading can be off by up to a BIOS tick (55 ms).

Example: [`examples/desert`](../../examples/desert/src/main.cpp). Benchmarks: [`bench/README.md`](../../bench/README.md#frame-pacing-ms-dos-mt2dmt2d_frameh).

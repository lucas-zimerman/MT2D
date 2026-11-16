/* Frame pacing for the Emscripten examples.

   emscripten_set_main_loop(frame, 0, 1) calls frame() once per screen refresh: 60 times a second on most screens,
   but 120 or 144 on a fast monitor, and the games count their animations in frames. Call web_step_due(60) at the
   top of frame() and return when it is 0: the program then runs at (at most) 60 steps a second whatever the screen does. */
#ifndef MT2D_WEB_TIMING_H
#define MT2D_WEB_TIMING_H

#include <emscripten.h>

static int web_step_due(int rate)
{
    static double last = 0.0, acc = 0.0;
    double now = emscripten_get_now(), step = 1000.0 / rate;

    if (last == 0.0) last = now;
    acc += now - last;
    last = now;
    if (acc < step - 1.0) return 0; /* a little tolerance: a 60 Hz screen delivers frames 16.6 ms apart */
    acc -= step;
    if (acc > step) acc = 0.0; /* the tab was in the background or the machine is slow, do not catch up */
    return 1;
}

#endif

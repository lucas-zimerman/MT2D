/*===================================================================
	MT2D_SDL_Frame.h, frame pacing for SDL. The API and its documentation are in MT2D/MT2D_Frame.h,
	this header is what MT2D_Frame.h maps to when SDL is used (not on Emscripten, see there).
===================================================================*/
#ifndef MT2D_SDL_FRAME_H
#define MT2D_SDL_FRAME_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

/* frames per second to hold, define it before building the engine to change it */
#ifndef MT2D_FRAME_FPS
#define MT2D_FRAME_FPS 30
#endif

/* SDL_Delay(1) sleeps about 1 to 2 ms, so the sleep stops this long before the end of the frame */
#ifndef MT2D_FRAME_SPIN_US
#define MT2D_FRAME_SPIN_US 2000UL
#endif

/* A sleep step that takes longer than this (a platform with a coarse timer, 15 ms on some Windows setups) turns the sleeping off,
   the clock is polled for the whole frame instead */
#ifndef MT2D_FRAME_SLEEP_MAX_US
#define MT2D_FRAME_SLEEP_MAX_US 4000UL
#endif

MT2D_C_BEGIN

void MT2D_SDL_Frame_Wait_Next(void);
unsigned long MT2D_SDL_Frame_Core_Consumed(void);

MT2D_C_END

#endif

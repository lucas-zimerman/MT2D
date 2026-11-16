/*===================================================================
	M2Frame.h, frame pacing for MS-DOS (DJGPP). The API and its documentation are in MT2D/MT2D_Frame.h,
	this header is what MT2D_Frame.h maps to on DOS.
===================================================================*/
#ifndef M2FRAME_H
#define M2FRAME_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

/* frames per second to hold, define it before building the engine to change it */
#ifndef MT2D_FRAME_FPS
#define MT2D_FRAME_FPS 30
#endif

/* delay(1) can take up to ~1.8 ms more than asked (measured in DOSBox and 86Box, see bench/bench_frame.c),
   so the sleep stops this long before the end of the frame */
#ifndef MT2D_FRAME_SPIN_US
#define MT2D_FRAME_SPIN_US 3000UL
#endif

/* A sleep step that takes longer than this (the BIOS wait of some machines is rounded to the 55 ms tick: delay(1) took
   54 ms on a simulated 386, see bench/bench_delay.c) turns the sleeping off, the clock is polled for the whole frame instead */
#ifndef MT2D_FRAME_SLEEP_MAX_US
#define MT2D_FRAME_SLEEP_MAX_US 4000UL
#endif

MT2D_C_BEGIN

void MT2D_Dos_Frame_Wait_Next(void);
unsigned long MT2D_Dos_Frame_Core_Consumed(void);

MT2D_C_END

#endif

/*===================================================================
	MT2D_Frame.h, frame pacing for a game loop.

	  while (running) {
	      update_and_draw();
	      MT2D_Frame_Wait_Next();      sleeps what is left of the frame
	  }

	Real implementations: MS-DOS built with DJGPP (MT2D/_MSDOS/M2Frame.c) and SDL (MT2D/SDL/MT2D_SDL_Frame.c).
	Every other platform gets no-ops here: MT2D_Frame_Wait_Next returns at once and MT2D_Frame_Core_Consumed returns 0, so a
	loop that is paced elsewhere (an Emscripten main loop, vsync) keeps working unchanged. Emscripten gets no-ops even with
	SDL: a browser tab cannot be blocked to wait.
	MT2D_FRAME_HAS_CLOCK is defined (1) when the platform has a real implementation, a loop that needs another pacing
	when it is not can test it:  #ifndef MT2D_FRAME_HAS_CLOCK  MT2D_System_Delay(30);  #endif

	The frame rate is MT2D_FRAME_FPS (default 30, see MT2D/_MSDOS/M2Frame.h), define it before building the engine to change it.
	The wait sleeps (MS-DOS: delay(1), the DOS idle call, a streamed song is refilled; SDL: SDL_Delay(1)) until
	MT2D_FRAME_SPIN_US before the end of the frame (3 ms on MS-DOS, 2 ms on SDL), then polls the clock so the frame ends on time.
	If a sleep takes longer than MT2D_FRAME_SLEEP_MAX_US (4 ms) the platform's wait is too coarse, sleeping stops for good
	(100% CPU, but the frame still ends on time).
	A frame that took longer than its time is not caught up, the next one gets a full period.

	SDL does not handle events while it waits: poll them once per frame as before.
	MS-DOS only: while a song plays (MT2D_Dos_Music_*) the clock is the song timer interrupt (MT2D_Dos_Music_Clock_Us), because
	uclock() reads the PIT the song moved and goes backwards. Its resolution is one song tick (1.4 ms at the default 700 Hz),
	so frames end within about that of their time. When a song starts, stops or is replaced the clock changes: that frame is not
	paced (it starts a new frame), and right after the song stops the first uclock() reading can be off by up to a BIOS tick (55 ms).
===================================================================*/
#ifndef MT2D_FRAME_H
#define MT2D_FRAME_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/MT2D_Terminal_Define.h> /* SDL_USE */

/* frames per second to hold, define it before building the engine to change it */
#ifndef MT2D_FRAME_FPS
#define MT2D_FRAME_FPS 30
#endif

/* MT2D_FRAME_EXTERNAL / MT2D_FRAME_SDL_EXTERNAL: the tests build the DOS / SDL implementation on the host with a simulated clock */
#if defined(__DJGPP__) || defined(MT2D_FRAME_EXTERNAL)
#include <MT2D/_MSDOS/M2Frame.h>
#define MT2D_FRAME_HAS_CLOCK 1

/* Sleeps until the end of the frame, then starts the next one. The first call only starts the clock. */
#define MT2D_Frame_Wait_Next MT2D_Dos_Frame_Wait_Next
/* Milliseconds the last frame spent working (game + render), without the wait. 0 before the second MT2D_Frame_Wait_Next. */
#define MT2D_Frame_Core_Consumed MT2D_Dos_Frame_Core_Consumed
#elif (defined(SDL_USE) && !defined(__EMSCRIPTEN__)) || defined(MT2D_FRAME_SDL_EXTERNAL)
#include <MT2D/SDL/MT2D_SDL_Frame.h>
#define MT2D_FRAME_HAS_CLOCK 1
#define MT2D_Frame_Wait_Next MT2D_SDL_Frame_Wait_Next
#define MT2D_Frame_Core_Consumed MT2D_SDL_Frame_Core_Consumed
#else
/* no frame clock on this platform: the loop is paced elsewhere */
MT2D_INLINE void MT2D_Frame_Wait_Next(void) {}
MT2D_INLINE unsigned long MT2D_Frame_Core_Consumed(void) { return 0; }
#endif

#endif

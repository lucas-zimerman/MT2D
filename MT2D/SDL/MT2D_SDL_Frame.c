/* Frame pacing for SDL, see MT2D/MT2D_Frame.h. ANSI C (C89) plus the SDL timer calls.
   The clock is the SDL performance counter, the sleep is SDL_Delay(1). MT2D_FRAME_SDL_EXTERNAL (the tests, on the host): same logic,
   the clock and the sleep come from the test. Not built for Emscripten (a browser tab cannot block), MT2D_Frame.h gives it no-ops. */
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/SDL/MT2D_SDL_Frame.h>

#if (defined(SDL_USE) && !defined(__EMSCRIPTEN__)) || defined(MT2D_FRAME_SDL_EXTERNAL)

#ifdef MT2D_FRAME_SDL_EXTERNAL
/* provided by the test */
MT2D_C_BEGIN
unsigned long MT2D_Sdl_Frame_Hook_Now_Us(void);
void MT2D_Sdl_Frame_Hook_Sleep(void);
MT2D_C_END
#define CLOCK_US() MT2D_Sdl_Frame_Hook_Now_Us()
#define SLEEP_STEP() MT2D_Sdl_Frame_Hook_Sleep()
#else
#include <SDL_timer.h>

static unsigned long clock_us(void) {
	static Uint64 freq = 0;
	Uint64 c, sec, rem;
	if (!freq) freq = SDL_GetPerformanceFrequency();
	c = SDL_GetPerformanceCounter();
	/* seconds and the rest apart, so the multiplication cannot overflow */
	sec = c / freq;
	rem = c % freq;
	return (unsigned long)((sec * (Uint64)1000000 + rem * (Uint64)1000000 / freq) & (Uint64)0xFFFFFFFFUL); /* only differences are used */
}
static void sleep_step(void) {
	SDL_Delay(1);
}
#define CLOCK_US() clock_us()
#define SLEEP_STEP() sleep_step()
#endif

#define PERIOD_US (1000000UL / (MT2D_FRAME_FPS))

static int g_started = 0;
static int g_sleep_ok = 1;             /* 0 once a sleep step took too long */
static unsigned long g_start = 0;     /* start of the frame in progress */
static unsigned long g_deadline = 0;  /* end of the frame in progress */
static unsigned long g_consumed = 0;  /* us */

/* a - b as a signed value, valid while the two are less than ~35 minutes apart (the clock is 32 bits) */
static long diff(unsigned long a, unsigned long b) {
	unsigned long d = (a - b) & 0xFFFFFFFFUL;
	return (d & 0x80000000UL) ? -(long)((~d & 0xFFFFFFFFUL) + 1UL) : (long)d;
}

void MT2D_SDL_Frame_Wait_Next(void) {
	unsigned long now = CLOCK_US();

	if (!g_started) {
		g_started = 1;
		g_start = now;
		g_deadline = (now + PERIOD_US) & 0xFFFFFFFFUL;
		return;
	}
	g_consumed = (now - g_start) & 0xFFFFFFFFUL;

	/* coarse part: sleep while the end of the frame is far away */
	while (g_sleep_ok && diff(g_deadline, now) > (long)MT2D_FRAME_SPIN_US) {
		unsigned long before = now;
		SLEEP_STEP();
		now = CLOCK_US();
		if (((now - before) & 0xFFFFFFFFUL) > MT2D_FRAME_SLEEP_MAX_US) g_sleep_ok = 0;
	}
	/* last part: poll the clock */
	while (diff(g_deadline, now) > 0) {
		now = CLOCK_US();
	}

	g_start = now;
	/* hold the phase when a little late, give a full period when a whole frame was lost */
	if (diff(now, g_deadline) > (long)PERIOD_US) {
		g_deadline = (now + PERIOD_US) & 0xFFFFFFFFUL;
	} else {
		g_deadline = (g_deadline + PERIOD_US) & 0xFFFFFFFFUL;
	}
}

unsigned long MT2D_SDL_Frame_Core_Consumed(void) {
	return g_consumed / 1000UL;
}

#endif

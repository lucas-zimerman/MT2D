/* Frame pacing for MS-DOS, see MT2D/MT2D_Frame.h. ANSI C (C89) plus the DJGPP calls.
   DJGPP: the PIT clock (uclock) or the song timer clock. MT2D_FRAME_EXTERNAL (the tests, on the host): same logic, the clock and
   the sleep come from the test. Other DOS compilers have no frame clock yet, MT2D_Frame.h gives them no-ops. */
#include <MT2D/_MSDOS/M2Frame.h>

#if defined(__DJGPP__) && !defined(MT2D_FRAME_EXTERNAL)
#include <time.h>
#include <dos.h>
#include <dpmi.h>
#include <MT2D/_MSDOS/M2Music.h>
#endif

#if defined(__DJGPP__) || defined(MT2D_FRAME_EXTERNAL)

#ifdef MT2D_FRAME_EXTERNAL
/* provided by the test */
MT2D_C_BEGIN
unsigned long MT2D_Dos_Frame_Hook_Now_Us(void);
void MT2D_Dos_Frame_Hook_Sleep(void);
int MT2D_Dos_Frame_Hook_Source(void);
MT2D_C_END
#define CLOCK_SOURCE() MT2D_Dos_Frame_Hook_Source()
#define CLOCK_US() MT2D_Dos_Frame_Hook_Now_Us()
#define SLEEP_STEP() MT2D_Dos_Frame_Hook_Sleep()
#else
/* Where the time comes from: 0 = uclock(), otherwise the id of the song timer (see MT2D_Dos_Music_Clock_Us). uclock() reads the PIT
   that a song moves, so while one plays the time comes from the song timer interrupt instead. */
static int clock_source(void) {
	unsigned long us;
	return MT2D_Dos_Music_Clock_Us(&us);
}
static unsigned long clock_us(void) {
	unsigned long us;
	uclock_t t;
	if (MT2D_Dos_Music_Clock_Us(&us)) return us;
	t = uclock();
	return (unsigned long)((t * 1000000UL) / UCLOCKS_PER_SEC); /* wraps after 71 minutes, only differences are used */
}
static void sleep_step(void) {
	__dpmi_regs r;
	MT2D_Dos_Music_Update(); /* refills a streamed song, as MT2D_System_Delay does */
	r.x.ax = 0x1680;         /* release the time slice (DPMI hosts and Windows DOS boxes), harmless if unsupported */
	__dpmi_int(0x2F, &r);
	delay(1);
}
#define CLOCK_SOURCE() clock_source()
#define CLOCK_US() clock_us()
#define SLEEP_STEP() sleep_step()
#endif

#define PERIOD_US (1000000UL / (MT2D_FRAME_FPS))

static int g_started = 0;
static int g_source = 0;               /* clock source of the frame in progress */
static int g_sleep_ok = 1;             /* 0 once a sleep step took too long */
static unsigned long g_start = 0;     /* start of the frame in progress */
static unsigned long g_deadline = 0;  /* end of the frame in progress */
static unsigned long g_consumed = 0;  /* us */

/* a - b as a signed value, valid while the two are less than ~35 minutes apart */
static long diff(unsigned long a, unsigned long b) {
	return (long)(a - b);
}

void MT2D_Dos_Frame_Wait_Next(void) {
	unsigned long now;
	int source = CLOCK_SOURCE();

	/* a song started, stopped or was replaced: the times of the two clocks cannot be mixed, begin a new frame */
	if (source != g_source) {
		g_source = source;
		g_started = 0;
	}
	now = CLOCK_US();
	if (!g_started) {
		g_started = 1;
		g_start = now;
		g_deadline = now + PERIOD_US;
		return;
	}
	g_consumed = now - g_start;

	/* coarse part: sleep while the end of the frame is far away */
	while (g_sleep_ok && diff(g_deadline, now) > (long)MT2D_FRAME_SPIN_US) {
		unsigned long before = now;
		SLEEP_STEP();
		now = CLOCK_US();
		if (now - before > MT2D_FRAME_SLEEP_MAX_US) g_sleep_ok = 0;
	}
	/* last part: poll the clock */
	while (diff(g_deadline, now) > 0) {
		now = CLOCK_US();
	}

	g_start = now;
	/* hold the phase when a little late, give a full period when a whole frame was lost */
	if (diff(now, g_deadline) > (long)PERIOD_US) {
		g_deadline = now + PERIOD_US;
	} else {
		g_deadline += PERIOD_US;
	}
}

unsigned long MT2D_Dos_Frame_Core_Consumed(void) {
	return g_consumed / 1000UL;
}

#endif

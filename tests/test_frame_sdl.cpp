// Tests for the frame pacing of SDL (MT2D/SDL/MT2D_SDL_Frame.c). The engine file is built with MT2D_FRAME_SDL_EXTERNAL: the
// clock and the sleep are simulated here, so the pacing math runs on the host without SDL (30 fps, the default).
// The real SDL clock (SDL_GetPerformanceCounter, SDL_Delay) is not covered here.
#define MT2D_FRAME_SDL_EXTERNAL // MT2D_Frame.h maps to the SDL implementation, built here with a simulated clock
#include <mt2d_test.h>
#include <MT2D/MT2D_Frame.h>

static unsigned long g_now = 4294000000UL; // close to the 32 bit wrap, the pacing must survive it
static unsigned long g_sleeps = 0;
static unsigned long g_sleep_cost = 1500;

extern "C" {
// every read of the clock costs 10 us, so a loop that polls it ends. The clock is 32 bits like the real one
unsigned long MT2D_Sdl_Frame_Hook_Now_Us(void) { g_now = (g_now + 10) & 0xFFFFFFFFUL; return g_now; }
// SDL_Delay(1) that takes 1.5 ms
void MT2D_Sdl_Frame_Hook_Sleep(void) { g_now = (g_now + g_sleep_cost) & 0xFFFFFFFFUL; g_sleeps++; }
}

static void work(unsigned long us) { g_now = (g_now + us) & 0xFFFFFFFFUL; }
static unsigned long since(unsigned long t) { return (g_now - t) & 0xFFFFFFFFUL; }

TEST(there_is_a_frame_clock) {
#ifdef MT2D_FRAME_HAS_CLOCK
    CHECK(1);
#else
    CHECK(0);
#endif
}

TEST(first_call_only_starts_the_clock) {
    unsigned long t = g_now;
    MT2D_Frame_Wait_Next();
    CHECK(since(t) < 100);
    CHECK_EQ((int)MT2D_Frame_Core_Consumed(), 0);
}

TEST(frame_takes_the_period_and_reports_the_work) {
    for (int i = 0; i < 5; i++) {
        work(4000);
        unsigned long t = g_now;
        MT2D_Frame_Wait_Next();
        CHECK_EQ((int)MT2D_Frame_Core_Consumed(), 4);
        // 33.3 ms per frame: what is left after 4 ms of work was waited
        CHECK(since(t) > 29000 && since(t) < 30000);
    }
}

TEST(rate_does_not_drift_across_the_clock_wrap) {
    g_now = 4294900000UL; // the 32 bit clock wraps (at 4294967296) during this test
    MT2D_Frame_Wait_Next();
    unsigned long t0 = g_now;
    for (int i = 0; i < 90; i++) { work(2500); MT2D_Frame_Wait_Next(); }
    unsigned long total = since(t0);
    // 90 frames of 33333 us = 3 s, within one frame
    CHECK(total > 3000000UL - 34000UL && total < 3000000UL + 34000UL);
}

TEST(a_late_frame_is_not_caught_up) {
    MT2D_Frame_Wait_Next();
    work(100000); // 3 periods of work
    MT2D_Frame_Wait_Next();
    CHECK_EQ((int)MT2D_Frame_Core_Consumed(), 100);
    // the next frame gets a whole period again
    work(1000);
    unsigned long t = g_now;
    MT2D_Frame_Wait_Next();
    CHECK_EQ((int)MT2D_Frame_Core_Consumed(), 1);
    CHECK(since(t) > 31000 && since(t) < 33500);
}

TEST(sleeps_only_in_the_coarse_part) {
    MT2D_Frame_Wait_Next();
    work(1000);
    g_sleeps = 0;
    unsigned long t = g_now;
    MT2D_Frame_Wait_Next();
    // 32 ms left, sleeps of 1.5 ms stop 2 ms before the end: about 20 of them, never past the deadline
    CHECK(g_sleeps >= 17 && g_sleeps <= 22);
    CHECK(since(t) <= 33000);
}

TEST(a_coarse_sleep_turns_sleeping_off) {
    // a platform whose wait is rounded to 15 ms: one step takes 15 ms, longer than MT2D_FRAME_SLEEP_MAX_US
    g_sleep_cost = 15000;
    g_sleeps = 0;
    work(1000);
    MT2D_Frame_Wait_Next();
    CHECK_EQ((int)g_sleeps, 1); // the first step overshoots, no more after it
    // from now on the frames are polled and end on time
    for (int i = 0; i < 3; i++) {
        work(1000);
        unsigned long t = g_now;
        g_sleeps = 0;
        MT2D_Frame_Wait_Next();
        CHECK_EQ((int)g_sleeps, 0);
        if (i > 0) CHECK(since(t) > 31000 && since(t) < 34000);
    }
}

MT2D_TEST_MAIN()

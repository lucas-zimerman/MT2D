// Tests for the frame pacing (MT2D/_MSDOS/M2Frame.c). The engine file is built with MT2D_FRAME_EXTERNAL: the
// clock and the sleep are simulated here, so the pacing math runs on the host (30 fps, the default).
// The DJGPP clock (uclock, delay) is measured by bench/bench_frame.c in DOSBox instead.
#define MT2D_FRAME_EXTERNAL // MT2D_Frame.h maps to the DOS implementation, built here with a simulated clock
#include <mt2d_test.h>
#include <MT2D/MT2D_Frame.h>

static unsigned long g_now = 1000000UL;
static unsigned long g_sleeps = 0;
static unsigned long g_sleep_cost = 1500;
static int g_source = 0;

extern "C" {
// every read of the clock costs 10 us, so a loop that polls it ends
unsigned long MT2D_Dos_Frame_Hook_Now_Us(void) { g_now += 10; return g_now; }
int MT2D_Dos_Frame_Hook_Source(void) { return g_source; }
// delay(1) that takes 1.5 ms
void MT2D_Dos_Frame_Hook_Sleep(void) { g_now += g_sleep_cost; g_sleeps++; }
}

static void work(unsigned long us) { g_now += us; }

TEST(first_call_only_starts_the_clock) {
    unsigned long t = g_now;
    MT2D_Frame_Wait_Next();
    CHECK(g_now - t < 100);
    CHECK_EQ((int)MT2D_Frame_Core_Consumed(), 0);
}

TEST(frame_takes_the_period_and_reports_the_work) {
    for (int i = 0; i < 5; i++) {
        work(4000);
        unsigned long t = g_now;
        MT2D_Frame_Wait_Next();
        CHECK_EQ((int)MT2D_Frame_Core_Consumed(), 4);
        // 33.3 ms per frame: what is left after 4 ms of work was waited
        CHECK(g_now - t > 29000 && g_now - t < 30000);
    }
}

TEST(rate_does_not_drift) {
    MT2D_Frame_Wait_Next();
    unsigned long t0 = g_now;
    for (int i = 0; i < 90; i++) { work(2500); MT2D_Frame_Wait_Next(); }
    unsigned long total = g_now - t0;
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
    CHECK(g_now - t > 31000 && g_now - t < 33500);
}

TEST(sleeps_only_in_the_coarse_part) {
    MT2D_Frame_Wait_Next();
    work(1000);
    g_sleeps = 0;
    unsigned long t = g_now;
    MT2D_Frame_Wait_Next();
    // 32 ms left, sleeps of 1.5 ms stop 3 ms before the end: about 19 of them, never past the deadline
    CHECK(g_sleeps >= 15 && g_sleeps <= 21);
    CHECK(g_now - t <= 33000);
}

TEST(a_coarse_sleep_turns_sleeping_off) {
    // a BIOS wait rounded to the 55 ms tick: one step takes 54 ms, longer than the frame
    g_sleep_cost = 54000;
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
        // the first frame after the late one is shorter (the phase is kept), then every frame is a full period
        if (i > 0) CHECK(g_now - t > 31000 && g_now - t < 34000);
    }
}

TEST(a_new_clock_source_starts_a_new_frame) {
    g_sleep_cost = 1500;
    MT2D_Frame_Wait_Next();
    work(1000);
    MT2D_Frame_Wait_Next();
    unsigned long consumed = MT2D_Frame_Core_Consumed();
    // a song starts: the times of the old and the new clock cannot be mixed, this call only starts the clock
    g_source = 1;
    work(50000);
    unsigned long t = g_now;
    MT2D_Frame_Wait_Next();
    CHECK(g_now - t < 100);
    CHECK_EQ((int)MT2D_Frame_Core_Consumed(), (int)consumed); // the work of that frame is not measured
    // and the frames after it are paced as usual, on the new source
    for (int i = 0; i < 3; i++) {
        work(2000);
        t = g_now;
        MT2D_Frame_Wait_Next();
        CHECK(g_now - t > 30000 && g_now - t < 34000);
    }
    // a different id (another song) restarts again
    g_source = 2;
    t = g_now;
    MT2D_Frame_Wait_Next();
    CHECK(g_now - t < 100);
    g_source = 0;
    MT2D_Frame_Wait_Next();
}

MT2D_TEST_MAIN()

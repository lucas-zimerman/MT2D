/* Runs the real MT2D_Frame_Wait_Next (build MT2D/_MSDOS/M2Frame.c with -DMT2D_FRAME_FPS=35) with 3 ms of work per frame and prints
   how the frames came out. Integer math only (runs on a 386 without an FPU). */
#include <stdio.h>
#include <time.h>
#include <MT2D/MT2D_Frame.h>

static unsigned long now_us(void) {
    return (unsigned long)(((uclock_t)uclock() * 1000000UL) / UCLOCKS_PER_SEC);
}

int main(void) {
    unsigned long t0, last, t, d, mn = 0xFFFFFFFFUL, mx = 0, work_end, csum = 0;
    int i, off = 0, n = 0;
    MT2D_Frame_Wait_Next();
    t0 = last = now_us();
    for (i = 0; i < 140; i++) {
        work_end = now_us() + 3000UL;
        while ((long)(work_end - now_us()) > 0) {}
        MT2D_Frame_Wait_Next();
        t = now_us();
        d = t - last;
        last = t;
        if (i > 0) {
            if (d < mn) mn = d;
            if (d > mx) mx = d;
            if (d > 28571UL + 1000UL || d + 1000UL < 28571UL) off++;
            csum += MT2D_Frame_Core_Consumed();
            n++;
        }
    }
    printf("RESULT fwait35 frames 140 total_us %lu (ideal %lu) min_us %lu max_us %lu off_by_over_1ms %d avg_consumed_ms %lu\n", last - t0, 140UL * 28571UL, mn, mx, off, csum / n);
    return 0;
}

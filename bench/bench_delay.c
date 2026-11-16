/* Distribution of delay(n) and of the real frame wait (MS-DOS, DJGPP). Integer math only, so it runs on a 386 without an FPU.
   delay_N: how long delay(N ms) takes, as min / median / 95th / 99th percentile / max (us) and the number of calls that took
            more than 4 ms longer than asked.
   frame_wait: the wait of MT2D_Frame_Wait_Next replayed at 35 fps (sleep with delay(1) until 3 ms before the end of the frame,
            then poll the clock), how late the frames end (us after the deadline) and how many ended over 1 ms late. */
#include <stdio.h>
#include <dos.h>
#include <time.h>

#define MAXRUNS 300

static unsigned long now_us(void) {
    return (unsigned long)(((uclock_t)uclock() * 1000000UL) / UCLOCKS_PER_SEC);
}

static unsigned long samples[MAXRUNS];

static void sort(unsigned long *a, int n) {
    int i, j;
    unsigned long v;
    for (i = 1; i < n; i++) {
        v = a[i];
        for (j = i - 1; j >= 0 && a[j] > v; j--) a[j + 1] = a[j];
        a[j + 1] = v;
    }
}

static void probe(unsigned int ms, int runs) {
    int i, over = 0;
    unsigned long t;
    for (i = 0; i < runs; i++) {
        t = now_us();
        delay(ms);
        samples[i] = now_us() - t;
        if (samples[i] > ms * 1000UL + 4000UL) over++;
    }
    sort(samples, runs);
    printf("RESULT delay_%u runs %d min %lu p50 %lu p95 %lu p99 %lu max %lu over_4ms_late %d\n", ms, runs, samples[0], samples[runs / 2],
           samples[runs * 95 / 100], samples[runs * 99 / 100], samples[runs - 1], over);
}

static void frame_wait(int frames) {
    unsigned long period = 1000000UL / 35UL, deadline, now, late_max = 0, late;
    int i, over = 0, sleeps = 0;
    now = now_us();
    deadline = now + period;
    for (i = 0; i < frames; i++) {
        /* 3 ms of work */
        { unsigned long e = now_us() + 3000UL; while ((long)(e - now_us()) > 0) {} }
        now = now_us();
        while ((long)(deadline - now) > 3000L) { delay(1); sleeps++; now = now_us(); }
        while ((long)(deadline - now) > 0) now = now_us();
        late = (long)(now - deadline) > 0 ? now - deadline : 0;
        if (late > late_max) late_max = late;
        if (late > 1000UL) over++;
        deadline += period;
        if ((long)(now - deadline) > (long)period) deadline = now + period;
    }
    printf("RESULT frame_wait frames %d delay1_calls %d late_max_us %lu over_1ms_late %d\n", frames, sleeps, late_max, over);
}

int main(void) {
    probe(1, MAXRUNS);
    probe(2, MAXRUNS);
    probe(5, 100);
    probe(10, 100);
    frame_wait(140);
    return 0;
}

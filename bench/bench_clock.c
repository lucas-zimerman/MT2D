/* Compares the clocks a frame loop could use next to the music timer (MS-DOS, DJGPP). Needs -std=gnu89 or later.
   The ISR below is a prototype of "one timer at a middle rate that counts ticks" (the music timer in M2Music.c is the same
   thing with a song callback). For every base rate R it prints:
     clock_*   what the tick clock reports for delay(500) (the BIOS wait runs off the RTC, so it is an independent reference)
     over_*    the time of a fixed busy loop with the ISR running, in % of the same loop with the DOS default timer
     uclock_*  what uclock() reports for delay(100) while the PIT runs at R (the problem of the frame clock during music)
   The engine itself is not touched. */
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <time.h>

#define PIT_CLOCK 1193182UL
#define LOOP_N    4000000UL

static volatile unsigned long g_ticks = 0;
static volatile unsigned long g_div = 0;
static volatile unsigned long g_acc = 0;
static struct { unsigned long offset; unsigned short selector; } g_old_far;
static _go32_dpmi_seginfo g_old_isr, g_new_isr;
static int g_on = 0;

static void tick_isr(void) {
    g_ticks++;
    g_acc += g_div;
    if (g_acc >= 65536UL) {
        g_acc -= 65536UL;
        __asm__ __volatile__("pushfl\n\tlcall *%0" : : "m"(g_old_far) : "memory", "cc");
    } else {
        outportb(0x20, 0x20);
    }
}
static void tick_isr_end(void) {}

static void pit_set(unsigned long divisor) {
    outportb(0x43, 0x36);
    outportb(0x40, (unsigned char)(divisor & 0xFF));
    outportb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

static void isr_on(unsigned long hz) {
    g_div = PIT_CLOCK / hz;
    g_acc = 0;
    g_ticks = 0;
    _go32_dpmi_lock_code((void *)tick_isr, (unsigned long)tick_isr_end - (unsigned long)tick_isr);
    _go32_dpmi_lock_data((void *)&g_ticks, sizeof(g_ticks));
    _go32_dpmi_lock_data((void *)&g_div, sizeof(g_div));
    _go32_dpmi_lock_data((void *)&g_acc, sizeof(g_acc));
    _go32_dpmi_lock_data(&g_old_far, sizeof(g_old_far));
    _go32_dpmi_get_protected_mode_interrupt_vector(8, &g_old_isr);
    g_old_far.offset = g_old_isr.pm_offset;
    g_old_far.selector = g_old_isr.pm_selector;
    g_new_isr.pm_selector = _go32_my_cs();
    g_new_isr.pm_offset = (unsigned long)tick_isr;
    _go32_dpmi_allocate_iret_wrapper(&g_new_isr);
    disable();
    _go32_dpmi_set_protected_mode_interrupt_vector(8, &g_new_isr);
    pit_set(g_div);
    enable();
    g_on = 1;
}

static void isr_off(void) {
    if (!g_on) return;
    disable();
    pit_set(0);
    _go32_dpmi_set_protected_mode_interrupt_vector(8, &g_old_isr);
    enable();
    _go32_dpmi_free_iret_wrapper(&g_new_isr);
    g_on = 0;
}

/* microseconds counted by the ISR (resolution: one tick) */
static unsigned long tick_us(void) {
    uclock_t t = (uclock_t)g_ticks * g_div;
    return (unsigned long)((t * 1000000UL) / PIT_CLOCK);
}

static volatile unsigned long sink;
static unsigned long uclock_us(void) {
    return (unsigned long)(((uclock_t)uclock() * 1000000UL) / UCLOCKS_PER_SEC);
}

/* Runs a busy loop for EDGES vertical retraces (70.085 Hz, independent of the PIT: 14268.4 us each) and counts the work done.
   tick_clock = 1: the elapsed time is also read from the ISR tick clock, 0: from uclock() (only valid with the default timer).
   iters is the work done in the time (overhead of the ISR = less work), clock_us what the clock says it took. */
#define EDGES 100
static void run_retraces(int tick_clock, unsigned long *iters, unsigned long *clock_us) {
    unsigned long n = 0, s = 0, t0;
    int edges = 0, prev;
    while (!(inportb(0x3DA) & 8)) {}      /* synchronize on the start of a retrace */
    while (inportb(0x3DA) & 8) {}
    prev = 0;
    t0 = tick_clock ? tick_us() : uclock_us();
    while (edges < EDGES) {
        int k;
        for (k = 0; k < 20; k++) { s += n ^ (s >> 3); n++; }
        k = (inportb(0x3DA) & 8) != 0;
        if (k && !prev) edges++;
        prev = k;
    }
    *clock_us = (tick_clock ? tick_us() : uclock_us()) - t0;
    *iters = n;
    sink = s;
}

/* how long delay(n) really takes, seen by uclock() with the default timer (validated by the retrace period) */
static void probe_delay(unsigned int ms) {
    unsigned long t = uclock_us();
    delay(ms);
    t = uclock_us() - t;
    printf("RESULT delay_%u_ms_takes_us %lu\n", ms, t);
}

/* uclock() while the PIT is moved but the BIOS tick is chained: how far off are short intervals, does it go backwards? */
static void uclock_quality(unsigned long hz) {
    unsigned long a, b, lo = 0xFFFFFFFFUL, hi = 0, back = 0, worst_back = 0, last, i;
    int k;
    if (hz) isr_on(hz); /* 0 = the DOS default timer, as a reference */
    delay(200);
    for (k = 0; k < 60; k++) {
        a = uclock_us();
        delay(10);
        b = uclock_us() - a;
        if (b < lo) lo = b;
        if (b > hi) hi = b;
    }
    last = uclock_us();
    for (i = 0; i < 30000UL; i++) {
        a = uclock_us();
        if ((long)(a - last) < 0) {
            back++;
            if (last - a > worst_back) worst_back = last - a;
        }
        last = a;
    }
    isr_off();
    printf("RESULT uclock_quality_%luHz delay_10ms_seen min %lu max %lu us, went_backwards %lu times of 30000 reads, worst_back_us %lu\n",
           hz, lo, hi, back, worst_back);
    delay(200);
}

int main(void) {
    static const unsigned long rates[] = { 140UL, 560UL, 700UL, 1000UL, 2800UL };
    unsigned long it0, c0, it, c;
    int i;

    probe_delay(1); probe_delay(10); probe_delay(50); probe_delay(65); probe_delay(66);
    probe_delay(67); probe_delay(100); probe_delay(200); probe_delay(500);

    run_retraces(0, &it0, &c0);
    printf("RESULT default_timer retraces %d (= %lu us) uclock_says_us %lu work_iters %lu\n", EDGES, (unsigned long)(EDGES * 14268.4), c0, it0);
    uclock_quality(0UL);
    uclock_quality(140UL);
    uclock_quality(700UL);
    for (i = 0; i < 5; i++) {
        unsigned long hz = rates[i];
        isr_on(hz);
        run_retraces(1, &it, &c);
        isr_off();
        printf("RESULT isr_%luHz retraces %d tick_clock_says_us %lu (error permille %ld) work_iters %lu (work lost permille %ld)\n", hz, EDGES, c,
               ((long)c - (long)(EDGES * 14268.4)) * 1000L / (long)(EDGES * 14268.4), it, ((long)it0 - (long)it) * 1000L / (long)it0);
        delay(200);
    }
    return 0;
}

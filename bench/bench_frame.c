/* Timing experiments for the frame pacing API (MS-DOS, DJGPP). ANSI C (C89).
   Each experiment prints RESULT lines, run it with bench/run_dosbox.sh (see bench/README.md).

   1. uclock():          cost of a call and the smallest step it can show
   2. delay(n):          how long delay(1), delay(5), delay(16) really take (min / avg / max)
   3. idle call:         cost of INT 2Fh AX=1680h (release the time slice)
   4. port 0x3DA:        cost of one read of the VGA status register
   5. retrace period:    time of 100 vertical retraces, in text mode and in mode 13h
*/
#include <stdio.h>
#include <dos.h>
#include <dpmi.h>
#include <pc.h>
#include <time.h>
#include <sys/time.h>

#define N_CLOCK 20000
#define N_PORT  20000
#define N_IDLE  2000

/* microseconds, uclock() is a 64 bit counter in UCLOCKS_PER_SEC (1193180) units */
static unsigned long now_us(void) {
    return (unsigned long)((double)uclock() * 1000000.0 / UCLOCKS_PER_SEC);
}

static void exp_clock(void) {
    unsigned long a, b, i, steps = 0, smallest = 0xFFFFFFFFUL, d, last;
    a = now_us();
    for (i = 0; i < N_CLOCK; i++) {
        (void)uclock();
    }
    b = now_us();
    printf("RESULT uclock_call_us %.3f\n", (double)(b - a) / N_CLOCK);
    last = now_us();
    for (i = 0; i < N_CLOCK; i++) {
        b = now_us();
        d = b - last;
        if (d) {
            steps++;
            if (d < smallest) smallest = d;
            last = b;
        }
    }
    printf("RESULT uclock_steps %lu smallest_step_us %lu\n", steps, smallest);
}

static void exp_delay(int ms, int runs) {
    unsigned long t, min = 0xFFFFFFFFUL, max = 0, sum = 0;
    int i;
    for (i = 0; i < runs; i++) {
        t = now_us();
        delay(ms);
        t = now_us() - t;
        if (t < min) min = t;
        if (t > max) max = t;
        sum += t;
    }
    printf("RESULT delay_%d_ms min_us %lu avg_us %lu max_us %lu\n", ms, min, sum / runs, max);
}

static void exp_idle(void) {
    __dpmi_regs r;
    unsigned long t;
    int i;
    t = now_us();
    for (i = 0; i < N_IDLE; i++) {
        r.x.ax = 0x1680;
        __dpmi_int(0x2F, &r);
    }
    t = now_us() - t;
    printf("RESULT idle_call_us %.3f (AL after call: 0x%02X, 0 = supported)\n", (double)t / N_IDLE, (unsigned)r.h.al);
}

static void exp_port(void) {
    unsigned long t;
    int i, v = 0;
    t = now_us();
    for (i = 0; i < N_PORT; i++) {
        v += inportb(0x3DA) & 8;
    }
    t = now_us() - t;
    printf("RESULT port_3DA_read_us %.3f (%d)\n", (double)t / N_PORT, v & 1);
}

/* waits one vertical retrace: leaves the current one, then waits for the next */
static int wait_retrace(unsigned long timeout_us) {
    unsigned long t = now_us();
    while (inportb(0x3DA) & 8) {
        if (now_us() - t > timeout_us) return 0;
    }
    while (!(inportb(0x3DA) & 8)) {
        if (now_us() - t > timeout_us) return 0;
    }
    return 1;
}

static void set_mode(int mode) {
    __dpmi_regs r;
    r.x.ax = (unsigned short)mode;
    __dpmi_int(0x10, &r);
}

static void exp_retrace(const char *name, int mode) {
    unsigned long t0, t1, mn = 0xFFFFFFFFUL, mx = 0, a, b;
    int i, ok = 1, missed = 0;
    set_mode(mode);
    delay(100);
    if (!wait_retrace(500000UL)) ok = 0;
    t0 = now_us();
    a = t0;
    for (i = 0; ok && i < 100; i++) {
        if (!wait_retrace(500000UL)) { ok = 0; break; }
        b = now_us();
        if (b - a < mn) mn = b - a;
        if (b - a > mx) mx = b - a;
        a = b;
    }
    t1 = now_us();
    set_mode(3);
    if (!ok) {
        printf("RESULT retrace_%s no retrace seen (timeout)\n", name);
        return;
    }
    printf("RESULT retrace_%s period_us %.1f hz %.3f min_us %lu max_us %lu\n", name,
           (double)(t1 - t0) / 100.0, 100.0 * 1000000.0 / (double)(t1 - t0), mn, mx);
    (void)missed;
}

/* how late a "sleep until 2 ms before the retrace, then spin" lands: the phase locked wait of the plan */
static void exp_phase_lock(void) {
    unsigned long period, t, target, miss = 0, worst = 0, e;
    int i;
    set_mode(0x13);
    delay(100);
    wait_retrace(500000UL);
    t = now_us();
    for (i = 0; i < 20; i++) wait_retrace(500000UL);
    period = (now_us() - t) / 20;
    wait_retrace(500000UL);
    target = now_us();
    for (i = 0; i < 100; i++) {
        target += period;
        /* coarse part: delay(1) while more than 3 ms remain */
        while (target > now_us() && target - now_us() > 3000UL) delay(1);
        /* spin part: look for the retrace until the predicted time + 2 ms */
        e = now_us();
        if (!wait_retrace(40000UL)) { miss++; continue; }
        e = now_us();
        {
            unsigned long dev = e > target ? e - target : target - e;
            if (dev > worst) worst = dev;
            if (dev > 1500UL) miss++;
        }
        target = e; /* re-lock on the real retrace */
    }
    set_mode(3);
    printf("RESULT phase_lock period_us %lu worst_deviation_us %lu off_by_over_1500us %lu of 100\n", period, worst, miss);
}

/* what uclock() reports for delay(100) (the BIOS wait runs off the RTC, not the PIT) before and after PIT channel 0
   is moved to a song rate, as MT2D_Dos_Music_* does (M2Music.c pit_set) */
static void pit_set(unsigned long divisor) {
    outportb(0x43, 0x36);
    outportb(0x40, (unsigned char)(divisor & 0xFF));
    outportb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

static void exp_pit_moved(void) {
    unsigned long t;
    int i;
    for (i = 0; i < 2; i++) {
        t = now_us();
        delay(100);
        printf("RESULT pit_default delay_100ms_seen_by_uclock_us %lu\n", now_us() - t);
    }
    pit_set(1193182UL / 140UL); /* 140 Hz (the engine default is 700 Hz, MT2D_MUSIC_DEFAULT_RATE) */
    for (i = 0; i < 2; i++) {
        t = now_us();
        delay(100);
        printf("RESULT pit_140hz   delay_100ms_seen_by_uclock_us %lu\n", now_us() - t);
    }
    pit_set(0); /* 65536, DOS default */
    t = now_us();
    delay(100);
    printf("RESULT pit_restored delay_100ms_seen_by_uclock_us %lu\n", now_us() - t);
}

int main(void) {
    exp_clock();
    exp_delay(1, 200);
    exp_delay(5, 100);
    exp_delay(16, 50);
    exp_idle();
    exp_port();
    exp_retrace("text80x25", 3);
    exp_retrace("mode13h", 0x13);
    exp_phase_lock();
    exp_pit_moved();
    return 0;
}

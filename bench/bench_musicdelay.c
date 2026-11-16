/* Does delay() make the song timer lose ticks? (MS-DOS, DJGPP, needs an OPL chip, integer math only)
   Plays a silent looping song, then for 10 seconds of the RTC (CMOS clock: hardware, not driven by the PIT or by interrupts that
   could be lost) it either spins, calls delay(30) in a loop, or calls delay(1) in a loop, and reports what the song timer's
   own clock (MT2D_Dos_Music_Clock_Us) counted in that time. 10000000 us = nothing was lost.
   The BIOS tick counter (0040:006C, advanced by the same interrupt) is reported too. */
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <pc.h>
#include <sys/farptr.h>
#include <go32.h>
#include <MT2D/_MSDOS/M2Music.h>

static unsigned char song[64 * 4];

static unsigned char rtc_seconds(void) {
    unsigned char s;
    outportb(0x70, 0);
    s = inportb(0x71);
    return (unsigned char)((s >> 4) * 10 + (s & 15));
}

static unsigned long bios_ticks(void) {
    return _farpeekl(_dos_ds, 0x46C);
}

static unsigned long music_us(void) {
    unsigned long us = 0;
    MT2D_Dos_Music_Clock_Us(&us);
    return us;
}

/* waits for the seconds register to change, then returns */
static void sync_rtc(void) {
    unsigned char s = rtc_seconds();
    while (rtc_seconds() == s) {}
}

static void phase(const char *name, int mode) {
    unsigned long m0, m1, b0, b1, calls = 0;
    unsigned char start, now;
    int secs = 0, last;
    sync_rtc();
    start = last = rtc_seconds();
    m0 = music_us();
    b0 = bios_ticks();
    while (secs < 10) {
        if (mode == 1) delay(30);
        else if (mode == 2) delay(1);
        calls++;
        now = rtc_seconds();
        if (now != last) { secs++; last = now; }
    }
    m1 = music_us();
    b1 = bios_ticks();
    (void)start;
    printf("RESULT %s rtc_seconds 10 song_clock_us %lu (ideal 10000000, error permille %ld) bios_ticks %lu (ideal ~182) loop_calls %lu\n", name, m1 - m0,
           ((long)(m1 - m0) - 10000000L) / 10000L, b1 - b0, calls);
}

int main(void) {
    int i;
    memset(song, 0, sizeof(song));
    for (i = 0; i < 64; i++) { song[i * 4] = (unsigned char)(i == 0 ? 0 : 0xB0); song[i * 4 + 2] = 100; }
    if (!MT2D_Dos_Music_Init()) { printf("RESULT no_opl_chip_found\n"); return 0; }
    if (MT2D_Dos_Music_PlayMemory(song, sizeof(song), 0, MT2D_MUSIC_LOOP) != MT2D_MUSIC_OK) { printf("RESULT play_error\n"); return 0; }
    phase("spin", 0);
    phase("delay30", 1);
    phase("delay1", 2);
    phase("spin_again", 0);
    MT2D_Dos_Music_Stop();
    return 0;
}

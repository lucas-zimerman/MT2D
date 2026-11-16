/* The real MT2D_Frame_Wait_Next while a song plays (MS-DOS, DJGPP; build MT2D/_MSDOS/M2Frame.c with -DMT2D_FRAME_FPS=35).
   Plays a silent looping song from memory (needs an OPL chip, the program says if there is none) and checks that frames still last
   1/35 s. uclock() cannot time that: the song moves the PIT. The reference is the BIOS tick counter (0040:006C, 18.2065 Hz), which the
   song timer chains at the right rate. Integer math only.
   A: 140 frames without music   B: 560 frames with music   C: 400 frames, the song starts, is replaced and stops. */
#include <stdio.h>
#include <string.h>
#include <sys/farptr.h>
#include <go32.h>
#include <MT2D/MT2D_Frame.h>
#include <MT2D/_MSDOS/M2Music.h>

#define FRAME_US 28571UL

static unsigned long bios_ticks(void) {
    return _farpeekl(_dos_ds, 0x46C);
}

/* a type 0 IMF: 64 events that write a silent value to an OPL register, 100 song ticks apart (9 s at 700 Hz) */
static unsigned char song[64 * 4];
static void make_song(void) {
    int i;
    memset(song, 0, sizeof(song));
    for (i = 0; i < 64; i++) {
        song[i * 4 + 0] = (unsigned char)(i == 0 ? 0x00 : 0xB0); /* the first word is 0: a type 0 file, only events */
        song[i * 4 + 1] = 0;
        song[i * 4 + 2] = 100;
        song[i * 4 + 3] = 0;
    }
}

static void play(void) {
    int err = MT2D_Dos_Music_PlayMemory(song, sizeof(song), 0, MT2D_MUSIC_LOOP);
    if (err != MT2D_MUSIC_OK) printf("RESULT play_error %d\n", err);
}

static void frames(const char *name, int n, int start_at, int replace_at, int stop_at) {
    unsigned long b0, b1, t3;
    int i;
    MT2D_Frame_Wait_Next();
    b0 = bios_ticks();
    for (i = 0; i < n; i++) {
        unsigned long k;
        for (k = 0; k < 6000UL; k++) { /* about 3 ms of work, counted in loops so it does not read a clock */
            t3 = k ^ (t3 >> 3);
        }
        if (i == start_at || i == replace_at) play();
        if (i == stop_at) MT2D_Dos_Music_Stop();
        MT2D_Frame_Wait_Next();
    }
    b1 = bios_ticks();
    /* ticks * 1000000 / 18.2065 = us, in integers: ticks * 54925 us */
    printf("RESULT %s frames %d bios_ms %lu ideal_ms %lu (error permille %ld)\n", name, n, (b1 - b0) * 54925UL / 1000UL, (unsigned long)n * FRAME_US / 1000UL,
           ((long)((b1 - b0) * 54925UL / 1000UL) - (long)((unsigned long)n * FRAME_US / 1000UL)) * 1000L / (long)((unsigned long)n * FRAME_US / 1000UL));
}

int main(void) {
    make_song();
    if (!MT2D_Dos_Music_Init()) {
        printf("RESULT no_opl_chip_found\n");
        return 0;
    }
    frames("A_no_music", 140, -1, -1, -1);
    play();
    frames("B_music", 560, -1, -1, -1);
    MT2D_Dos_Music_Stop();
    frames("C_start_replace_stop", 400, 100, 200, 300);
    return 0;
}

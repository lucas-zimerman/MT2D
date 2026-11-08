/*
   Play music: MT2D_Play_Music streams an .ogg file and loops it. Playing another file replaces the
   current one. Press a key while the visualizer runs to go back to the menu (the music keeps playing).

   Needs SDL2_mixer with OGG support. Without it the engine ignores the audio calls and this example
   only says so. The songs (Beethoven, public domain) are rendered by tools/make_assets.py.

   MS-DOS (DJGPP): the songs are IMF files of the "d" folder (ode.ogg -> d/ode.imf), played on the OPL2 chip
   (Adlib / Sound Blaster, DOSBox emulates one) by the same MT2D_Play_Music call. tools/make_imf.py renders them.
   The MS-DOS menu has a third song, "Bad Apple!!" (d/badapple.imf, made by tools/midi2imf.py). It is 130 KB, bigger
   than the 32 KB song buffer, so the engine streams it from the disk. It is an unofficial fan work: its credits are
   shown on screen and in NOTICE.md.
   MS-DOS also takes a song on the command line: playmus.exe d/mysong.imf plays any IMF file (type 0 or 1) right away.
*/
#include <stdio.h>
#include <math.h>
#include "example.h"
#include <MT2D/Audio/MT2D_Audio_core.h>

#define BARS 26
#ifdef __DJGPP__
#define MUSIC_TITLE "MT2D play music (imf)"
#define EXIT_OPTION 4 /* the MS-DOS menu has a third song */
/* Credits of d/badapple.imf, shown while it plays (the details are in NOTICE.md). */
static const char *const CREDITS[3] = {
    "\"Bad Apple!!\" by ZUN (Team Shanghai Alice), Touhou Project (1998)",
    "Arrangement: Masayoshi Minoshima / Alstroemeria Records (2007)",
    "MIDI: PGFLIMSXD / nlexsctex / handhule90. Unofficial non-commercial fan work."
};
#else
#define EXIT_OPTION 3
#define MUSIC_TITLE "MT2D play music (ogg)"
#endif

#ifdef MT2D_EXAMPLE_AUDIO_FILES
/* A fake spectrum analyzer, just something to look at while the music plays. */
static void visualizer(const char *title, double t, int credits)
{
    char line[80];
    int bar, row, height;

    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)MUSIC_TITLE, 1, 29, DISPLAY_WINDOW1);
    sprintf(line, "Now playing: %s", title);
    insert_string_on_display(line, 3, 4, DISPLAY_WINDOW1);
#ifdef __DJGPP__
    insert_string_on_display(MT2D_Dos_Music_IsStreaming() ? (char *)"(streamed from disk)" : (char *)"(loaded in memory)", 5, 4, DISPLAY_WINDOW1);
#endif
    insert_string_on_display((char *)"press any key to go back to the menu", 4, 4, DISPLAY_WINDOW1);
#ifdef __DJGPP__
    if (credits) {
        for (row = 0; row < 3; row++) insert_string_on_display((char *)CREDITS[row], 22 + row, 1, DISPLAY_WINDOW1);
    }
#else
    (void)credits;
#endif
    for (bar = 0; bar < BARS; bar++) {
        height = 8 + (int)(7.0 * sin(t * 2.3 + bar * 0.45) * sin(t * 0.9 + bar * 0.15));
        if (height < 1) height = 1;
        for (row = 0; row < height; row++) {
            unsigned char c = row > 11 ? 177 : 219;
            WINDOW1[21 - row][6 + bar * 2 + bar / 2] = c;
            WINDOW1[21 - row][7 + bar * 2 + bar / 2] = c;
        }
    }
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

#ifdef __DJGPP__
static const char *g_cmdline_song = 0;
#endif

static int run(void)
{
    int option = 0;
    int credits = 0;
    double t = 0.0;
    const char *title = "";

#ifdef __DJGPP__
    if (!MT2D_Audio_Init()) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"No OPL2 (Adlib / Sound Blaster) found: you will not hear the music.", 12, 5, DISPLAY_WINDOW1);
        insert_string_on_display((char *)"Press any key to continue.", 14, 5, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        MT2D_Keyboard_keytouched();
    }
    if (g_cmdline_song) {
        MT2D_Play_Music((char *)g_cmdline_song);
        while (!MT2D_Keyboard_touched()) {
            visualizer(g_cmdline_song, t, 0);
            t += 0.04;
            MT2D_System_Delay(40);
        }
        MT2D_Keyboard_keytouched();
        MT2D_Audio_Close();
        return 0;
    }
#else
    MT2D_Audio_Init();
#endif
    while (option != EXIT_OPTION) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)MUSIC_TITLE, 1, 29, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
#ifdef __DJGPP__
        option = MT2D_Menu((char *)"SONGS", 20, 12, 5, 0, true, ' ', '>',
                           (char *)"Ode to Joy - Beethoven", (char *)"Fur Elise - Beethoven",
                           (char *)"Bad Apple!! (Touhou fan arrangement)", (char *)"Exit",
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EXIT_OPTION);
#else
        option = MT2D_Short_Menu((char *)"SONGS", 20, 12, 5, true, ' ', '>',
                                 (char *)"Ode to Joy - Beethoven", (char *)"Fur Elise - Beethoven", (char *)"Exit", 0);
#endif
        credits = 0;
        if (option == 1) {
            MT2D_Play_Music((char *)"ode.ogg");
            title = "Ode to Joy (ode.ogg)";
        } else if (option == 2) {
            MT2D_Play_Music((char *)"elise.ogg");
            title = "Fur Elise (elise.ogg)";
#ifdef __DJGPP__
        } else if (option == 3) {
            MT2D_Play_Music((char *)"d/badapple.imf");
            title = "Bad Apple!! (badapple.imf)";
            credits = 1;
#endif
        }
        if (option > 0 && option < EXIT_OPTION) {
            while (!MT2D_Keyboard_touched()) {
                visualizer(title, t, credits);
                t += 0.04;
                MT2D_System_Delay(40);
            }
            MT2D_Keyboard_keytouched();
        }
    }
    MT2D_Audio_Close();
    return 0;
}
#else
static int run(void)
{
    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"This build has no audio: install SDL2_mixer and build the example again.", 12, 3, DISPLAY_WINDOW1);
    MT2D_Draw_Window(DISPLAY_WINDOW1);
    MT2D_Keyboard_keytouched();
    return 0;
}
#endif

int main(int argc, char **argv)
{
#ifdef __DJGPP__
    if (argc > 1) g_cmdline_song = argv[1];
#else
    (void)argc;
    (void)argv;
#endif
    MT2D_Init();
    return run();
}

/*
   Play music: MT2D_Play_Music streams an .ogg file and loops it. Playing another file replaces the
   current one. Press a key while the visualizer runs to go back to the menu (the music keeps playing).

   Needs SDL2_mixer with OGG support. Without it the engine ignores the audio calls and this example
   only says so. The songs (Beethoven, public domain) are rendered by tools/make_assets.py.
*/
#include <stdio.h>
#include <math.h>
#include "example.h"
#include <MT2D/Audio/MT2D_Audio_core.h>

#define BARS 26

#ifdef MT2D_EXAMPLE_AUDIO_FILES
/* A fake spectrum analyzer, just something to look at while the music plays. */
static void visualizer(const char *title, double t)
{
    char line[80];
    int bar, row, height;

    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"MT2D play music (ogg)", 1, 29, DISPLAY_WINDOW1);
    sprintf(line, "Now playing: %s", title);
    insert_string_on_display(line, 3, 4, DISPLAY_WINDOW1);
    insert_string_on_display((char *)"press any key to go back to the menu", 4, 4, DISPLAY_WINDOW1);
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

static int run(void)
{
    int option = 0;
    double t = 0.0;
    const char *title = "";

    MT2D_Audio_Init();
    while (option != 3) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"MT2D play music (ogg)", 1, 29, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        option = MT2D_Short_Menu((char *)"SONGS", 20, 12, 5, true, ' ', '>',
                                 (char *)"Ode to Joy - Beethoven", (char *)"Fur Elise - Beethoven", (char *)"Exit", 0);
        if (option == 1) {
            MT2D_Play_Music((char *)"ode.ogg");
            title = "Ode to Joy (ode.ogg)";
        } else if (option == 2) {
            MT2D_Play_Music((char *)"elise.ogg");
            title = "Fur Elise (elise.ogg)";
        }
        if (option == 1 || option == 2) {
            while (!MT2D_Keyboard_touched()) {
                visualizer(title, t);
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
    (void)argc;
    (void)argv;
    MT2D_Init();
    return run();
}

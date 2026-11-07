/*
   Play sound: MT2D_Play_Audio plays a .wav or .ogg file once. The call returns right away, so you
   can start a new sound while the other ones are still playing (up to 8 at the same time).

   Needs SDL2_mixer. Without it the engine ignores the audio calls and this example only says so.
   The sounds are made by tools/make_assets.py.
*/
#include <stdio.h>
#include "example.h"
#include <MT2D/Audio/MT2D_Audio_core.h>

static void screen(const char *status)
{
    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"MT2D play sound (wav / ogg)", 1, 26, DISPLAY_WINDOW1);
    insert_string_on_display((char *)status, 22, 3, DISPLAY_WINDOW1);
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

#ifdef MT2D_EXAMPLE_AUDIO_FILES
static int run(void)
{
    static const char *files[6] = { "coin.wav", "laser.wav", "powerup.wav", "jump.ogg", "explosion.ogg", "blip.wav" };
    int option = 0;
    char status[80];

    MT2D_Audio_Init();
    screen("Pick a sound, you can play many at the same time.");
    while (option != 7) {
        option = MT2D_Menu((char *)"PICK A SOUND", 18, 10, 3, 0, true, ' ', '>',
                           (char *)"Coin (wav)", (char *)"Laser (wav)", (char *)"Power up (wav)",
                           (char *)"Jump (ogg)", (char *)"Explosion (ogg)", (char *)"Blip (wav)", (char *)"Exit",
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7);
        if (option >= 1 && option <= 6) {
            MT2D_Play_Audio((char *)files[option - 1]);
            sprintf(status, "Last played: %s", files[option - 1]);
            screen(status);
        }
    }
    MT2D_Audio_Close();
    return 0;
}
#else
static int run(void)
{
    screen("This build has no audio: install SDL2_mixer and build the example again.");
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

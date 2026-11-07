/*
   PC speaker: sound effects and songs made of beeps. Every effect is a list of beeps (frequency in Hz, time in ms), so it works
   everywhere the engine can beep: the PC speaker on MS-DOS, the Beep API on Windows and a
   generated tone played by afplay/paplay/aplay on macOS and Linux. No audio files needed.
*/
#include <stdio.h>
#include "example.h"
#include "songs.h"

static void box(int row, int width, char c)
{
    int i;
    for (i = 0; i < width; i++)
        WINDOW1[row][20 + i] = c;
}

/* A little animation that goes with the sound so the screen is not boring. */
static void splash(const char *name, int frame)
{
    char line[80];
    int i;
    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"MT2D PC speaker", 1, 30, DISPLAY_WINDOW1);
    sprintf(line, "Playing: %s", name);
    insert_string_on_display(line, 4, 30, DISPLAY_WINDOW1);
    for (i = 0; i < 5; i++)
        box(8 + i, (frame * 7 + i * 13) % 40 + 1, 219 - (i % 3));
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

static void coin(void)
{
    splash("coin", 1);
    MT2D_System_Beep(988, 80);
    splash("coin", 4);
    MT2D_System_Beep(1319, 350);
}

static void laser(void)
{
    int f, n = 0;
    for (f = 2400; f > 300; f -= 150) {
        splash("laser", n++);
        MT2D_System_Beep(f, 18);
    }
}

static void powerup(void)
{
    int f, n = 0;
    for (f = 300; f < 1800; f += 90) {
        splash("power up", n++);
        MT2D_System_Beep(f, 20);
    }
}

static void alarm_siren(void)
{
    int i;
    for (i = 0; i < 6; i++) {
        splash("siren", i);
        MT2D_System_Beep(i % 2 ? 660 : 880, 140);
    }
}

static void robot(void)
{
    int i;
    for (i = 0; i < 12; i++) {
        splash("robot", i);
        MT2D_System_Beep(120 + (i * 97) % 400, 45);
    }
}

static void bomb(void)
{
    int f, n = 0;
    for (f = 220; f > 40; f -= 12) {
        splash("bomb", n++);
        MT2D_System_Beep(f > 37 ? f : 37, 28);
    }
}

int main(int argc, char **argv)
{
    int option = 0;

    (void)argc;
    (void)argv;
    MT2D_Init();

    while (option != 9) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"MT2D PC speaker", 1, 30, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        option = MT2D_Menu((char *)"PICK A SOUND", 18, 10, 3, 0, true, ' ', '>',
                           (char *)"Coin", (char *)"Laser", (char *)"Power up", (char *)"Siren",
                           (char *)"Robot", (char *)"Bomb", (char *)"Song: Ode to Joy",
                           (char *)"Song: Fur Elise", (char *)"Exit",
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 9);
        switch (option) {
        case 1: coin(); break;
        case 2: laser(); break;
        case 3: powerup(); break;
        case 4: alarm_siren(); break;
        case 5: robot(); break;
        case 6: bomb(); break;
        case 7: Song_Play_Ode_To_Joy(); break;
        case 8: Song_Play_Fur_Elise(); break;
        default: break;
        }
    }
    return 0;
}

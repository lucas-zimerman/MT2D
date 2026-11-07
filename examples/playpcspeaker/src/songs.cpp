/*
   Songs played with MT2D_System_Beep (one note at a time), with a little equalizer on the screen.
   Both tunes are public domain compositions by Beethoven. Press any key to stop a song.
*/
#include <stdio.h>
#include <math.h>
#include "example.h"
#include "songs.h"

struct Note {
    int midi;  /* 0 = rest, 60 = middle C */
    int beats; /* length in 1/4 of a beat */
};

/* Beethoven, Ode to Joy (1824) */
static const Note ode[] = {
    {64,4},{64,4},{65,4},{67,4},{67,4},{65,4},{64,4},{62,4},{60,4},{60,4},{62,4},{64,4},{64,6},{62,2},{62,8},
    {64,4},{64,4},{65,4},{67,4},{67,4},{65,4},{64,4},{62,4},{60,4},{60,4},{62,4},{64,4},{62,6},{60,2},{60,8},
    {62,4},{62,4},{64,4},{60,4},{62,4},{64,2},{65,2},{64,4},{60,4},{62,4},{64,2},{65,2},{64,4},{62,4},{60,4},{62,4},{55,8},
    {64,4},{64,4},{65,4},{67,4},{67,4},{65,4},{64,4},{62,4},{60,4},{60,4},{62,4},{64,4},{62,6},{60,2},{60,12}
};

/* Beethoven, Fur Elise (1810) */
static const Note elise[] = {
    {76,2},{75,2},{76,2},{75,2},{76,2},{71,2},{74,2},{72,2},{69,6},{0,2},
    {60,2},{64,2},{69,2},{71,6},{0,2},{64,2},{68,2},{71,2},{72,6},{0,2},
    {64,2},{76,2},{75,2},{76,2},{75,2},{76,2},{71,2},{74,2},{72,2},{69,6},{0,2},
    {60,2},{64,2},{69,2},{71,6},{0,2},{64,2},{72,2},{71,2},{69,8}
};

static int frequency(int midi)
{
    return (int)(440.0 * pow(2.0, (midi - 69) / 12.0));
}

static void draw(const char *title, int midi, int index, int total)
{
    char line[80];
    int bar, height, row;

    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"MT2D music player", 1, 31, DISPLAY_WINDOW1);
    sprintf(line, "Now playing: %s", title);
    insert_string_on_display(line, 3, 4, DISPLAY_WINDOW1);
    sprintf(line, "Note %d / %d   (press any key to stop)", index + 1, total);
    insert_string_on_display(line, 4, 4, DISPLAY_WINDOW1);

    /* equalizer: the higher the note the more the bars jump */
    height = midi > 0 ? (midi - 50) / 2 : 0;
    for (bar = 0; bar < 20; bar++) {
        int h = height - (bar % 5) * 2 + (index + bar) % 3;
        if (h < 1) h = 1;
        if (h > 14) h = 14;
        for (row = 0; row < h; row++) {
            WINDOW1[21 - row][10 + bar * 3] = 219;
            WINDOW1[21 - row][11 + bar * 3] = 219;
        }
    }
    /* progress bar */
    for (bar = 0; bar < 70 * (index + 1) / total; bar++)
        WINDOW1[22][5 + bar] = 178;
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

static void play(const char *title, const Note *notes, int total, int tempo_ms)
{
    int i;
    for (i = 0; i < total; i++) {
        draw(title, notes[i].midi, i, total);
        if (MT2D_Keyboard_touched()) {
            MT2D_Keyboard_keytouched();
            return;
        }
        if (notes[i].midi == 0)
            MT2D_System_Delay(notes[i].beats * tempo_ms);
        else
            MT2D_System_Beep(frequency(notes[i].midi), notes[i].beats * tempo_ms);
        MT2D_System_Delay(8); /* tiny gap between two notes */
    }
}

void Song_Play_Ode_To_Joy(void)
{
    play("Ode to Joy - Beethoven", ode, (int)(sizeof(ode) / sizeof(ode[0])), 55);
}

void Song_Play_Fur_Elise(void)
{
    play("Fur Elise - Beethoven", elise, (int)(sizeof(elise) / sizeof(elise[0])), 70);
}

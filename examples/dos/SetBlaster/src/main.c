/*
   SetBlaster: the smallest example of how to set up the sound card on MS-DOS.

   The engine finds the Sound Blaster with the BLASTER environment variable (SET BLASTER=A220 I5 D1 T3).
   When it is not set, or is wrong, MT2D_DOS_SET_AUDIO gives the engine the setup:

       int cfg[] = { MT2D_DOS_SET_AUDIO, port, irq, dma, rate };
       MT2D_Set_Settings_Argv(5, cfg);

   A value that is left out or is -1 is not overridden (BLASTER, or the default rate, is used for it).
   Call it before MT2D_Audio_Init(); if the card is already running it is restarted with the new setup.

   Usage: setblast [port irq dma rate]       (the port is hexadecimal)
   Example: setblast 220 5 1 22050
   Plays d/coin.wav when the card answers.
*/
#include <stdio.h>
#include <stdlib.h>
#include "example.h"
#include <MT2D/MT2D_Settings.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <MT2D/_MSDOS/M2Sound.h>

static const char *result_text(MT2D_Result r)
{
    switch (r) {
    case MT2D_OK:                return "OK";
    case MT2D_INVALID:           return "invalid (the card did not answer)";
    case MT2D_BEFORE_INIT_ONLY:  return "only before init";
    case MT2D_BAD_PARAMETER:     return "bad parameter (port 210-280, irq 3-15, dma 0/1/3, rate 4000-44100)";
    case MT2D_UNSUPPORTED:       return "unsupported";
    }
    return "?";
}

int main(int argc, char **argv)
{
    int cfg[5];
    MT2D_Result result;
    int waited;

    cfg[0] = MT2D_DOS_SET_AUDIO;
    cfg[1] = argc > 1 ? (int)strtol(argv[1], NULL, 16) : 0x220;
    cfg[2] = argc > 2 ? atoi(argv[2]) : 5;
    cfg[3] = argc > 3 ? atoi(argv[3]) : 1;
    cfg[4] = argc > 4 ? atoi(argv[4]) : 22050;

    printf("Sound card setup: port %X, irq %d, dma %d, rate %d Hz\n", cfg[1], cfg[2], cfg[3], cfg[4]);
    result = MT2D_Set_Settings_Argv(5, cfg);
    printf("MT2D_Set_Settings_Argv: %s\n", result_text(result));
    if (result != MT2D_OK) {
        return 1;
    }

    if (!MT2D_Audio_Init()) {
        printf("No Sound Blaster or OPL chip found with that setup.\n");
        return 1;
    }
    printf("Sound card started, playing coin...\n");
    MT2D_Play_Audio((char *)"coin.wav");
    for (waited = 0; MT2D_Dos_Sound_Active() > 0 && waited < 3000; waited += 20) {
        MT2D_System_Delay(20);
    }
    MT2D_Audio_Close();
    return 0;
}

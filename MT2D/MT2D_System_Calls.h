/*===================================================================
system_calls.h/ system_calls.cpp created By Lucas Zimerman Fraulob

This file will contain some simple hardware calls

        List of functions and what they do and their returns:

        -Delay()
                Description: freeze the code by x miliseconds,
                NOTE: this will not freeze if the software make use of threads, only the thread with the freeze code
                will be delayed.
                Return: none
        -pc_speaker_beep()
                Description: Make a beep with a given frequency on pc speaker, (MS-DOS and WINDOWS ONLY), or into the sound card (Using SDL [works under windows, linux])
                        NOTE: since windows vista, the pc speaker code is emulated, so it'll not sound as good as XP or MS-DOS (So better use the SDL version)
                        for fast beeps.
                Return: none

===================================================================*/
#ifndef MT2D_SYSC_H
#define MT2D_SYSC_H

#include <MT2D/MT2D_Terminal_Define.h>

#if defined(SDL_USE)
#include "SDL/MT2D_SDL_System_Calls.h"
//#include <MT2D/SDL/MT2D_SDL_main.h>
#define MT2D_System_Delay MT2D_SDL_System_Delay
#define MT2D_System_Beep MT2D_SDL_System_Beep
#elif defined(_WIN32)
#include "_WINDOWS\MT2D_Win_System_Calls.h"
#define MT2D_System_Delay MT2D_Win_System_Delay
#define MT2D_System_Beep MT2D_Win_System_Beep
#elif defined(__MSDOS__) || defined(__DOS__)
#include <MT2D\_MSDOS\MT2D_Dos_System_Calls.h>
#define MT2D_System_Delay MT2D_Dos_System_Delay
#define MT2D_System_Beep MT2D_Dos_System_Beep
#elif defined(linux)
#include "_LINUX/MT2D_Linux_System_Calls.h"
#define MT2D_System_Delay MT2D_Linux_System_Delay
#define MT2D_System_Beep MT2D_Linux_System_Beep
#else
void MT2D_System_Delay(int time_milisecond);
void MT2D_System_Beep(int frequency, int time_milisecond);
#endif

#endif

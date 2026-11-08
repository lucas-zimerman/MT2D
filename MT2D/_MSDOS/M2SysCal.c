#if defined(__MSDOS__) || defined(__DOS__)
#include <MT2D/MT2D_Terminal_Define.h>
#include <dos.h>
#ifdef __DJGPP__
#include <MT2D/_MSDOS/M2Music.h>
#endif

void MT2D_Dos_System_Delay(int time_millisecond) {
#ifdef __DJGPP__
        MT2D_Dos_Music_Update(); /* refills a streamed song, does nothing otherwise */
#endif
        delay(time_millisecond);
}

void MT2D_Dos_System_Beep(int frequency, int time_milisecond) {
        sound(frequency);
        delay(time_milisecond);
        nosound();
}
#endif

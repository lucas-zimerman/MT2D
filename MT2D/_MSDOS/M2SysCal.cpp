#if defined(__MSDOS__) || defined(__DOS__)
#include <MT2D/MT2D_Terminal_Define.h>
#include <dos.h>

void MT2D_Dos_System_Delay(int time_millisecond) {
        delay(time_millisecond);
}

void MT2D_Dos_System_Beep(int frequency, int time_milisecond) {
        sound(frequency);
        delay(time_milisecond);
        nosound();
}
#endif

#ifndef __MSDOS__
#include "MT2D/MT2D_Terminal_Define.h"
#endif

#ifdef SDL_USE
    #include "SDL_MT2D/SDL_MT2Dmain.h"
    #include "SDL_MT2D/SDL_MT2DBeep.h"
#elif defined _WIN32
    #include <windows.h>
#elif defined __MSDOS__
    #include <dos.h>
#elif defined linux
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <linux/kd.h>
#endif

void MT2D_System_Delay(int time_millisecond){
#ifdef SDL_USE
    SDL_MT2DDelay(time_millisecond);
#elif defined _WIN32
	Sleep(time_millisecond);
#elif defined __MSDOS__
	delay(time_millisecond);//ms-dos
#elif defined linux
    usleep(time_millisecond);
#endif
}


void MT2D_System_Beep(int frequency, int time_milisecond){
#ifdef SDL_USE
    SDL_Beep(frequency,time_milisecond);
    //not implemented
#elif defined _WIN32
	Beep(frequency,time_milisecond);
#elif defined __MSDOS__
	sound(frequency);
	delay(time_milisecond);
	nosound();
#elif defined linux
    /* start beep */
    ioctl(2, KIOCSOUND, 1193180/frequency);//http://www.hardware.com.br/comunidade/beep-linux/730836/
    /* sleep for the duration time  */
    usleep(time_milisecond * 1000);
    /* stop beep */
    ioctl(2, KIOCSOUND, 0);
#endif

}

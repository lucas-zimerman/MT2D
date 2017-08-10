#ifndef __MSDOS__
#include <MT2D/MT2D_Terminal_Define.h>
#endif

#ifdef SDL_USE_AUDIO
#include <MT2D/SDL/Audio/MT2D_SDL_Beep.h>
#elif defined _WIN32
//	#include <wrl.h>// includes the WINAPI_FAMILY_DESKTOP_APP flag
//	#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
//  we do this check so we're not going to call the Beep function from windows.h over UWP apps.
	#ifndef WINAPI_FAMILY // not the correct way but it works for now
    #include <windows.h>
	#endif
#elif defined __MSDOS__
    #include <dos.h>
#elif defined linux
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <linux/kd.h>
#endif
#ifdef SDL_USE
	#include <MT2D/SDL/MT2D_SDL_main.h>
#endif

void MT2D_System_Delay(int time_millisecond){
#ifdef SDL_USE
    SDL_MT2DDelay(time_millisecond);
#elif defined _WIN32 /* && !defined WINAPI_FAMILY*//*exclude windows uwp*/
//I'll fix this shit later in UWP
	Sleep(time_millisecond);
#elif defined __MSDOS__
	delay(time_millisecond);//ms-dos
#elif defined linux
    usleep(time_millisecond);
#endif
}


void MT2D_System_Beep(int frequency, int time_milisecond){
#ifdef SDL_USE_AUDIO
    SDL_Beep(frequency,time_milisecond);
    //not implemented
#elif defined(_WIN32 ) && !defined (WINAPI_FAMILY)
	Beep(frequency,time_milisecond);//Do not load this beep function in UWP apps
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

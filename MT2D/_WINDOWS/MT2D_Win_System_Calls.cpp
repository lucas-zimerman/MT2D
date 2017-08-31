#include "../MT2D_Terminal_Define.h"
#if defined(WINDOWS_TARGET) && !defined(SDL_USE)
#include <windows.h>

void MT2D_Win_System_Delay(int time_millisecond) {
//#elif defined _WIN32 /* && !defined WINAPI_FAMILY*//*exclude windows uwp*/
	//I'll fix this shit later in UWP
	Sleep(time_millisecond);
//#endif
}


void MT2D_System_Beep(int frequency, int time_milisecond) {
	Beep(frequency, time_milisecond);//Do not load this beep function in UWP apps
}
#endif

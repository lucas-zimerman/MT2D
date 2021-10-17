#include <MT2D/MT2D_Terminal_Define.h>

#if defined(SDL_USE)
#include <SDL_timer.h>
#include "Audio\MT2D_SDL_Beep.h"

void MT2D_SDL_System_Delay(int time_millisecond) {
	SDL_Delay((Uint32)time_millisecond);
}


void MT2D_SDL_System_Beep(int frequency, int time_milisecond) {
	SDL_Beep(frequency, time_milisecond);
}
#endif
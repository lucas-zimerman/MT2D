#include "../MT2D_Terminal_Define.h"
#if !defined(MTSDL_SYSCALL_H) && defined(SDL_USE)
#define MTSDL_SYSCALL_H
void MT2D_SDL_System_Delay(int time_millisecond);
void MT2D_SDL_System_Beep(int frequency, int time_milisecond);
#endif
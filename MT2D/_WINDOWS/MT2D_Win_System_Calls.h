#include "../MT2D_Terminal_Define.h"
#if !defined(MTWIN_SYSCALL_H) && defined(_WIN32) && !defined(SDL_USE)
#define MTWIN_SYSCALL_H
void MT2D_Win_System_Delay(int time_millisecond);
void MT2D_Win_System_Beep(int frequency, int time_milisecond);
#endif
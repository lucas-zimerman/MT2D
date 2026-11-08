#include <MT2D/ObjectCore/MT2D_CDecl.h>
#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE) && !defined(MTLIN_SYSCALL_H)
#define MTLIN_SYSCALL_H
MT2D_C_BEGIN
void MT2D_Linux_System_Delay(int time_millisecond);
void MT2D_Linux_System_Beep(int frequency, int time_milisecond);
MT2D_C_END
#endif
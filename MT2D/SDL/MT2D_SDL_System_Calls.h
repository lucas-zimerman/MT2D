#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../MT2D_Terminal_Define.h"
#if !defined(MTSDL_SYSCALL_H) && defined(SDL_USE)
#define MTSDL_SYSCALL_H
MT2D_C_BEGIN
void MT2D_SDL_System_Delay(int time_millisecond);
void MT2D_SDL_System_Beep(int frequency, int time_milisecond);
MT2D_C_END
#endif
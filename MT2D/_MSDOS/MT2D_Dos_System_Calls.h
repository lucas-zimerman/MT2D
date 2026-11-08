#include <MT2D/ObjectCore/MT2D_CDecl.h>
#if defined(__MSDOS__) || defined(__DOS__) && !defined(MTDOS_SYSCALL_H)
#define MTDOS_SYSCALL_H
MT2D_C_BEGIN
void MT2D_Dos_System_Delay(int time_millisecond);
void MT2D_Dos_System_Beep(int frequency, int time_milisecond);
MT2D_C_END
#endif

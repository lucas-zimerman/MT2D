#include <MT2D/ObjectCore/MT2D_CDecl.h>
#if  defined(__MSDOS__)  || defined(__DOS__) && !defined(MTDOS_JOY_H)
#define MTDOS_JOY_H
MT2D_C_BEGIN
void MT2D_Dos_Joystick_Init(void);
void MT2D_Dos_Joystick_Close(void);
void MT2D_Dos_Joystick_Update(void);
MT2D_C_END
#endif

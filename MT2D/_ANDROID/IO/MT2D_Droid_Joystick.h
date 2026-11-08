#include <MT2D/ObjectCore/MT2D_CDecl.h>
#if  defined(__ANDROID__) && !defined(DROID_JOY_H)
#define DROID_JOY_H
MT2D_C_BEGIN
void MT2D_Droid_Joystick_Init(void);
void MT2D_Droid_Joystick_Close(void);
void MT2D_Droid_Joystick_Update(void);
MT2D_C_END
#endif
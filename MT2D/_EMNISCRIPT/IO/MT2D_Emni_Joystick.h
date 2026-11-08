#include <MT2D/ObjectCore/MT2D_CDecl.h>
#if  defined(__EMSCRIPTEN__) && !defined(EM_JOY_H)
#define EM_JOY_H
MT2D_C_BEGIN
void MT2D_Ems_Joystick_Init(void);
void MT2D_Ems_Joystick_Close(void);
void MT2D_Ems_Joystick_Update(void);
MT2D_C_END
#endif
#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../../MT2D_Terminal_Define.h"
//#if  defined(WINDOWS_TARGET) && !defined(MTWIN_JOY_H)
//#define MTWIN_JOY_H
MT2D_C_BEGIN
void MT2D_Win_Joystick_Init(void);
void MT2D_Win_Joystick_Close(void);
void MT2D_Win_Joystick_Update(void);
MT2D_C_END
//#endif
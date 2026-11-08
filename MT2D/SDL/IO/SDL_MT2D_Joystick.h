#include <MT2D/ObjectCore/MT2D_CDecl.h>
#ifndef SDL_MT2DJoy
#define SDL_MT2DJoy

#include "../../MT2D_Terminal_Define.h"
#ifdef SDL_USE
#include <SDL_joystick.h>
#include "../../Joystick/MT2D_Joystick.h"

MT2D_C_BEGIN
SDL_Joystick* SDL_LoadJoystick(void);
void MT2D_SDL_Update_Keys(MT2D_Joystick *MyJosytick);
void SDL_Joystick_Init(void);
/* Desktop SDL implementation of MT2D_Joystick_Init / Close / Update (arrow keys act as the left stick) */
void MT2D_SDL_Joystick_Init(void);
void MT2D_SDL_Joystick_Close(void);
void MT2D_SDL_Joystick_Update(void);
MT2D_C_END
#endif
#endif
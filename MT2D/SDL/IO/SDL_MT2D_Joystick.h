#ifndef SDL_MT2DJoy
#define SDL_MT2DJoy

#include "../../MT2D_Terminal_Define.h"
#ifdef SDL_USE
#include <SDL_joystick.h>
#include "../../Joystick/MT2D_Joystick.h"

SDL_Joystick* SDL_LoadJoystick();
void MT2D_SDL_Update_Keys(MT2D_Joystick *MyJosytick);
void SDL_Joystick_Init();
#endif
#endif
#ifndef SDL_MT2DJoy
#define SDL_MT2DJoy

typedef struct MT2D_Joystick;

SDL_Joystick* SDL_LoadJoystick();
void MT2D_SDL_Update_Keys(MT2D_Joystick *MyJosytick);
void SDL_Joystick_Init();
#endif
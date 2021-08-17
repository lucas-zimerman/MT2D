/*code can be used with SDL*/
#ifdef __EMSCRIPTEN__
#include <MT2D/MT2D_Terminal_Define.h>

#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/MT2D_Debug.h>
#if  defined(SDL_USE)
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/IO/SDL_MT2D_Joystick.h>
#include <MT2D/MT2D_Keyboard.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#endif
#include <cstdlib>
#include <cstdio>

extern MT2D_Joystick *GlobalJoystickHandler;


void MT2D_Ems_Joystick_Init() {
	GlobalJoystickHandler = (MT2D_Joystick*)malloc(sizeof(MT2D_Joystick));
	GlobalJoystickHandler->Button_Pressed[0] = false;
	GlobalJoystickHandler->Button_Pressed[1] = false;
	GlobalJoystickHandler->Button_Pressed[2] = false;
	GlobalJoystickHandler->Button_Pressed[3] = false;
	GlobalJoystickHandler->Left_Modified = false;
	GlobalJoystickHandler->Left_X_axis = 0;
	GlobalJoystickHandler->Left_Y_axis = 0;
	GlobalJoystickHandler->Right_Modified = false;
	GlobalJoystickHandler->Right_X_axis = 0;
	GlobalJoystickHandler->Right_Y_axis = 0;
	GlobalJoystickHandler->Touched = false;
	GlobalJoystickHandler->DeadZoneAxis = 8000;
#ifdef SDL_USE
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "1");
	const char *B = SDL_GetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK);
	GlobalJoystickHandler->gGameController = SDL_LoadJoystick();
	SDL_Joystick_Init();
#endif

}
void MT2D_Ems_Joystick_Close() {
#ifdef SDL_USE
	if (GlobalJoystickHandler->gGameController != 0) {
		SDL_JoystickClose(GlobalJoystickHandler->gGameController);
	}
#endif
	free(GlobalJoystickHandler);
}

void MT2D_Ems_Joystick_Update() {
	int Key = 0;
	int hardcoredebug = 0;
	//clear the joystick buffer.
	GlobalJoystickHandler->Button_Pressed[0] = false;
	GlobalJoystickHandler->Button_Pressed[1] = false;
	GlobalJoystickHandler->Button_Pressed[2] = false;
	GlobalJoystickHandler->Button_Pressed[3] = false;
	GlobalJoystickHandler->Right_Modified = false;
	GlobalJoystickHandler->Right_X_axis = 0;
	GlobalJoystickHandler->Right_Y_axis = 0;
#ifdef SDL_USE
	MT2D_SDL_Event_Handler();
	MT2D_SDL_Update_Keys(GlobalJoystickHandler);
#endif
}

#endif
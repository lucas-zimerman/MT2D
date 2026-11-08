/*code can be used with SDL*/
#ifdef _WIN32
#include "../../MT2D_Terminal_Define.h"

#include "../../Joystick/MT2D_Joystick.h"
#include "../../MT2D_Debug.h"

#include <stdlib.h>
#include <stdio.h>
#if  defined(SDL_USE)
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/IO/SDL_MT2D_Joystick.h>
#include <MT2D/MT2D_Keyboard.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#endif

extern MT2D_Joystick *GlobalJoystickHandler;

/* UWP accelerometer: C++/CX, lives in MT2D_Win_Uwp.cpp */
#if !(!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
#include "MT2D_Win_Uwp.h"
#endif

void MT2D_Win_Joystick_Init(void) {
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
	MT2D_Ide_Printf((char*)B);
	GlobalJoystickHandler->gGameController = SDL_LoadJoystick();
	SDL_Joystick_Init();
#endif
#if !(!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
	MT2D_Win_Uwp_Accelerometer_Start();
#endif
}
void MT2D_Win_Joystick_Close(void) {
#ifdef SDL_USE
	if (GlobalJoystickHandler->gGameController != 0) {
		MT2D_SDL_Update_Keys(GlobalJoystickHandler);
		GlobalJoystickHandler->Button_Pressed[0] = false;
		GlobalJoystickHandler->Button_Pressed[1] = false;
		GlobalJoystickHandler->Button_Pressed[2] = false;
		GlobalJoystickHandler->Button_Pressed[3] = false;
		GlobalJoystickHandler->Right_Modified = false;
		GlobalJoystickHandler->Left_Modified = false;
		GlobalJoystickHandler->Right_X_axis = 0;
		GlobalJoystickHandler->Right_Y_axis = 0;
		SDL_JoystickClose(GlobalJoystickHandler->gGameController);
	}
#endif
#if !(!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
	MT2D_Win_Uwp_Accelerometer_Stop();
#endif
	free(GlobalJoystickHandler);
}

void MT2D_Win_Joystick_Update(void) {
	int Key = 0;
	int hardcoredebug = 0;
	//clear the joystick buffer.
	GlobalJoystickHandler->Button_Pressed[0] = false;
	GlobalJoystickHandler->Button_Pressed[1] = false;
	GlobalJoystickHandler->Button_Pressed[2] = false;
	GlobalJoystickHandler->Button_Pressed[3] = false;
	GlobalJoystickHandler->Right_Modified = false;
	GlobalJoystickHandler->Left_Modified = false;
	GlobalJoystickHandler->Right_X_axis = 0;
	GlobalJoystickHandler->Right_Y_axis = 0;
#ifdef SDL_USE
	MT2D_SDL_Event_Handler();
	MT2D_SDL_Update_Keys(GlobalJoystickHandler);
#endif
}

#endif
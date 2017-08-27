#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <cstdlib>
#include <cstdio>
#ifdef SDL_USE
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/IO/SDL_MT2D_Joystick.h>
#include <MT2D/MT2D_Keyboard.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#include <MT2D/MT2D_Debug.h>
#endif

MT2D_Joystick *GlobalJoystickHandler;

void MT2D_Joystick_Init() {
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
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "1");
	const char *B = SDL_GetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK);
	MT2D_Ide_Printf((char*)B);
	GlobalJoystickHandler->gGameController = SDL_LoadJoystick();
#ifdef SDL_USE
	SDL_Joystick_Init();
#endif
}
void MT2D_Joystick_Close() {
#ifdef SDL_USE
	if (GlobalJoystickHandler->gGameController != 0) {
		SDL_JoystickClose(GlobalJoystickHandler->gGameController);
	}
#endif
	free(GlobalJoystickHandler);

}

void MT2D_Joystick_Update() {
	int Key = 0;
	int hardcoredebug = 0;
	//clear the joystick buffer.
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
#ifdef SDL_USE
	MT2D_SDL_Event_Handler();
	MT2D_SDL_Update_Keys(GlobalJoystickHandler);
#endif
}

bool MT2D_Joystick_Touched() {
	return GlobalJoystickHandler->Touched;
}
bool MT2D_Joystick_ArrowKey_Touched() {
	return GlobalJoystickHandler->Left_Modified;
}
bool MT2D_Joystick_LeftArrow_Touched() {
	return GlobalJoystickHandler->Left_Modified;
}
bool MT2D_Joystick_RightArrow_Touched() {
	return GlobalJoystickHandler->Right_Modified;
}
void MT2D_Joystick_Map_Button01(int Keyboard_Key) {
	GlobalJoystickHandler->Mapped_Button[0] = Keyboard_Key;
}
void MT2D_Joystick_Map_Button02(int Keyboard_Key) {
	GlobalJoystickHandler->Mapped_Button[1] = Keyboard_Key;
}
void MT2D_Joystick_Map_Button03(int Keyboard_Key) {
	GlobalJoystickHandler->Mapped_Button[2] = Keyboard_Key;
}
void MT2D_Joystick_Map_Button04(int Keyboard_Key) {
	GlobalJoystickHandler->Mapped_Button[3] = Keyboard_Key;
}
bool MT2D_Joystick_Button_Touched(int ID) {
	bool Output = GlobalJoystickHandler->Button_Pressed[ID - 1];
	GlobalJoystickHandler->Button_Pressed[ID - 1] = false;
	GlobalJoystickHandler->Touched = false;
	return Output;
}

int MT2D_Joystick_Get_LeftAxisX() {
	GlobalJoystickHandler->Left_Modified = false;
	GlobalJoystickHandler->Touched = false;
	return GlobalJoystickHandler->Left_X_axis;
}
int MT2D_Joystick_Get_LeftAxisY() {
	GlobalJoystickHandler->Left_Modified = false;
	GlobalJoystickHandler->Touched = false;
	return GlobalJoystickHandler->Left_Y_axis;
}
int MT2D_Joystick_Get_RightAxisX() {
	GlobalJoystickHandler->Right_Modified = false;
	GlobalJoystickHandler->Touched = false;
	return GlobalJoystickHandler->Right_X_axis;
}
int MT2D_Joystick_Get_RightAxisY() {
	GlobalJoystickHandler->Right_Modified = false;
	GlobalJoystickHandler->Touched = false;
	return GlobalJoystickHandler->Right_X_axis;
}

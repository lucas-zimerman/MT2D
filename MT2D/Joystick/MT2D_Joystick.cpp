#include "../MT2D_Terminal_Define.h"
#include "../Joystick/MT2D_Joystick.h"
#include <cstdlib>
#include <cstdio>

MT2D_Joystick *GlobalJoystickHandler;



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

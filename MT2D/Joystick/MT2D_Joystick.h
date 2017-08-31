/**
	Created by: Lucas Zimerman Fraulob
	This code deals with joystick and also with keyboard where you can read multiple keys at once.
	This code may affect the MT2D_Keyboard code, so you better not use both at the same time.
**/


/**
	X/Y axis goes from -32768 to 32767
**/
#ifndef MTJOY_H
#define MTJOY_H
#include <MT2D/MT2D_Terminal_Define.h>

#ifdef SDL_USE
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#endif

typedef struct MT2D_Joystick {
	int Left_X_axis;
	int Left_Y_axis;
	bool Left_Modified;
	int Right_X_axis;
	int Right_Y_axis;
	bool Right_Modified;
	bool Button_Pressed[4];
	int Mapped_Button[4];// return as a normal keyboard button
	bool Touched;
	int DeadZoneAxis;
#ifdef SDL_USE
	SDL_Joystick* gGameController;
#endif
};


#ifdef WINDOWS_TARGET
#include "../_WINDOWS/IO/MT2D_Win_Joystick.h"
#define MT2D_Joystick_Init MT2D_Win_Joystick_Init
#define MT2D_Joystick_Close  MT2D_Win_Joystick_Close
#define MT2D_Joystick_Update MT2D_Win_Joystick_Update
#elif defined(__MSDOS__)
#include "../_MSDOS/IO/MT2D_Dos_Joystick.h"
#define MT2D_Joystick_Init MT2D_Dos_Joystick_Init
#define MT2D_Joystick_Close MT2D_Dos_Joystick_Close
#define MT2D_Joystick_Update MT2D_Dos_Joystick_Update
#else
#error "No code was done for this platform"
#endif

bool MT2D_Joystick_Touched();
bool MT2D_Joystick_ArrowKey_Touched();
bool MT2D_Joystick_LeftArrow_Touched();
bool MT2D_Joystick_RightArrow_Touched();
void MT2D_Joystick_Map_Button01(int Keyboard_Key);
void MT2D_Joystick_Map_Button02(int Keyboard_Key);
void MT2D_Joystick_Map_Button03(int Keyboard_Key);
void MT2D_Joystick_Map_Button04(int Keyboard_Key);
bool MT2D_Joystick_Button_Touched(int ID);
int MT2D_Joystick_Get_LeftAxisX();
int MT2D_Joystick_Get_LeftAxisY();
int MT2D_Joystick_Get_RightAxisX();
int MT2D_Joystick_Get_RightAxisY();
#endif
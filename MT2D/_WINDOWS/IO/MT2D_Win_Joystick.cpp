/*code can be used with SDL*/
#ifdef _WIN32
#include "../../MT2D_Terminal_Define.h"

#include "../../Joystick/MT2D_Joystick.h"
#include "../../MT2D_Debug.h"

#include <cstdlib>
#include <cstdio>
#if  defined(SDL_USE)
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/IO/SDL_MT2D_Joystick.h>
#include <MT2D/MT2D_Keyboard.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#endif

extern MT2D_Joystick *GlobalJoystickHandler;

#pragma region UWP_SPECIFIC_CODE
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
// This code is for Win32 desktop apps
#else
#define MT2D_USING_ACCELEROMETER
// This code is for Windows Store or Windows phone app
#include <process.h>
#include <algorithm>


char test[1000];
using namespace Windows::Devices::Sensors;
bool UWPThreadClose = false;
AccelerometerReading^ WinPhone_Accelerometer;
Accelerometer^ accelerometer;
int AllowMe = 0;
double UWP_Acell_DoubleBuffer;

void ReadingChanged(Accelerometer^ sender, AccelerometerReadingChangedEventArgs^ e)
{
	// We need to dispatch to the UI thread to display the output
	WinPhone_Accelerometer = e->Reading;
	UWP_Acell_DoubleBuffer = WinPhone_Accelerometer->AccelerationX;
	if (UWP_Acell_DoubleBuffer > 0.5)
	{
		UWP_Acell_DoubleBuffer = -32767;
	}
	else if (UWP_Acell_DoubleBuffer < -0.5) {
		UWP_Acell_DoubleBuffer = 32767;
	}
	else if (UWP_Acell_DoubleBuffer >= 0.1) {
		UWP_Acell_DoubleBuffer = (UWP_Acell_DoubleBuffer + 0.5)* -32767;
	}
	else if (UWP_Acell_DoubleBuffer <= -0.1) {
		UWP_Acell_DoubleBuffer = (UWP_Acell_DoubleBuffer - 0.5)* -32767;
	}
	else {
		UWP_Acell_DoubleBuffer = 0;
		GlobalJoystickHandler->Left_Y_axis = 0;
	}
	if (UWP_Acell_DoubleBuffer != 0) {
		GlobalJoystickHandler->Touched = true;
		GlobalJoystickHandler->Left_Modified = true;
		GlobalJoystickHandler->Left_Y_axis = (int)UWP_Acell_DoubleBuffer;
	}
	UWP_Acell_DoubleBuffer = WinPhone_Accelerometer->AccelerationY;
	if (UWP_Acell_DoubleBuffer > 0.5)
	{
		UWP_Acell_DoubleBuffer = -32767;
	}
	else if (UWP_Acell_DoubleBuffer < -0.5) {
		UWP_Acell_DoubleBuffer = 32767;
	}
	else if (UWP_Acell_DoubleBuffer >= 0.1) {
		UWP_Acell_DoubleBuffer = (UWP_Acell_DoubleBuffer + 0.5)* -32767;
	}
	else if (UWP_Acell_DoubleBuffer <= -0.1) {
		UWP_Acell_DoubleBuffer = (UWP_Acell_DoubleBuffer - 0.5)* -32767;
	}
	else {
		UWP_Acell_DoubleBuffer = 0;
		GlobalJoystickHandler->Left_X_axis = 0;
	}
	if (UWP_Acell_DoubleBuffer != 0) {
		GlobalJoystickHandler->Touched = true;
		GlobalJoystickHandler->Left_X_axis = (int)UWP_Acell_DoubleBuffer;
		GlobalJoystickHandler->Left_Modified = true;
	}
}

void MT2D_Joystick_UWPAcellerometer(void *param) {
	accelerometer = Windows::Devices::Sensors::Accelerometer::GetDefault();
	if (accelerometer) {
		accelerometer->ReportInterval = (std::max)(accelerometer->MinimumReportInterval, 16U);;
		accelerometer->ReadingChanged += ref new Windows::Foundation::TypedEventHandler<Accelerometer^,
			AccelerometerReadingChangedEventArgs^>(ReadingChanged);
		double tmp;
		while (!UWPThreadClose) {
			MT2D_System_Delay(10);
		}
	}
	//else this device has no accelerometer
	UWPThreadClose = false;
}

#endif

#pragma endregion
void MT2D_Win_Joystick_Init() {
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
	_beginthread(MT2D_Joystick_UWPAcellerometer, 0, 0);
#endif
}
void MT2D_Win_Joystick_Close() {
#ifdef SDL_USE
	if (GlobalJoystickHandler->gGameController != 0) {
		SDL_JoystickClose(GlobalJoystickHandler->gGameController);
	}
#endif
#if !(!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
	UWPThreadClose = true;
	while (UWPThreadClose == true) {
		MT2D_System_Delay(1);
	}
#endif
	free(GlobalJoystickHandler);
}

void MT2D_Win_Joystick_Update() {
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
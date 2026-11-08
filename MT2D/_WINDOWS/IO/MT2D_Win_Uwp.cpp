/*
	Windows Store / Phone (UWP) accelerometer reader for the joystick.
	This is C++/CX (ref new, ^ handles), so it is the one engine file that stays C++.
	MT2D_Win_Joystick.c starts and stops it through MT2D_Win_Uwp_Accelerometer_Start/Stop.
*/
#include "../../MT2D_Terminal_Define.h"

#if defined(_WIN32) && !(!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
#define MT2D_USING_ACCELEROMETER
#include "../../Joystick/MT2D_Joystick.h"
#include "../../MT2D_System_Calls.h"
#include "MT2D_Win_Uwp.h"
#include <process.h>
#include <algorithm>

extern "C" MT2D_Joystick *GlobalJoystickHandler;

using namespace Windows::Devices::Sensors;
static bool UWPThreadClose = false;
static AccelerometerReading^ WinPhone_Accelerometer;
static Accelerometer^ accelerometer;
static double UWP_Acell_DoubleBuffer;

static void ReadingChanged(Accelerometer^ sender, AccelerometerReadingChangedEventArgs^ e)
{
	WinPhone_Accelerometer = e->Reading;
	UWP_Acell_DoubleBuffer = WinPhone_Accelerometer->AccelerationX;
	GlobalJoystickHandler->Left_Modified = true;
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

static void MT2D_Joystick_UWPAcellerometer(void *param) {
	accelerometer = Windows::Devices::Sensors::Accelerometer::GetDefault();
	if (accelerometer) {
		accelerometer->ReportInterval = (std::max)(accelerometer->MinimumReportInterval, 16U);
		accelerometer->ReadingChanged += ref new Windows::Foundation::TypedEventHandler<Accelerometer^,
			AccelerometerReadingChangedEventArgs^>(ReadingChanged);
		while (!UWPThreadClose) {
			MT2D_System_Delay(10);
		}
	}
	//else this device has no accelerometer
	UWPThreadClose = false;
}

extern "C" void MT2D_Win_Uwp_Accelerometer_Start(void) {
	_beginthread(MT2D_Joystick_UWPAcellerometer, 0, 0);
}

extern "C" void MT2D_Win_Uwp_Accelerometer_Stop(void) {
	UWPThreadClose = true;
	while (UWPThreadClose == true) {
		MT2D_System_Delay(1);
	}
}
#endif

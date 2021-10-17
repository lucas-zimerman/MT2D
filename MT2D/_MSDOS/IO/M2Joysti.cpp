/*code can be used with SDL*/
#if defined(__MSDOS__) || defined(__DOS__)
#include <MT2D/MT2D_Terminal_Define.h>

#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/MT2D_Debug.h>

#include <stdlib.h>
#include <stdio.h>

extern MT2D_Joystick *GlobalJoystickHandler;

void MT2D_Dos_Joystick_Init() {
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
}
void MT2D_Dos_Joystick_Close() {
        free(GlobalJoystickHandler);
}

void MT2D_Dos_Joystick_Update() {
        //clear the joystick buffer.
        GlobalJoystickHandler->Button_Pressed[0] = false;
        GlobalJoystickHandler->Button_Pressed[1] = false;
        GlobalJoystickHandler->Button_Pressed[2] = false;
        GlobalJoystickHandler->Button_Pressed[3] = false;
        GlobalJoystickHandler->Right_Modified = false;
        GlobalJoystickHandler->Right_X_axis = 0;
        GlobalJoystickHandler->Right_Y_axis = 0;
}

#endif

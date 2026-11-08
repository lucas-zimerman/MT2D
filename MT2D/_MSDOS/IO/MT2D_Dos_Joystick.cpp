/*code can be used with SDL*/
#ifdef __MSDOS__
#include <MT2D/MT2D_Terminal_Define.h>

#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/MT2D_Keyboard.h>
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
	int Key = 0;
	int hardcoredebug = 0;
	//clear the joystick buffer.
   GlobalJoystickHandler->Touched = false;
	GlobalJoystickHandler->Button_Pressed[0] = false;
	GlobalJoystickHandler->Button_Pressed[1] = false;
	GlobalJoystickHandler->Button_Pressed[2] = false;
	GlobalJoystickHandler->Button_Pressed[3] = false;
	GlobalJoystickHandler->Right_Modified = false;
	GlobalJoystickHandler->Right_X_axis = 0;
	GlobalJoystickHandler->Right_Y_axis = 0;
	GlobalJoystickHandler->Left_X_axis = 0;
	GlobalJoystickHandler->Left_Y_axis = 0;
   while(MT2D_Keyboard_touched() == true){
      Key = MT2D_Keyboard_keytouched();
      if(Key == arrow_key_pressed){
         Key = MT2D_Keyboard_keytouched();
         GlobalJoystickHandler->Left_Modified = true;
         if(Key == arrow_up_pressed){
         	 GlobalJoystickHandler->Touched = true;
             GlobalJoystickHandler->Left_Y_axis = -32767;
         }else if(Key == arrow_down_pressed){
         	 GlobalJoystickHandler->Touched = true;
             GlobalJoystickHandler->Left_Y_axis = 32767;
         }else if(Key == arrow_left_pressed){
         	 GlobalJoystickHandler->Touched = true;
             GlobalJoystickHandler->Left_X_axis = -32767;
         }else{
             GlobalJoystickHandler->Touched = true;
             GlobalJoystickHandler->Left_X_axis = 32767;
         }
      }
      else if(Key == GlobalJoystickHandler->Mapped_Button[0]){
      	GlobalJoystickHandler->Touched = true;
      	GlobalJoystickHandler->Button_Pressed[0] = 1;
      }
      else if(Key ==  GlobalJoystickHandler->Mapped_Button[1]){
      	GlobalJoystickHandler->Touched = true;
         GlobalJoystickHandler->Button_Pressed[1] = 1;
      }
      else if(Key ==  GlobalJoystickHandler->Mapped_Button[2]){
      	GlobalJoystickHandler->Touched = true;
         GlobalJoystickHandler->Button_Pressed[2] = 1;
      }
   }
}

#endif
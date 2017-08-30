#if !defined(__ANDROID__)
#include "MT2D/MT2D.h"
#include "MT2D/MT2D_Keyboard.h"
#include "MT2D/MT2D_Display.h"
#include "MT2D/MT2D_Layout.h"
#include "MT2D/Cursor/Cursor.h"
#include "MT2D/MT2D_System_Calls.h"
#include "MT2D/MessageBox/MT2D_MessageBox_With_Result.h"
#else
#include "../../MT2D.h"
#include "../../MT2D_Keyboard.h"
#include "../../MT2D_Display.h"
#include "../../MT2D_Layout.h"
#include "../../Cursor/Cursor.h"
#include "../../MT2D_System_Calls.h"
#include "../../MessageBox/MT2D_MessageBox_With_Result.h"

#endif
#include <stdio.h>
#include <stdlib.h>

extern unsigned char WINDOW1[MAX_VER+1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER+1][MAX_HOR];

int MT2D_Menu(char *title_menu,int title_position,int horizontal_position,int vertical_position,int style,bool shadow,char background,char cursor,char *option1,char *option2,char *option3,char *option4,char *option5,char *option6,char *option7,char *option8,char *option9,char *option10,char *option11,char *option12,char *option13,char *option14,char *option15,char *option16,char *option17,char *option18,int total_option){
	int size_x=51;
	int size_y=5+total_option;
	int selected_option=0;
	int i=0,j=0,buff=0;
	int position_select=1;
	int l1,j1;
	if(vertical_position+5+total_option>=24){

		while(vertical_position+6+total_option>23)vertical_position--;
	}if(size_x<54)size_x=55;
	j1=vertical_position+4;
	l1=horizontal_position+3;
	if(total_option<=0){
		total_option=MT2D_MessageBox_With_Result("DEV MENU ERROR","NO OPTION RECEIVED","EXIT","BREAK FUNCTION");
		if(total_option==1)exit(-1);
		else return -1;
	}//==========================WINDOW STARTUP ==================================================================
	transfer_window1_to_window2();
	MT2D_Create_Window_Layout(horizontal_position,vertical_position,size_x,size_y,shadow,true,false,background,DISPLAY_WINDOW2);
	insert_string_on_display(title_menu,vertical_position+1,horizontal_position+1+title_position,DISPLAY_WINDOW2);
	insert_string_on_display("Select an option with the keys or press the option key",vertical_position+3,horizontal_position+1,DISPLAY_WINDOW2);
	while(i<total_option){
		WINDOW2[j1][l1]=')';
		WINDOW2[j1][l1-1]='1'+i;
		switch(i){
		case 0: 	insert_string_on_display(option1,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 1: 	insert_string_on_display(option2,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 2: 	insert_string_on_display(option3,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 3: 	insert_string_on_display(option4,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 4: 	insert_string_on_display(option5,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 5: 	insert_string_on_display(option6,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 6: 	insert_string_on_display(option7,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 7: 	insert_string_on_display(option8,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 8: 	insert_string_on_display(option9,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 9: 	insert_string_on_display(option10,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 10: 	insert_string_on_display(option11,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 11: 	insert_string_on_display(option12,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 12: 	insert_string_on_display(option13,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 13: 	insert_string_on_display(option14,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 14: 	insert_string_on_display(option15,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 15: 	insert_string_on_display(option16,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 16: 	insert_string_on_display(option17,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		case 17: 	insert_string_on_display(option18,vertical_position+4+i,horizontal_position+4,DISPLAY_WINDOW2); break;
		}
		j1++;
		i++;
//==================================MENU CORE ===========================================
	}while(!selected_option){
		MT2D_Draw_Window(DISPLAY_WINDOW2);
		MT2D_Cursor_Set_Position(vertical_position+4+position_select-1,horizontal_position+2);
		while(!MT2D_Keyboard_touched());
		buff=MT2D_Keyboard_keytouched();
		if(buff==arrow_key_pressed){
			buff=MT2D_Keyboard_keytouched();
			switch(buff){
			case arrow_up_pressed:
				if(position_select==1){
					position_select=total_option;
					WINDOW2[vertical_position+4+position_select-1][horizontal_position+1]='>';
					WINDOW2[vertical_position+4+position_select-total_option][horizontal_position+1]=background;

				}else{
					position_select--;
					WINDOW2[vertical_position+4+position_select-1][horizontal_position+1]='>';
					WINDOW2[vertical_position+4+position_select][horizontal_position+1]=background;
				}
				break;
			case arrow_down_pressed:
				if(position_select==total_option){
					position_select=1;
					WINDOW2[vertical_position+4+position_select-1][horizontal_position+1]='>';
					WINDOW2[vertical_position+4+position_select+total_option-2][horizontal_position+1]=background;
				}else{
					position_select++;
					WINDOW2[vertical_position+4+position_select-1][horizontal_position+1]='>';
					WINDOW2[vertical_position+4+position_select-2][horizontal_position+1]=background;
				}break;
			default: buff=0;
			}
		}
		else if(buff>='1' && buff<='1'+total_option){
			if(total_option>buff-'1'){
				selected_option=buff-'0';
			}
		}else if(buff==enter_pressed) selected_option=position_select;
	}
	return selected_option;
}

int MT2D_Short_Menu(char *title_menu,int title_position,int horizontal_position,int vertical_position,bool shadow,char background,char cursor,char *option1,char *option2,char *option3,char *option4){
	int selected_option=0;
	int total_options=0;
	if(option1!=0) total_options++;
	if(option2!=0) total_options++;
	if(option3!=0) total_options++;
	if(option4!=0) total_options++;
	if(total_options) selected_option=MT2D_Menu(title_menu,title_position,horizontal_position,vertical_position,0,shadow,background,cursor,option1,option2,option3,option4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,total_options);

	return selected_option;
}
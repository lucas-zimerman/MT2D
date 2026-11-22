#ifndef __ANDROID__
#include "MT2D/MT2D.h"
#include "MT2D/MT2D_Keyboard.h"
#include "MT2D/MT2D_Display.h"
#include "MT2D/MT2D_Layout.h"
#include "MT2D/MT2D_System_Calls.h"
#else
#include "../MT2D.h"
#include "../MT2D_Keyboard.h"
#include "../MT2D_Display.h"
#include "../MT2D_Layout.h"
#include "../MT2D_System_Calls.h"
#endif
#include <stdio.h>
#include <string.h>

#define MSGBOX_AREA_W 75 /* box (73 + 1) + shadow */
#define MSGBOX_AREA_H 6  /* box (4 + 1) + shadow */

void MT2D_MessageBox(char *mensage){
	static unsigned char covered[MSGBOX_AREA_W * MSGBOX_AREA_H]; /* WINDOW1 cells under the box */
	int i=0,j=0,mensage_size;
	int pos_hor=4,pos_ver=10;
	int win_hor=3,win_ver=9;
	bool out_loop=false;
	mensage_size=strlen(mensage);
	printf("\a");
	while(out_loop==false){
		i=0;
		j=0;
		pos_hor=win_hor+1;
		pos_ver=win_ver+1;
		MT2D_Window_Save_Area(covered,win_hor,win_ver,MSGBOX_AREA_W,MSGBOX_AREA_H,DISPLAY_WINDOW1);
		MT2D_Create_Window_Layout(win_hor,win_ver,73,4,true,false,false,' ',DISPLAY_WINDOW1);
		j=0;
		while(j<mensage_size){ /* Put the mensage on the window */
			if(pos_hor==win_hor+70){pos_hor=win_hor+1;pos_ver++;}
			if(pos_ver==win_ver+4 )break;
			WINDOW1[pos_ver][pos_hor]=mensage[j];
			j++;
			pos_hor++;
		}
		MT2D_Draw_Window(DISPLAY_WINDOW1);
		MT2D_Window_Restore_Area(covered,win_hor,win_ver,MSGBOX_AREA_W,MSGBOX_AREA_H,DISPLAY_WINDOW1);
		i=MT2D_Keyboard_keytouched();
		if(i==arrow_key_pressed){ /* MOVE THE WINDOW POSITION */
			i=MT2D_Keyboard_keytouched();
			switch(i){
			case arrow_up_pressed:
				if(win_ver>0)win_ver--;
				out_loop=false;
				break;
			case arrow_down_pressed:
				if(win_ver<MAX_VER-5)win_ver++;
				out_loop=false;
				break;
			case arrow_left_pressed:
				if(win_hor>0)win_hor--;
				out_loop=false;
				break;
			case arrow_right_pressed:
				if(win_hor<MAX_HOR-74)win_hor++;
				out_loop=false;
				break;
			}
		}
		else out_loop=true;
	}
}

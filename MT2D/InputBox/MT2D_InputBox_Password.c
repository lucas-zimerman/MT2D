#include <stdio.h>
#include <stdlib.h>
#if !defined(__ANDROID__)
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
#include <string.h>

char *MT2D_InputBox_Password(char *mensage){
	static unsigned char covered[75 * 8]; /* WINDOW1 cells under the 73x6 box and its shadow */
/* ESC return a null value, delete erase the last char, key left key right move the cursor. */
/* Enter finish the string */
	char *string=0;
	int string_size=0;
	char buff; /* Buffer to get char */
	int pos_hor=4,pos_ver=10; /*  */
	int win_hor=3,win_ver=9; /* Position where the window will start to be */
	int mensage_size=strlen(mensage);
	int j=0,i=0,k=0;
	bool out_loop=false;
	string=(char*)malloc(sizeof(char));
	string[0]='\0';
	while(out_loop==false){
		i=0;
		j=0;
		pos_hor=win_hor+1;
		pos_ver=win_ver+1;
		MT2D_Window_Save_Area(covered,win_hor,win_ver,75,8,DISPLAY_WINDOW1);
		MT2D_Create_Window_Layout(win_hor,win_ver,73,6,true,false,false,' ',DISPLAY_WINDOW1);
		while(j<mensage_size){ /* Put the mensage on the window */
				if(pos_hor==win_hor+70){pos_hor=win_hor+1;pos_ver++;}
				if(pos_ver==win_ver+4 )break;
				WINDOW1[pos_ver][pos_hor]= mensage[j];
				j++;
				pos_hor++;
		}
		for(i=win_hor+2,  j=win_ver+3;i<win_hor+73;i++){
			WINDOW1[j][i]=219;
		}
		if(string_size>0){
			if(string_size>70)	k=string_size-71;
			else k=0;
			for(i=win_hor+2,  j=win_ver+3;k<string_size;i++, k++){
				WINDOW1[j][i] = '*'; /* string[k]; */
			}
		}
		MT2D_Draw_Window(DISPLAY_WINDOW1);
		MT2D_Window_Restore_Area(covered,win_hor,win_ver,75,8,DISPLAY_WINDOW1);
		while (!MT2D_Keyboard_touched()) {
			MT2D_System_Delay(1);
		}
		buff=MT2D_Keyboard_keytouched();
		if(buff==arrow_key_pressed){
			buff=MT2D_Keyboard_keytouched();
			buff=0;
		}else if(buff==back_space_pressed){
			if(string_size>1){
				string_size--;
				string=(char*)realloc(string,(string_size+1)*sizeof(char));
				string[string_size]='\0';
			}else{
				string_size=0;
				string[0]=' ';
			}
		}else if(buff==27 || buff==enter_pressed){
			if(buff==27){
				string=(char*)realloc(string,sizeof(char));
				string[0]='\0';
			}
			out_loop=true;
		}else if(buff==escape_pressed){
			string_size=0;
			free(string);
			string=0;
			out_loop=true;
		}else{
			string_size++;
			string=(char*)realloc(string,(string_size+1)*sizeof(char));
			string[string_size]='\0';
			string[string_size-1]=buff;
		}
	}
	return string;
}

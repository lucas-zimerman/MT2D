#include <stdio.h>
#include <stdlib.h>

#include "MT2D/MT2D.h"
#include "MT2D/MT2D_Keyboard.h"
#include "MT2D/MT2D_Display.h"
#include "MT2D/MT2D_Layout.h"
#include "MT2D/MT2D_System_Calls.h"
#include <string.h>

extern unsigned char WINDOW1[MAX_VER+1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER+1][MAX_HOR];

int MT2D_InputBox_Integer(char *mensage){
//ESC return a null value, delete erase the last char, key left key right move the cursor.
//Enter finish the string
	int output_integer=0;
	char *string=0;
	int string_size=0;
	char buff;//Buffer to get char
	int pos_hor=4,pos_ver=10;//
	int win_hor=3,win_ver=9;// Position where the window will start to be
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
		while(i<MAX_VER-1	){
			while(j<MAX_HOR){
				WINDOW2[i][j]=WINDOW1[i][j];
				j++;
			}
			i++;
			j=0;
		}
		MT2D_Create_Window_Layout(win_hor,win_ver,73,6,true,false,false,' ',DISPLAY_WINDOW2);
		while(j<mensage_size){//Put the mensage on the window
				if(pos_hor==win_hor+70){pos_hor=win_hor+1;pos_ver++;}
				if(pos_ver==win_ver+4 )break;
				WINDOW2[pos_ver][pos_hor]=mensage[j];
				j++;
				pos_hor++;
		}
		for(i=win_hor+2,  j=win_ver+3;i<win_hor+73;i++){
			WINDOW2[j][i]=219;
		}
		if(string_size>0){
			if(string_size>70)	k=string_size-71;
			else k=0;
			for(i=win_hor+2,  j=win_ver+3;k<string_size;i++, k++){
				WINDOW2[j][i]=string[k];
			}
		}
		MT2D_Draw_Window(DISPLAY_WINDOW2);
		while (!MT2D_Keyboard_touched()) {
			MT2D_System_Delay(1);// this avoids a heavy use of processor
		}
		buff = MT2D_Keyboard_keytouched();
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
				string[0]=NULL;
			}
			out_loop=true;
		}else if(buff==escape_pressed){
			string_size=0;
			string=0;
			out_loop=true;
		}else if(buff>='0' && buff<='9'){
			string_size++;
			string=(char*)realloc(string,(string_size+1)*sizeof(char));
			string[string_size]='\0';
			string[string_size-1]=buff;
		}else if(buff=='-'){//only accept the minus signal only if there's no number typed
			if(string_size==0){
				string_size++;
				string=(char*)realloc(string,(string_size+1)*sizeof(char));
				string[string_size]='\0';
				string[string_size-1]=buff;
			}
		}
	}
	output_integer = atoi(string);
	return output_integer;
}

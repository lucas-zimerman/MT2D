#ifndef __ANDROID__
#include "MT2D/MT2D.h"
#include "MT2D/MT2D_Keyboard.h"
#include "MT2D/MT2D_Display.h"
#include "MT2D/MT2D_Layout.h"
#include "MT2D/MT2D_System_Calls.h"
#include "MT2D/Cursor/Cursor.h"
#else
#include "../MT2D.h"
#include "../MT2D_Keyboard.h"
#include "../MT2D_Display.h"
#include "../MT2D_Layout.h"
#include "../MT2D_System_Calls.h"
#include "../Cursor/Cursor.h"
#endif
#include <stdio.h>
#include <string.h>

int MT2D_MessageBox_With_Result(char *error_mensage_title,char *error_mensage, char *option_1, char *option_2){
    //OUTPUTE int 1, int 2
	int i=0,j=0;
	int pos_hor=10,pos_ver=7;
	while(i<MAX_VER){//Transfere window1 para window2
		while(j<MAX_HOR){
			WINDOW2[i][j]=WINDOW1[i][j];
			j++;
		}
		i++;
		j=0;
	}
	MT2D_Create_Window_Layout(pos_hor-1,pos_ver-1,50,8,true,true,true,' ',DISPLAY_WINDOW2);
	i=strlen(error_mensage_title);
	while(j<i){//Escreve titulo do texto em tela
		if(pos_hor>MAX_HOR)break;
		WINDOW2[pos_ver][pos_hor]=error_mensage_title[j];
		j++;
		pos_hor++;
	}

	i=strlen(error_mensage);
	j=0;
	pos_hor=10;
	pos_ver=9;
	while(j<i){//Escreve Texto na tela
		if(pos_hor==59){pos_hor=10;pos_ver++;}
		if(pos_ver==12)break;
		WINDOW2[pos_ver][pos_hor]=error_mensage[j];
		j++;
		pos_hor++;
	}
	i=strlen(option_1);
	j=0;
	pos_hor=13;
	pos_ver=13;
	while(j<i){ //Digita opcao 1 na tela
		if(pos_hor==35)break;
		WINDOW2[pos_ver][pos_hor]=option_1[j];
		j++;
		pos_hor++;
	}
	i=strlen(option_2);
	j=0;
	pos_hor=38;
	pos_ver=13;
	while(j<i){ //Digita opcao 2 na tela
		if(pos_hor==59)break;
		WINDOW2[pos_ver][pos_hor]=option_2[j];
		j++;
		pos_hor++;
	}
	i=10;
	WINDOW2[13][10]='(';WINDOW2[13][11]='1';WINDOW2[13][12]=')';
	WINDOW2[13][35]='(';WINDOW2[13][36]='2';WINDOW2[13][37]=')';
	i=0;
    MT2D_Draw_Window(DISPLAY_WINDOW2);
    MT2D_Cursor_goto_topscreen();
	while(i!='1' && i!='2'){
		i=MT2D_Keyboard_keytouched();
		if(i==224)
			i= MT2D_Keyboard_keytouched();
		if(i!='1' && i!='2'){
			//printf("\a");
		}
	}
	return i-48;
}

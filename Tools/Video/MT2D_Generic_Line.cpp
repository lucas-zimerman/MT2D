#include "MT2D/MT2D.h"

void MT2D_Line_Draw_Vertical(int pos, int type){
 int i=0;
 while(i<MAX_HOR){
	 WINDOW1[pos][i]=type;
	 i++;
 }

}

void MT2D_Line_Draw_Horizontal(int pos, int type){
 int i=0;
 while(i<MAX_VER){
	 WINDOW1[i][pos]=type;
	 i++;
 }

}

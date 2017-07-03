/*Sprites.cpp , created by Lucas Zimerman Fraulob
 this file contains the basic functions related to sprites, it just loads and
 renders it.

*/
#include <stdio.h>
#include <stdlib.h>
#include <MT2D/Objectcore/Sprites.h>
#include <MT2D/MT2D_Keyboard.h> // only used for taking the definition of the key enter
#include <MT2D/MT2D.h> // for geting the display definition

Sprite *Load_Sprite(char *file) {
	Sprite *S=0;
	char BUFF;
	int Xi=0, Yi=0, X=0, Y=0;
	FILE *fl = fopen(file, "rb");//read only
	if (fl) {
		S = (Sprite*)malloc(sizeof(Sprite));
		//STEP 1: count the size of the line and column
		while (!feof(fl)) {
			BUFF = fgetc(fl);
			if (BUFF == enter_pressed) {
				fgetc(fl);// why there's always a '\r' after the \n ?
				if (Xi == 0) {
					Xi = X;
				}
				else if (Xi < X) {
					Xi = X; //no more fixed horizontal size
				}
				X = 0;//only zeroed to avoid overflow...
				Y++;
			}
			X++;
		}
		Yi = Y+1 ;
		Xi = Xi +1;
		S->size.X = Xi;
		S->size.Y = Yi;
		//STEP 2: alloc the memory
		S->Data = (char**)malloc(S->size.Y*sizeof(char*));
		Y = 0;
		while (Y < Yi) {
			S->Data[Y] = (char*)malloc((S->size.X+1) * sizeof(char));
			Y++;
		}
		Y = 0;
		//STEP 3: erase the sprite memory
		while (Y < Yi) {
			X = 0;
			while (X < Xi) {
				S->Data[Y][X] = NULL;
				X++;
			}
			Y++;
		}
		//STEP 4: load the file data over the memory
		rewind(fl);//move the pointer into the top of the file
		X = 0;
		Y = 0;
		while (!feof(fl)) {
			BUFF = fgetc(fl);
			printf("%c", BUFF);
			if (BUFF == enter_pressed) 
			{
				fgetc(fl);// why there's always a '\r' after the \n ?
				X = -1;//only zeroed to avoid overflow...
				Y++;
			}
			else {
				S->Data[Y][X] = BUFF;
			}
			X++;
		}
		fclose(fl);
		S->scale.X = 1;
		S->scale.Y = 1;
	}//else return a null sprite
	return S;
}

bool Sprite_Render_on_Window(Sprite *img, int witch_window, int pos_x, int pos_y) {//output: out = false : invalid sprite | out = true : valid sprite
	//to be implemented: Sprites Scale
	bool out = false;
	int x = 0, x1 = 0;//,x2; not used
	int y = 0, y1 = 0;//,y2; not used
	if (img) {
		out = true;
		x = (int)pos_x + img->size.X;
		y = (int)pos_y + img->size.Y;
		x1 = pos_x;
		y1 = pos_y;
		if (y1<0)y1 = 0;
		else if (y1>24)y1 = y;
		if (x1<0)x1 = 0;
		else if (x1>80)y1 = y;
		if (witch_window == DISPLAY_WINDOW1) {
			while (y1<y) {
				while (x1 < x) {
					if (img->Data[y1 - pos_y][x1 - pos_x] != NULL) {
						if (y1 < MAX_VER && y1 >= 0 && x1 < MAX_HOR && x1 >= 0) {
							WINDOW1[y1][x1] = img->Data[y1 - pos_y][x1 - pos_x];
						}
					}
					x1++;
				}
				x1 = pos_x;
				y1++;
			}
		}
		else {
			while (y1<y) {
				while (x1 < x) {
					if (img->Data[y1 - pos_y][x1 - pos_x] != NULL){
						if (y1 < MAX_VER && y1 >= 0 && x1 < MAX_HOR && x1 >= 0) {
							WINDOW2[y1][x1] = img->Data[y1 - pos_y][x1 - pos_x];
						}
					}
					x1++;
				}
				x1 = pos_x;
				y1++;
			}
		}
	}
	return out;
}
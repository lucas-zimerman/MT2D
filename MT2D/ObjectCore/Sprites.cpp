/*Sprites.cpp , created by Lucas Zimerman Fraulob
 this file contains the basic functions related to sprites, it just loads and
 renders it.

*/
#include <stdio.h>
#include <stdlib.h>
#ifndef __ANDROID__
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/Objectcore/Sprites.h>
#include <MT2D/MT2D_Keyboard.h> // only used for taking the definition of the key enter
#include <MT2D/MT2D.h> // for geting the display definition
#include <MT2D/File/MT2D_File.h>
#else
#include "../MT2D_Terminal_Define.h"
#include "../Objectcore/Sprites.h"
#include "../MT2D_Keyboard.h" // only used for taking the definition of the key enter
#include "../MT2D.h" // for geting the display definition
#include "../File/MT2D_File.h"

#endif

#if defined(SDL_USE) && !defined(__ANDROID__)
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/MT2D_SDL_main.h>
#include <MT2D/SDL/Render/MT2D_SDL_Render.h>
#elif defined(SDL_USE) && defined(__ANDROID__)
#include "../SDL/MT2D_SDL_Redefine.h"
#include "../SDL/MT2D_SDL_main.h"
#include "../SDL/Render/MT2D_SDL_Render.h"
#endif

#if defined(MT2D_USING_CONTAINER)
#include <MT2D/Container/MT2D_Container.h>
#endif
#include <MT2D\MT2D_Debug.h>


Sprite *Load_Sprite(char *file) {
	Sprite *S=0;
	char BUFF;
	int Xi=0, Yi=0, X=0, Y=0;
	MT2D_FILE *fl = MT2D_FILE_OPEN(file, "rb");//read only
	if (fl) {
		S = (Sprite*)malloc(sizeof(Sprite));
		//STEP 1: count the size of the line and column
		while (!MT2D_FILE_EOF(fl)) {
			BUFF = MT2D_FILE_READ_BYTE(fl);
			if (BUFF == enter_pressed) {
				MT2D_FILE_READ_BYTE(fl);// why there's always a '\r' after the \n ?
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
		MT2D_FILE_SEEK(fl,0L,SEEK_SET);//move the pointer into the top of the file
		X = 0;
		Y = 0;
		while (!MT2D_FILE_EOF(fl)) {
			BUFF = MT2D_FILE_READ_BYTE(fl);
			if (BUFF == enter_pressed) 
			{
				MT2D_FILE_READ_BYTE(fl);// why there's always a '\r' after the \n ?
				X = -1;//only zeroed to avoid overflow...
				Y++;
			}
			else {
				S->Data[Y][X] = BUFF;
			}
			X++;
		}
		MT2D_FILE_CLOSE(fl);
		S->scale.X = 1;
		S->scale.Y = 1;
		S->type = 0;
	}//else return a null sprite
	return S;
}

Sprite * Load_Sprite_Image(char * file, int ScaleX, int ScaleY)
{
#ifdef DEBUG_OBJECTCORE
	char buff[200];
	sprintf(buff, "Loading Sprite %s", file);
	MT2D_Ide_Printf(buff);
#endif
#ifdef SDL_USE
	SDL_Surface *Img = 0;
	Sprite *S = 0;
	int Xi = 0, Yi = 0, X = 0, Y = 0;
	MT2D_FILE *fl = MT2D_FILE_OPEN(file, "rb");//read only
	if (fl) {
		MT2D_FILE_CLOSE(fl);
		Img = MT2D_SDL_Load_Image(file);
		S = (Sprite*)malloc(sizeof(Sprite));
		S->Data = (char**)MT2D_SDL_Create_Texture(Img);
		S->RotatedTexture = (char**)MT2D_SDL_Create_Rotated_Texture(Img, (MT2D_SDL_Texture*)S->Data);
		S->size.X = Img->w;
		S->size.Y = Img->h;
		S->scale.X = ScaleX;
		S->scale.Y = ScaleY;
		S->type = 1;
		//we need to kill that surface...
	}//else return a null sprite
	return S;
#else 
#pragma message ("Sprites with image are not supported so we'll return a blank image...")
	Sprite *S = (Sprite*)malloc(sizeof(Sprite));
	S->scale.X = ScaleX;
	S->scale.Y = ScaleY;
	S->size.X = 3;
	S->size.Y = 1;
	S->Data = (char**)malloc(sizeof(char*));
	S->Data[0] = (char*)malloc(3 * sizeof(char));
	S->Data[0][0] = '<';
	S->Data[0][1] = '!';
	S->Data[0][2] = '>';
	S->type = 0;
	return S;
#endif
 }

#if defined(MT2D_USING_CONTAINER)
Sprite *Load_Sprite_Image_From_Container(char *file, int ScaleX, int ScaleY) {
#ifdef SDL_USE
		SDL_Surface *Img = 0;
		Sprite *S = 0;
		int Xi = 0, Yi = 0, X = 0, Y = 0;
		int Id = MT2D_Container_Get_FileId(file);
		//!!! Always open non encrypted image
		if(Id >= 0){
			Img = MT2D_SDL_Load_Image_From_Container(Id);
			S = (Sprite*)malloc(sizeof(Sprite));
			S->Data = (char**)MT2D_SDL_Create_Texture(Img);
			S->RotatedTexture = (char**)MT2D_SDL_Create_Rotated_Texture(Img, (MT2D_SDL_Texture*)S->Data);
			S->size.X = Img->w;
			S->size.Y = Img->h;
			S->scale.X = ScaleX;
			S->scale.Y = ScaleY;
			S->type = 1;
			//we need to kill that surface...
		}//else return a null sprite
		return S;
#else 
#pragma message ("Sprites with image are not supported so we'll return a blank image...")
		Sprite *S = (Sprite*)malloc(sizeof(Sprite));
		S->scale.X = ScaleX;
		S->scale.Y = ScaleY;
		S->size.X = 3;
		S->size.Y = 1;
		S->Data = (char**)malloc(sizeof(char*));
		S->Data[0] = (char*)malloc(3 * sizeof(char));
		S->Data[0][0] = '<';
		S->Data[0][1] = '!';
		S->Data[0][2] = '>';
		S->type = 0;
		return S;
#endif
}
#endif

/*
	Crash Tip:
	-You heap corruption, check pos_x, pos_y to find an object that matches the image.
		That could be caused if you tried to malloc an array of sprites smaller than the number of sprites loaded
	- Actual Frame is outside of the sprite range loaded
*/
 bool Sprite_Render_on_Window(Sprite *img, int witch_window, int pos_x, int pos_y) {//output: out = false : invalid sprite | out = true : valid sprite
	//to be implemented: Sprites Scale
	 bool out = false;
	 int x = 0, x1 = 0;//,x2; not used
	 int y = 0, y1 = 0;//,y2; not used
	 if (img) {
 		 if (img->type == 0) {//render as a sprite
			 out = true;
			 x = (int)pos_x + img->size.X;
			 y = (int)pos_y + img->size.Y;
			 x1 = pos_x;
			 y1 = pos_y;
			 if (y1 < 0)y1 = 0;
			 else if (y1 > 24)y1 = y;
			 if (x1 < 0)x1 = 0;
			 else if (x1 > 80)y1 = y;
			 if (witch_window == DISPLAY_WINDOW1) {
				 while (y1 < y) {
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
				 while (y1 < y) {
					 while (x1 < x) {
						 if (img->Data[y1 - pos_y][x1 - pos_x] != NULL) {
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
		 else {
#ifdef SDL_USE
			 //lets optimize a bit...
			 if (pos_x <= 320 && pos_x + img->scale.X >= 0) {
				 SDL_Add_ImagetoBuffer(img, pos_x, pos_y);
			 }
#endif
		 }
	 }

	return out;
}

 void Sprite_Delete(Sprite *Me) {
	 if (Me) {
		 if (Me->type == 1) {
#ifdef SDL_USE
			 if (Me->RotatedTexture) {
				 MT2D_SDL_DestroyTexture((MT2D_SDL_Texture*)Me->RotatedTexture);
			 }
			 if (Me->Data) {
				 MT2D_SDL_DestroyTexture((MT2D_SDL_Texture*)Me->Data);
			 }
#endif
		 }
		 else {
			 for (int i = 0; i < Me->size.Y; i++) {
				 free(Me->Data[i]);
			 }
			 free(Me->Data);
		 }
		 free(Me);
	 }
 }
/*Sprites.cpp , created by Lucas Zimerman Fraulob
 this file contains the basic functions related to sprites, it just loads and
 renders it.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __ANDROID__
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/MT2D_Keyboard.h> // only used for taking the definition of the key enter
#include <MT2D/MT2D.h> // for geting the display definition
#include <MT2D/File/MT2D_File.h>
#include <MT2D/ObjectCore/MT2D_Text_Image.h>
#else
#include "../MT2D_Terminal_Define.h"
#include "../ObjectCore/Sprites.h"
#include "../MT2D_Keyboard.h" // only used for taking the definition of the key enter
#include "../MT2D.h" // for geting the display definition
#include "../File/MT2D_File.h"

#endif

#if defined(SDL_USE) && !defined(__ANDROID__)
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/MT2D_SDL_main.h>
#include <MT2D/SDL/Render/MT2D_SDL_Render.h>
#include <MT2D/SDL/MT2D_SDL_Internal_Image.h>
#elif defined(SDL_USE) && defined(__ANDROID__)
#include "../SDL/MT2D_SDL_Redefine.h"
#include "../SDL/MT2D_SDL_main.h"
#include "../SDL/Render/MT2D_SDL_Render.h"
#include "../SDL/MT2D_SDL_Internal_Image.h"
#endif

#if defined(MSDOS_USE_GRAPHICS)
#include <MT2D/_MSDOS/Render/DosImg.h>
#include <MT2D/_MSDOS/Render/dVgaRen.h>
#include <MT2D/_MSDOS/Files/bmp.h>
#endif

#if defined(MT2D_USING_CONTAINER)
#include <MT2D/Container/MT2D_Container.h>
#endif
#include <MT2D/MT2D_Debug.h>


Sprite *Sprite_Create_Ascii_From_Cells(int SizeX, int SizeY, const char *cells) {
        Sprite *S = (Sprite*)malloc(sizeof(Sprite));
        if (!S) return 0;
        S->Text = MT2D_Text_Image_Create(SizeX, SizeY, cells);
        if (!S->Text) {
                free(S);
                return 0;
        }
        S->SizeX = SizeX;
        S->SizeY = SizeY;
        S->ScaleX = 1;
        S->ScaleY = 1;
        S->type = 0;
        S->refCount = 0;
        return S;
}

char Sprite_Get_Char(const Sprite *Me, int x, int y) {
        if (!Me || Me->type != 0) return 0;
        return MT2D_Text_Image_Get(Me->Text, x, y);
}

Sprite *Load_Sprite(char *file) {
        Sprite *S=0;
        char BUFF;
        char *cells;
        int Xi=0, Yi=0, X=0, Y=0;
        MT2D_FILE *fl = MT2D_FILE_OPEN(file, "rb");//read only
        if (fl) {
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
                //STEP 2: alloc and erase a temporary cell buffer
                cells = (char*)malloc((size_t)Xi * Yi);
                if (cells) {
                        memset(cells, 0, (size_t)Xi * Yi);
                        //STEP 3: load the file data over the cells
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
                                else if (X >= 0 && X < Xi && Y < Yi) {
                                        cells[(size_t)Y * Xi + X] = BUFF;
                                }
                                X++;
                        }
                        //STEP 4: compress it
                        S = Sprite_Create_Ascii_From_Cells(Xi, Yi, cells);
                        free(cells);
                }
                MT2D_FILE_CLOSE(fl);
        }//else return a null sprite
        return S;
}

#ifdef SDL_USE
static Sprite *Create_Image_Sprite(SDL_Surface *Img, int ScaleX, int ScaleY) {
        Sprite *S = (Sprite*)malloc(sizeof(Sprite));
        S->Image = (MT2D_Internal_Image*)malloc(sizeof(MT2D_Internal_Image));
        S->Image->Texture = MT2D_SDL_Create_Texture(Img);
        S->Image->RotatedTexture = MT2D_SDL_Create_Rotated_Texture(Img, S->Image->Texture);
        S->SizeX = Img->w;
        S->SizeY = Img->h;
        S->ScaleX = ScaleX;
        S->ScaleY = ScaleY;
        S->type = 1;
        S->refCount = 0;
        SDL_FreeSurface(Img);
        return S;
}
#elif defined(MSDOS_USE_GRAPHICS)
static Sprite *Create_Placeholder_Sprite(int ScaleX, int ScaleY);
/* VGA: loads an 8 bit BMP into a pixel buffer */
static Sprite *Load_Dos_Image_Sprite(char *file, int ScaleX, int ScaleY) {
        Sprite *S;
        FILE *fl = fopen(file, "rb");
        if (!fl) return 0;
        S = (Sprite*)malloc(sizeof(Sprite));
        if (!S) {
                fclose(fl);
                return 0;
        }
        S->Image = 0;
        if (!MT2D_Dos_Load_BMP(fl, S)) {
                fclose(fl);
                free(S);
                return Create_Placeholder_Sprite(ScaleX, ScaleY);
        }
        fclose(fl);
        S->ScaleX = ScaleX;
        S->ScaleY = ScaleY;
        S->refCount = 0;
        return S;
}
#endif
#ifndef SDL_USE
static Sprite *Create_Placeholder_Sprite(int ScaleX, int ScaleY) {
#pragma message ("Sprites with image are not supported so we'll return a blank image...")
        Sprite *S = Sprite_Create_Ascii_From_Cells(3, 1, "<!>");
        if (S) {
                S->ScaleX = ScaleX;
                S->ScaleY = ScaleY;
        }
        return S;
}
#endif

Sprite * Load_Sprite_Image(char * file, int ScaleX, int ScaleY)
{
#ifdef DEBUG_OBJECTCORE
        char buff[200];
        sprintf(buff, "Loading Sprite %s", file);
        MT2D_Ide_Printf(buff);
#endif
#ifdef SDL_USE
        Sprite *S = 0;
        MT2D_FILE *fl = MT2D_FILE_OPEN(file, "rb");//read only
        if (fl) {
                MT2D_FILE_CLOSE(fl);
                S = Create_Image_Sprite(MT2D_SDL_Load_Image(file), ScaleX, ScaleY);
        }//else return a null sprite
        return S;
#elif defined(MSDOS_USE_GRAPHICS)
        return Load_Dos_Image_Sprite(file, ScaleX, ScaleY);
#else
        return Create_Placeholder_Sprite(ScaleX, ScaleY);
#endif
 }

#if defined(MT2D_USING_CONTAINER)
Sprite *Load_Sprite_Image_From_Container(char *file, int ScaleX, int ScaleY) {
#ifdef SDL_USE
                Sprite *S = 0;
                int Id = MT2D_Container_Get_FileId(file);
                //!!! Always open non encrypted image
                if(Id >= 0){
                        S = Create_Image_Sprite(MT2D_SDL_Load_Image_From_Container(Id), ScaleX, ScaleY);
                }//else return a null sprite
                return S;
#else
                return Create_Placeholder_Sprite(ScaleX, ScaleY);
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
         if (img) {
                 if (img->type == 0) {//render as a sprite
                         out = true;
                         MT2D_Text_Image_Blit(img->Text, (unsigned char*)(witch_window == DISPLAY_WINDOW1 ? WINDOW1 : WINDOW2), MAX_HOR, MAX_VER, pos_x, pos_y);
                 }
                 else {
#ifdef SDL_USE
                         //lets optimize a bit...
                         if (pos_x <= 320 && pos_x + img->ScaleX >= 0) {
                                 SDL_Add_ImagetoBuffer(img, pos_x, pos_y);
                         }
#elif defined(MSDOS_USE_GRAPHICS)
                         if (img->Image && witch_window == DISPLAY_WINDOW1) {
                                 Dos_Add_ImagetoBuffer(img, pos_x, pos_y);
                                 out = true;
                         }
#endif
                 }
         }

        return out;
}

 void Sprite_Delete(Sprite *Me) {
         if (Me) {
                 if (Me->refCount > 0) {
                         Me->refCount--;
                 }
                 if (Me->refCount == 0) {
                         if (Me->type == 0) {
                                 MT2D_Text_Image_Free(Me->Text);
                         }
#ifdef SDL_USE
                         else if (Me->Image) {
                                 if (Me->Image->RotatedTexture) {
                                         MT2D_SDL_DestroyTexture(Me->Image->RotatedTexture);
                                 }
                                 if (Me->Image->Texture) {
                                         MT2D_SDL_DestroyTexture(Me->Image->Texture);
                                 }
                                 free(Me->Image);
                         }
#elif defined(MSDOS_USE_GRAPHICS)
                         else if (Me->Image) {
                                 free(Me->Image->Pixels);
                                 free(Me->Image);
                         }
#endif
                         free(Me);
                 }
         }
 }

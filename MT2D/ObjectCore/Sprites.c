/*Sprites.c , created by Lucas Zimerman Fraulob
 this file contains the basic functions related to sprites, it just loads and
 renders it.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __ANDROID__
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/MT2D_Keyboard.h> /* only used for taking the definition of the key enter */
#include <MT2D/MT2D.h> /* for geting the display definition */
#include <MT2D/File/MT2D_File.h>
#include <MT2D/ObjectCore/MT2D_Text_Image.h>
#else
#include "../MT2D_Terminal_Define.h"
#include "../ObjectCore/Sprites.h"
#include "../MT2D_Keyboard.h" /* only used for taking the definition of the key enter */
#include "../MT2D.h" /* for geting the display definition */
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

#if defined(MT2D_LINUX_SIXEL)
#include <MT2D/_LINUX/Render/MT2D_Linux_Image.h>
#include <MT2D/_LINUX/Render/MT2D_Linux_Render.h>
#endif

#if defined(MT2D_USING_CONTAINER)
#include <MT2D/Container/MT2D_Container.h>
#endif
#include <MT2D/MT2D_Debug.h>


Sprite *Sprite_Create_Ascii_From_Cells(int SizeX, int SizeY, const char *cells) {
        Sprite *S = (Sprite*)malloc(sizeof(Sprite));
        if (!S) return 0;
        S->Image = 0;
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

/* ------------------------------------------------------------------------------------------------
   Editing (MT2D_SpriteRaw, see SprRaw.h)
   ------------------------------------------------------------------------------------------------ */

#if defined(MSDOS_USE_GRAPHICS) || defined(MT2D_LINUX_SIXEL)
#define MT2D_SPRITE_INDEXED_IMAGE /* image sprites are Image->Pixels, 1 byte palette indexes */
#endif

#if defined(MSDOS_USE_GRAPHICS)
/* The VGA renderer only redraws the tiles whose sprites changed: every new or edited image gets a new Version */
static unsigned long Sprite_Image_Serial = 0;
#define SPRITE_IMAGE_CHANGED(img) ((img)->Version = ++Sprite_Image_Serial)
#else
#define SPRITE_IMAGE_CHANGED(img) ((void)0)
#endif

/* true when Raw has the format of the sprite kind (image or ASCII) of this build */
static bool Sprite_Raw_Format_Ok(const MT2D_SpriteRaw *Raw, bool image) {
        if (!Raw || !Raw->Data.Cells || Raw->Width <= 0 || Raw->Height <= 0) return false;
        if (!image) return !Raw->IsRgba;
#if defined(SDL_USE)
        return Raw->IsRgba;
#elif defined(MT2D_SPRITE_INDEXED_IMAGE)
        return !Raw->IsRgba;
#else
        return false; /* no image sprites on this build */
#endif
}

/* ASCII sprite -> Width*Height cells (malloc), NULL when there is no memory */
static unsigned char *Sprite_Expand_Text(const Sprite *Me) {
        unsigned char *cells = (unsigned char*)malloc((size_t)Me->SizeX * (size_t)Me->SizeY);
        if (cells) {
                memset(cells, 0, (size_t)Me->SizeX * (size_t)Me->SizeY);
                MT2D_Text_Image_Blit(Me->Text, cells, Me->SizeX, Me->SizeY, 0, 0);
        }
        return cells;
}

Sprite *MT2D_Sprite_Create_From_Raw(const MT2D_SpriteRaw *Raw, bool image, int ScaleX, int ScaleY) {
        Sprite *S;
        if (!Sprite_Raw_Format_Ok(Raw, image)) return 0;
        if (!image) {
                return Sprite_Create_Ascii_From_Cells(Raw->Width, Raw->Height, (const char*)Raw->Data.Cells);
        }
        S = (Sprite*)malloc(sizeof(Sprite));
        if (!S) return 0;
        S->Text = 0;
        S->Image = 0;
        S->SizeX = Raw->Width;
        S->SizeY = Raw->Height;
        S->ScaleX = ScaleX > 0 ? ScaleX : Raw->Width;
        S->ScaleY = ScaleY > 0 ? ScaleY : Raw->Height;
        S->type = 1;
        S->refCount = 0;
#if defined(SDL_USE)
        S->Image = MT2D_SDL_Image_Create_Rgba(Raw->Width, Raw->Height, (const unsigned char*)Raw->Data.Rgba);
#elif defined(MT2D_SPRITE_INDEXED_IMAGE)
        S->Image = (MT2D_Internal_Image*)malloc(sizeof(MT2D_Internal_Image));
        if (S->Image) {
                S->Image->Pixels = (unsigned char*)malloc((size_t)Raw->Width * (size_t)Raw->Height);
                if (S->Image->Pixels) {
                        memcpy(S->Image->Pixels, Raw->Data.Cells, (size_t)Raw->Width * (size_t)Raw->Height);
                        SPRITE_IMAGE_CHANGED(S->Image);
                }
                else {
                        free(S->Image);
                        S->Image = 0;
                }
        }
#endif
        if (!S->Image) {
                free(S);
                return 0;
        }
        return S;
}

int MT2D_Sprite_Alter_Block(Sprite *Me, int x, int y, const MT2D_SpriteRaw *Raw) {
        long x0, y0, x1, y1;
        int cw, ch, row;
        const unsigned char *src;
        size_t cellSize;
        if (!Me || !Sprite_Raw_Format_Ok(Raw, Me->type != 0)) return 0;
        if (Me->type != 0 && !Me->Image) return 0;
        if (Me->type == 0 && !Me->Text) return 0;
        /* the part of the sprite covered by Raw (longs: x + Width must not overflow) */
        x0 = x < 0 ? 0 : x;
        y0 = y < 0 ? 0 : y;
        x1 = (long)x + Raw->Width;
        y1 = (long)y + Raw->Height;
        if (x1 > Me->SizeX) x1 = Me->SizeX;
        if (y1 > Me->SizeY) y1 = Me->SizeY;
        if (x0 >= x1 || y0 >= y1) return 0;
        cw = (int)(x1 - x0);
        ch = (int)(y1 - y0);
        cellSize = Raw->IsRgba ? 4 : 1;
        /* first cell of Raw that lands on the sprite */
        src = Raw->Data.Cells + ((size_t)(y0 - y) * (size_t)Raw->Width + (size_t)(x0 - x)) * cellSize;

        if (Me->type == 0) {
                /* the text image is read only: expand, write, compress again */
                MT2D_Text_Image *text;
                unsigned char *cells = Sprite_Expand_Text(Me);
                if (!cells) return 0;
                for (row = 0; row < ch; row++) {
                        memcpy(cells + (size_t)(y0 + row) * (size_t)Me->SizeX + (size_t)x0,
                                src + (size_t)row * (size_t)Raw->Width, (size_t)cw);
                }
                text = MT2D_Text_Image_Create(Me->SizeX, Me->SizeY, (const char*)cells);
                free(cells);
                if (!text) return 0; /* too many opaque cells or no memory: the sprite is unchanged */
                MT2D_Text_Image_Free(Me->Text);
                Me->Text = text;
                return cw * ch;
        }
#if defined(SDL_USE)
        if (!MT2D_SDL_Image_Update_Rgba(Me->Image, (int)x0, (int)y0, cw, ch, src, Raw->Width * 4)) {
                return 0;
        }
        return cw * ch;
#elif defined(MT2D_SPRITE_INDEXED_IMAGE)
        for (row = 0; row < ch; row++) {
                memcpy(Me->Image->Pixels + (size_t)(y0 + row) * (size_t)Me->SizeX + (size_t)x0,
                        src + (size_t)row * (size_t)Raw->Width, (size_t)cw);
        }
        SPRITE_IMAGE_CHANGED(Me->Image);
        return cw * ch;
#else
        return 0;
#endif
}

MT2D_SpriteRaw *MT2D_Sprite_To_SpriteRaw(const Sprite *Me) {
        MT2D_SpriteRaw *raw;
        if (!Me || Me->SizeX <= 0 || Me->SizeY <= 0) return 0;
        if (Me->type == 0) {
                if (!Me->Text) return 0;
                raw = MT2D_SpriteRaw_Create(Me->SizeX, Me->SizeY, false);
                if (raw) MT2D_Text_Image_Blit(Me->Text, raw->Data.Cells, Me->SizeX, Me->SizeY, 0, 0);
                return raw;
        }
        if (!Me->Image) return 0;
#if defined(SDL_USE)
        raw = MT2D_SpriteRaw_Create(Me->SizeX, Me->SizeY, true);
        if (raw && !MT2D_SDL_Image_Read_Rgba(Me->Image, Me->SizeX, Me->SizeY, (unsigned char*)raw->Data.Rgba)) {
                MT2D_SpriteRaw_Free(raw);
                raw = 0;
        }
        return raw;
#elif defined(MT2D_SPRITE_INDEXED_IMAGE)
        raw = MT2D_SpriteRaw_Create(Me->SizeX, Me->SizeY, false);
        if (raw) memcpy(raw->Data.Cells, Me->Image->Pixels, (size_t)Me->SizeX * (size_t)Me->SizeY);
        return raw;
#else
        return 0;
#endif
}

Sprite *Load_Sprite(char *file) {
        Sprite *S=0;
        char BUFF;
        char *cells;
        int Xi=0, Yi=0, X=0, Y=0;
        MT2D_FILE *fl = MT2D_FILE_OPEN(file, "rb"); /* read only */
        if (fl) {
                /* STEP 1: count the size of the line and column */
                while (!MT2D_FILE_EOF(fl)) {
                        BUFF = MT2D_FILE_READ_BYTE(fl);
                        if (BUFF == enter_pressed) {
                                MT2D_FILE_READ_BYTE(fl); /* why there's always a '\r' after the \n ? */
                                if (Xi == 0) {
                                        Xi = X;
                                }
                                else if (Xi < X) {
                                        Xi = X; /* no more fixed horizontal size */
                                }
                                X = 0; /* only zeroed to avoid overflow... */
                                Y++;
                        }
                        X++;
                }
                Yi = Y+1 ;
                Xi = Xi +1;
                /* STEP 2: alloc and erase a temporary cell buffer */
                cells = (char*)malloc((size_t)Xi * Yi);
                if (cells) {
                        memset(cells, 0, (size_t)Xi * Yi);
                        /* STEP 3: load the file data over the cells */
                        MT2D_FILE_SEEK(fl,0L,SEEK_SET); /* move the pointer into the top of the file */
                        X = 0;
                        Y = 0;
                        while (!MT2D_FILE_EOF(fl)) {
                                BUFF = MT2D_FILE_READ_BYTE(fl);
                                if (BUFF == enter_pressed)
                                {
                                        MT2D_FILE_READ_BYTE(fl); /* why there's always a '\r' after the \n ? */
                                        X = -1; /* only zeroed to avoid overflow... */
                                        Y++;
                                }
                                else if (X >= 0 && X < Xi && Y < Yi) {
                                        cells[(size_t)Y * Xi + X] = BUFF;
                                }
                                X++;
                        }
                        /* STEP 4: compress it */
                        S = Sprite_Create_Ascii_From_Cells(Xi, Yi, cells);
                        free(cells);
                }
                MT2D_FILE_CLOSE(fl);
        } /* else return a null sprite */
        return S;
}

#ifdef SDL_USE
static Sprite *Create_Image_Sprite(SDL_Surface *Img, int ScaleX, int ScaleY) {
        Sprite *S = (Sprite*)malloc(sizeof(Sprite));
        S->Text = 0;
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

/*
        The images of the examples are PNGs, MS-DOS can't read them: "name.png" is loaded from the
        converted 8 bit BMP "d/<name cut to 8 chars>.bmp" (see examples/tools/png2dos.py).
        Names that already end with ".bmp" are used as they are.
*/
static const char *Dos_Image_Path(const char *file, char *out, size_t outSize) {
        const char *base = file, *p, *dot;
        size_t n, i;
        for (p = file; *p; p++) {
                if (*p == '/' || *p == '\\') base = p + 1;
        }
        dot = strrchr(base, '.');
        if (!dot || strcmp(dot, ".png") != 0 || outSize < 16) return file;
        n = (size_t)(dot - base);
        if (n > 8) n = 8;
        strcpy(out, "d/");
        for (i = 0; i < n; i++) {
                char c = base[i];
                out[2 + i] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
        }
        strcpy(out + 2 + n, ".bmp");
        return out;
}

static Sprite *Load_Dos_Image_Sprite(char *file, int ScaleX, int ScaleY) {
        Sprite *S;
        char path[32];
        FILE *fl = fopen(Dos_Image_Path(file, path, sizeof(path)), "rb");
        if (!fl) return 0;
        S = (Sprite*)malloc(sizeof(Sprite));
        if (!S) {
                fclose(fl);
                return 0;
        }
        S->Text = 0;
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
#ifdef SDL_USE
        Sprite *S = 0;
        MT2D_FILE *fl;
#endif
#ifdef DEBUG_OBJECTCORE
        char buff[200];
        sprintf(buff, "Loading Sprite %s", file);
        MT2D_Ide_Printf(buff);
#endif
#ifdef SDL_USE
        fl = MT2D_FILE_OPEN(file, "rb"); /* read only */
        if (fl) {
                MT2D_FILE_CLOSE(fl);
                S = Create_Image_Sprite(MT2D_SDL_Load_Image(file), ScaleX, ScaleY);
        } /* else return a null sprite */
        return S;
#elif defined(MSDOS_USE_GRAPHICS)
        return Load_Dos_Image_Sprite(file, ScaleX, ScaleY);
#elif defined(MT2D_LINUX_SIXEL)
        return MT2D_Linux_Load_Image_Sprite(file, ScaleX, ScaleY); /* _LINUX/Render/MT2D_Linux_Image.c */
#else
        return Create_Placeholder_Sprite(ScaleX, ScaleY);
#endif
 }

#if defined(MT2D_USING_CONTAINER)
Sprite *Load_Sprite_Image_From_Container(char *file, int ScaleX, int ScaleY) {
#ifdef SDL_USE
                Sprite *S = 0;
                int Id = MT2D_Container_Get_FileId(file);
                /* !!! Always open non encrypted image */
                if(Id >= 0){
                        S = Create_Image_Sprite(MT2D_SDL_Load_Image_From_Container(Id), ScaleX, ScaleY);
                } /* else return a null sprite */
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
 bool Sprite_Render_on_Window(Sprite *img, unsigned char (*window)[MAX_HOR], int pos_x, int pos_y) { /* output: out = false : invalid sprite | out = true : valid sprite */
        /* to be implemented: Sprites Scale */
         bool out = false;
         bool main_screen = !window || window == WINDOW1; /* images only go on the main screen */
         if (img) {
                 if (img->type == 0) { /* render as a sprite */
                         out = true;
                         MT2D_Text_Image_Blit(img->Text, (unsigned char*)MT2D_WINDOW_OR_MAIN(window), MAX_HOR, MAX_VER, pos_x, pos_y);
                 }
                 else {
#ifdef SDL_USE
                         /* lets optimize a bit... */
                         if (main_screen && pos_x <= 320 && pos_x + img->ScaleX >= 0) {
                                 SDL_Add_ImagetoBuffer(img, pos_x, pos_y);
                         }
#elif defined(MSDOS_USE_GRAPHICS)
                         if (img->Image && main_screen) {
                                 Dos_Add_ImagetoBuffer(img, pos_x, pos_y);
                                 out = true;
                         }
#elif defined(MT2D_LINUX_SIXEL)
                         if (img->Image && main_screen) {
                                 MT2D_Linux_Sixel_Queue_Image(img, pos_x, pos_y);
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
#elif defined(MT2D_LINUX_SIXEL)
                         else if (Me->Image) {
                                 MT2D_Linux_Image_Free(Me->Image);
                         }
#endif
                         free(Me);
                 }
         }
 }

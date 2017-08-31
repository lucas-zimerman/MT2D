#ifndef _MT2D_SDL_RED_H
#define _MT2D_SDL_RED_H

#include "../MT2D_Terminal_Define.h"

#ifdef SDL_USE // if you'll not use SDL dont load this file on your project
#include <stdio.h>
#include "../MT2D.h"

#include "../MT2D_Keyboard.h"
#include "../MT2D_System_Calls.h"
#include "MT2D_SDL_Defines.h"


#ifdef _WIN32
    #ifdef SDL_stbimage

    #define SDL_STBIMAGE_IMPLEMENTATION
    #include "SDL_stbimage.h"
    #elif defined(MT2D_SDL_GPU)
    #define SDL_GPU_DISABLE_OPENGL 1
    #define SDL_GPU_DISABLE_GLES_1 1
    #define SDL_GPU_DISABLE_GLES_3 1

    #include "SDL.h"
    #include "SDL_gpu.h"
    #else
	#include <SDL.h>
    #include <SDL_image.h>
    #endif
#else
    #if defined(__ANDROID__)
    #include <SDL.h>
    #elif defined linux
    #include <SDL2/SDL.h>
    #endif

    #ifdef SDL_stbimage
    #include "SDL_stbimage.h"
    #else
        #if defined(__ANDROID__)
        #include <SDL_image.h>
        #elif defined(linux)
        #include <SDL2/SDL_image.h>
        #endif
    #endif
    #include "MT2D_SDL_Defines.h"
#endif

#pragma region TYPES
#ifdef MT2D_SDL_GPU
#define IMG_INIT_PNG true
//============
#define MT2D_SDL_Texture GPU_Image
#define MT2D_SDL_Rect GPU_Rect
#define MT2D_SDL_Renderer GPU_Target
#define MT2D_SDL_Window int
#else
#define MT2D_SDL_Texture SDL_Texture
#define MT2D_SDL_Rect SDL_Rect
#define MT2D_SDL_Renderer SDL_Renderer
#define MT2D_SDL_Window SDL_Window
#endif
#pragma endregion

#ifdef MT2D_SDL_GPU
#define IMG_Load GPU_LoadSurface
#endif

SDL_Surface *MT2D_SDL_Load_Image(char *Addr);


#ifdef MT2D_SDL_GPU
MT2D_SDL_Texture *MT2D_SDL_CreateTextureFromSurface(MT2D_SDL_Renderer *render, SDL_Surface *surface) {
	return GPU_CopyImageFromSurface(surface);
}
#else
MT2D_SDL_Texture *MT2D_SDL_CreateTextureFromSurface(SDL_Renderer *render, SDL_Surface *surface);

#endif


void MT2D_SDL_DestroyTexture(MT2D_SDL_Texture *Tex);

int MT2D_SDL_RenderCopyEx(MT2D_SDL_Renderer * renderer, MT2D_SDL_Texture * texture,
	MT2D_SDL_Rect * srcrect, const MT2D_SDL_Rect * dstrect,
	const double angle, const SDL_Point *center, const SDL_RendererFlip flip);

int MT2D_SDL_RenderCopy(MT2D_SDL_Renderer * renderer, MT2D_SDL_Texture * texture,
	MT2D_SDL_Rect * srcrect, const MT2D_SDL_Rect * dstrect);

int MT2D_SDL_BlitSurface(SDL_Surface * src, const MT2D_SDL_Rect * srcrect,
	SDL_Surface * dst, MT2D_SDL_Rect * dstrect);

MT2D_SDL_Renderer *MT2D_SDL_CreateRenderer(MT2D_SDL_Window * window, int index, Uint32 flags);

MT2D_SDL_Window *MT2D_SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);


void MT2D_SDL_SetWindowIcon(MT2D_SDL_Window * window, SDL_Surface * icon);

void MT2D_SDL_SetRenderDrawColor(MT2D_SDL_Renderer * renderer,
	Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#ifdef MT2D_SDL_GPU
int IMG_Init(int i);

void SDL_DestroyWindow(MT2D_SDL_Window *window) {}

void IMG_Quit();
#endif

void MT2D_SDL_DestroyRenderer(MT2D_SDL_Renderer *renderer);

void MT2D_SDL_RenderPresent(MT2D_SDL_Renderer *renderer);

void MT2D_SDL_RenderClear(MT2D_SDL_Renderer *renderer);

void MT2D_SDL_SetWindowFullscreen(MT2D_SDL_Window * window, Uint32 flags);

void MT2D_SDL_SetWindowSize(MT2D_SDL_Window * window, int w, int h);

int MT2D_SDL_SetRenderTarget(MT2D_SDL_Renderer *renderer, MT2D_SDL_Texture *texture);

#endif
#endif

#ifndef _MT2D_SDL_RED_H
#define _MT2D_SDL_RED_H

#include <MT2D/MT2D_Terminal_Define.h>

#ifdef SDL_USE // if you'll not use SDL dont load this file on your project
	#include <MT2D/MT2D_Keyboard.h>
	#include <MT2D/MT2D_System_Calls.h>
	#include <MT2D/MT2D.h>
    #include "MT2D_SDL_Defines.h"
	#include <stdio.h>


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
		#include <SDL_image.h>
	#endif
#else
	#include <SDL2/SDL.h>
	#ifdef SDL_stbimage
		#include "SDL_stbimage.h"
	#else
		#include <SDL2/SDL_image.h>
	#endif
	#include "SDL_Defines.h"
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

SDL_Surface *MT2D_SDL_Load_Image(char *Addr) {
	#ifndef MT2D_SDL_GPU
		return IMG_Load(Addr);
	#else
		return GPU_LoadSurface(Addr);
	#endif
}


#ifdef MT2D_SDL_GPU
MT2D_SDL_Texture *MT2D_SDL_CreateTextureFromSurface(MT2D_SDL_Renderer *render, SDL_Surface *surface) {
	return GPU_CopyImageFromSurface(surface);
}
#else
MT2D_SDL_Texture *MT2D_SDL_CreateTextureFromSurface(SDL_Renderer *render, SDL_Surface *surface) {
	return SDL_CreateTextureFromSurface(render,surface);
}

#endif


void MT2D_SDL_DestroyTexture(MT2D_SDL_Texture *Tex) {
#ifdef MT2D_SDL_GPU
		GPU_FreeImage(Tex);
#else
	SDL_DestroyTexture(Tex);
#endif
}

int MT2D_SDL_RenderCopyEx(MT2D_SDL_Renderer * renderer, MT2D_SDL_Texture * texture,
	MT2D_SDL_Rect * srcrect, const MT2D_SDL_Rect * dstrect,
	const double angle, const SDL_Point *center, const SDL_RendererFlip flip) {
#ifdef MT2D_SDL_GPU
	GPU_BlitTransform(texture, srcrect, renderer, dstrect->x, dstrect->y, angle, dstrect->h, dstrect->w);
	return 0;
#else
	return SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip);
#endif
}

int MT2D_SDL_RenderCopy(MT2D_SDL_Renderer * renderer, MT2D_SDL_Texture * texture,
	 MT2D_SDL_Rect * srcrect, const MT2D_SDL_Rect * dstrect) {
#ifdef MT2D_SDL_GPU
	GPU_BlitScale(texture, srcrect, renderer, dstrect->x, dstrect->y, dstrect->h, dstrect->w);
	return 0;
#else
	return SDL_RenderCopy(renderer, texture, srcrect, dstrect);
#endif
}

int MT2D_SDL_BlitSurface(SDL_Surface * src, const MT2D_SDL_Rect * srcrect,
	SDL_Surface * dst, MT2D_SDL_Rect * dstrect) {
#ifdef MT2D_SDL_GPU
	SDL_Rect A, B;
	A.h = srcrect->h;
	A.w = srcrect->w;
	A.x = srcrect->x;
	A.y = srcrect->y;
	B.h = dstrect->h;
	B.w = dstrect->w;
	B.x = dstrect->x;
	B.y = dstrect->y;
	return SDL_BlitSurface(src, &A, dst, &B);
#else
	return SDL_BlitSurface(src, srcrect, dst, dstrect);
#endif
}

MT2D_SDL_Renderer *MT2D_SDL_CreateRenderer(MT2D_SDL_Window * window, int index, Uint32 flags) {
#ifdef MT2D_SDL_GPU
	return GPU_Init(SCREEN_WIDTH, SCREEN_HEIGHT, flags);
#else
	return SDL_CreateRenderer(window,index,flags);
#endif
}
MT2D_SDL_Window *MT2D_SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags) {
#ifdef MT2D_SDL_GPU
	SCREEN_WIDTH = w;
	SCREEN_HEIGHT = h;
	MT2D_SDL_Window *X = (MT2D_SDL_Window*)malloc(sizeof(MT2D_SDL_Window));
	X[0] = 1;
	return X;
#else
	
	return SDL_CreateWindow(title,x,y,w,h,flags);
#endif
}


void MT2D_SDL_SetWindowIcon(MT2D_SDL_Window * window, SDL_Surface * icon){
#ifdef MT2D_SDL_GPU
	//not implemented
#else
	SDL_SetWindowIcon(window, icon);
#endif
}

void MT2D_SDL_SetRenderDrawColor(MT2D_SDL_Renderer * renderer,
	Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
#ifdef MT2D_SDL_GPU
	GPU_ClearRGBA(renderer, r, g, b, a);
#else
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

#endif
}

#ifdef MT2D_SDL_GPU
int IMG_Init(int i) {
	return true;
}

void SDL_DestroyWindow(MT2D_SDL_Window *window){}

void IMG_Quit(){
	GPU_Quit();
}
#endif

void MT2D_SDL_DestroyRenderer(MT2D_SDL_Renderer *renderer) {
#ifdef MT2D_SDL_GPU
	GPU_FreeTarget(renderer);
#else
	SDL_DestroyRenderer(renderer);
#endif
}

void MT2D_SDL_RenderPresent(MT2D_SDL_Renderer *renderer) {
#ifdef MT2D_SDL_GPU
	GPU_Flip(renderer);
#else
	SDL_RenderPresent(renderer);
#endif
}

void MT2D_SDL_RenderClear(MT2D_SDL_Renderer *renderer) {
#ifdef MT2D_SDL_GPU
	GPU_Clear(renderer);
#else
	SDL_RenderClear(renderer);
#endif
}

void MT2D_SDL_SetWindowFullscreen(MT2D_SDL_Window * window, Uint32 flags){
#ifdef MT2D_SDL_GPU
	if (flags) {
		GPU_SetFullscreen(true, flags);
	}
	else {
		GPU_SetFullscreen(false, flags);
	}

#else
	SDL_SetWindowFullscreen(window, flags);
#endif
}

void MT2D_SDL_SetWindowSize(MT2D_SDL_Window * window, int w, int h) {
#ifdef MT2D_SDL_GPU
	GPU_SetWindowResolution(w, h);
#else
	SDL_SetWindowSize(window, w, h);

#endif
}

#endif

#endif

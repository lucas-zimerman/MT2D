
#include <MT2D/MT2D_Terminal_Define.h>

#ifdef SDL_USE // if you'll not use SDL dont load this file on your project
#include <MT2D/SDL/MT2D_SDL_Redefine.h>



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
	return SDL_CreateTextureFromSurface(render, surface);
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
	return SDL_CreateRenderer(window, index, flags);
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

	return SDL_CreateWindow(title, x, y, w, h, flags);
#endif
}


void MT2D_SDL_SetWindowIcon(MT2D_SDL_Window * window, SDL_Surface * icon) {
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

void SDL_DestroyWindow(MT2D_SDL_Window *window) {}

void IMG_Quit() {
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

void MT2D_SDL_SetWindowFullscreen(MT2D_SDL_Window * window, Uint32 flags) {
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

int MT2D_SDL_SetRenderTarget(MT2D_SDL_Renderer *renderer, MT2D_SDL_Texture *texture) {
#ifdef MT2D_SDL_GPU
	renderer = GPU_LoadTarget(texture);
	return 1;
#else
	return SDL_SetRenderTarget(renderer, texture);
#endif

}

#endif
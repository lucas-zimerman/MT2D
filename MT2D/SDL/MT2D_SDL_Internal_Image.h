#ifndef MT2D_SDL_INTERNAL_IMAGE_H
#define MT2D_SDL_INTERNAL_IMAGE_H
#if defined(SDL_USE)
#include <MT2D/ObjectCore/MT2D_Internal_Image.h>
#include <MT2D/SDL/MT2D_SDL_Redefine.h>

/* SDL implementation of the sprite image */
struct MT2D_Internal_Image {
        MT2D_SDL_Texture *Texture;
        MT2D_SDL_Texture *RotatedTexture; /* 90 degrees copy, used when the window is in portrait mode */
};
#endif
#endif

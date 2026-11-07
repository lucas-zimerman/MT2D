#ifndef MT2D_INTERNAL_IMAGE_H
#define MT2D_INTERNAL_IMAGE_H
/*
        Opaque handle for the platform specific image of a Sprite (see MT2D_SDL_Internal_Image.h).
        Platforms without a graphical image type never create one, the pointer is always NULL.
*/
struct MT2D_Internal_Image;
typedef struct MT2D_Internal_Image MT2D_Internal_Image;
#endif

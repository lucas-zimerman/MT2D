#ifndef MT2D_DOS_IMAGE_H
#define MT2D_DOS_IMAGE_H
#include <MT2D/MT2D_Terminal_Define.h>
#if defined(MSDOS_USE_GRAPHICS)
#include <MT2D/ObjectCore/MT2D_Internal_Image.h>

/* VGA (mode 13h) implementation of the sprite image: 8 bit palette indexed pixels,
   row major, top row first, SizeX*SizeY bytes (size is stored on the Sprite). */
struct MT2D_Internal_Image {
	unsigned char *Pixels;
};
#endif
#endif

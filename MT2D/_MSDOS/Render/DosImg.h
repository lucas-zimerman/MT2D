#ifndef MT2D_DOS_IMAGE_H
#define MT2D_DOS_IMAGE_H
#include <MT2D/MT2D_Terminal_Define.h>
#if defined(MSDOS_USE_GRAPHICS)
#include <MT2D/ObjectCore/MT2D_Internal_Image.h>

/* VGA (mode 13h) implementation of the sprite image: 8 bit palette indexed pixels,
   row major, top row first, SizeX*SizeY bytes (size is stored on the Sprite). */
struct MT2D_Internal_Image {
	unsigned char *Pixels;
	/* changes when the pixels change (MT2D_Sprite_Create_From_Raw, MT2D_Sprite_Alter_Block), 0 for loaded BMPs.
	   It is part of the tile signature of the VGA renderer: without it an edited sprite, or a new one that got
	   the same Pixels address from malloc, would not be redrawn. */
	unsigned long Version;
};
#endif
#endif

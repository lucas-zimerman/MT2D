#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Linux_Image.h / MT2D_Linux_Image.c

Image sprites of the terminal graphics mode (MT2D_TERMINAL_USE_GRAPHICS, Sixel).

The images are 8 bit palette BMP files, the same ones the MS-DOS VGA mode uses:
index 0 is the transparent color. "name.png" is loaded from the converted
"d/<name cut to 8 chars>.bmp" (see examples/tools/png2dos.py), a name that
ends with ".bmp" is used as it is.
===================================================================*/
#ifndef MT2DLIN_IMAGE_H
#define MT2DLIN_IMAGE_H

#include <stddef.h>
#include <stdio.h>
#include <MT2D/ObjectCore/Sprites.h>

struct MT2D_Internal_Image {
	unsigned char *Pixels;        /* SizeX * SizeY palette indexes (size stored on the Sprite), top row first */
	int Colors;                   /* used entries of Palette */
	unsigned char Palette[256][3]; /* RGB 0..255 */
};

MT2D_C_BEGIN
/* Path of the BMP of an image file name (see above), written into out when it is changed. */
const char *MT2D_Linux_Image_Path(const char *file, char *out, size_t outSize);

/* Reads an uncompressed 8 bit BMP into S (Image, SizeX, SizeY, type = 1). Returns 0 on any problem, S is left untouched. */
int MT2D_Linux_Load_BMP(FILE *file, Sprite *S);

void MT2D_Linux_Image_Free(MT2D_Internal_Image *image);
MT2D_C_END

#endif

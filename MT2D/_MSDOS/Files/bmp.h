#if defined(MSDOS_USE_GRAPHICS) && !defined(BMP_H)
#define BMP_H
#include <stdio.h>
#include <MT2D/ObjectCore/Sprites.h>

MT2D_C_BEGIN

void MT2D_Dos_Render_BMP(Sprite *bmp,int x,int y);
/* Fills S (SizeX, SizeY, type = 1, Image->Pixels) from an 8 bit BMP. Does not close the file.
   Returns 1 on success, 0 otherwise. */
int MT2D_Dos_Load_BMP(FILE *file,Sprite *S);
 void MT2D_Dos_Render_Transparent_BMP(Sprite *bmp,int x,int y);
 void MT2D_Dos_Render_BMP_Mixed(Sprite *bmp,int x,int y);
MT2D_C_END
#endif

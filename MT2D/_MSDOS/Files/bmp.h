#if defined(MSDOS_USE_GRAPHICS) && !defined(BMP_H)
#define BMP_H
#include <MT2D/ObjectCore/Sprites.h>

void MT2D_Dos_Render_BMP(Sprite *bmp,int x,int y);
void MT2D_Dos_Load_BMP(FILE *file,Sprite *S);
 void MT2D_Dos_Render_Transparent_BMP(Sprite *bmp,int x,int y);
 void MT2D_Dos_Render_BMP_Mixed(Sprite *bmp,int x,int y);
#endif

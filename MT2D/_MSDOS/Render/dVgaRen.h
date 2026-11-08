#ifdef MSDOS_USE_GRAPHICS
#include <MT2D/ObjectCore/Sprites.h>
void Dos_Add_ImagetoBuffer(Sprite *IMG,int X, int Y);
void MT2D_Dos_Render_Sprites();
void Dos_Clear_ImgBuffer();
/*Programs the VGA colors. bgra: count entries of 4 bytes (B,G,R,unused) with 8 bit values.*/
void MT2D_Dos_Set_Palette(const unsigned char *bgra, int count);
void MT2D_Dos_Render_VGA_ASCII(unsigned char BUFFER[][80]);
#endif
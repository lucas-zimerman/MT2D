#ifdef MSDOS_USE_GRAPHICS
#include <MT2D/ObjectCore/Sprites.h>
void Dos_Add_ImagetoBuffer(Sprite *IMG,int X, int Y);
void MT2D_Dos_Render_Sprites();
void Dos_Clear_ImgBuffer();
void MT2D_Dos_Render_VGA_ASCII(unsigned char BUFFER[][80]);
#endif
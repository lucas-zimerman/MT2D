#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
#include <MT2D/ObjectCore/Sprites.h>
MT2D_C_BEGIN
void Dos_Add_ImagetoBuffer(Sprite *IMG,int X, int Y);
void Dos_Clear_ImgBuffer(void);
/*Programs the VGA colors. bgra: count entries of 4 bytes (B,G,R,unused) with 8 bit values.*/
void MT2D_Dos_Set_Palette(const unsigned char *bgra, int count);
void MT2D_Dos_Render_VGA_ASCII(unsigned char BUFFER[][80]);
/*Makes the next render draw the whole screen (e.g. after the video mode was set again).*/
void MT2D_Dos_Render_Invalidate(void);
/*Frees the memory kept between frames, MT2D_Dos_VGA_Close calls it.*/
void MT2D_Dos_Render_Free(void);
MT2D_C_END
#endif

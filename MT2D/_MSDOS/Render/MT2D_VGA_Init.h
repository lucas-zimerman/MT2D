#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
bool MT2D_Dos_VGA_Init();
void MT2D_Dos_VGA_Close();
void MT2D_Dos_Render_Pixel(unsigned int X, unsigned int Y, unsigned char Color);
#endif

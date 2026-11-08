#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
MT2D_C_BEGIN
bool MT2D_Dos_VGA_Init(void);
void MT2D_Dos_VGA_Close(void);
void MT2D_Dos_Render_Pixel(unsigned int X, unsigned int Y, unsigned char Color);
MT2D_C_END
#endif

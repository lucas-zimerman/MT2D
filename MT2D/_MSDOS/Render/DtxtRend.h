#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/MT2D_Window.h>
#include <MT2D/MT2D_Terminal_Define.h>
#if defined(__MSDOS__) || defined(__DOS__)
MT2D_C_BEGIN
void MT2D_Dos_Draw_Window(unsigned char (*window)[MAX_HOR]); /* shows window, WINDOW1 when NULL */
void MT2D_Dos_Clear_Main_Window(void);
MT2D_C_END
#endif

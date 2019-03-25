#ifndef __MSDOS__
#include "MT2D_Terminal_Define.h"
#else
#include <MT2D/MT2D_Terminal_Define.h>
#endif
#if defined(__MSDOS__)
void MT2D_Dos_Draw_Window(int which);
void MT2D_Dos_Clear_Main_Window();
#endif
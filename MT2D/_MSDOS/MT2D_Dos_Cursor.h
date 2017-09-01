#ifndef __MSDOS__
#include "MT2D_Terminal_Define.h"
#else
#include "../../MT2D/MT2D_Terminal_Define.h"
#endif
#if defined(__MSDOS__)
void MT2D_Dos_Cursor_goto_topscreen();
void MT2D_Dos_Cursor_Set_Position(int line, int column);
#endif
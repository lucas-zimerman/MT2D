#ifndef __ANDROID__
#include "MT2D_Terminal_Define.h"
#else
#include "../../MT2D/MT2D_Terminal_Define.h"
#endif
#if defined(__ANDROID__)
void MT2D_Droid_Cursor_goto_topscreen();
void MT2D_Droid_Cursor_Set_Position(int line, int column);
#endif
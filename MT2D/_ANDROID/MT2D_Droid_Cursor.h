#include <MT2D/ObjectCore/MT2D_CDecl.h>
#ifndef __ANDROID__
#include "MT2D_Terminal_Define.h"
#else
#include <MT2D/MT2D_Terminal_Define.h>
#endif
#if defined(__ANDROID__)
MT2D_C_BEGIN
void MT2D_Droid_Cursor_goto_topscreen(void);
void MT2D_Droid_Cursor_Set_Position(int line, int column);
MT2D_C_END
#endif
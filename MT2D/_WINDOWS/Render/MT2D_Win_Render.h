#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../../MT2D_Terminal_Define.h"
#if defined(_WIN32) && !defined(SDL_USE)
MT2D_C_BEGIN
void MT2D_Win_Draw_Window(int which);
void MT2D_Win_Clear_Main_Window(void);
MT2D_C_END
#endif
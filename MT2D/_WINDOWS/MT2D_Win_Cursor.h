#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../MT2D_Terminal_Define.h"
#if defined(_WIN32) && !defined(SDL_USE)
MT2D_C_BEGIN
void MT2D_Win_Cursor_goto_topscreen(void);
void MT2D_Win_Cursor_Set_Position(int line, int column);
MT2D_C_END
#endif
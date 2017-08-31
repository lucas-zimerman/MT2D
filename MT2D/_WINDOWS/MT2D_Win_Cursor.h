#include "../MT2D_Terminal_Define.h"
#if defined(_WIN32) && !defined(SDL_USE)
void MT2D_Win_Cursor_goto_topscreen();
void MT2D_Win_Cursor_Set_Position(int line, int column);
#endif
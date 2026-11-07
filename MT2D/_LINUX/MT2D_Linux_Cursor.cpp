// the "linux" macro (also used for macOS) is defined by this header, include it before the #if below
#include "../MT2D_Terminal_Define.h"
#include "../MT2D_Terminal_Define.h"

#if defined(linux)

void MT2D_Linux_Cursor_goto_topscreen() {
}

void MT2D_Linux_Cursor_Set_Position(int line, int column) {
}
#endif
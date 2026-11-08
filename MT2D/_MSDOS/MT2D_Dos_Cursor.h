#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/MT2D_Terminal_Define.h>
#if defined(__MSDOS__) || defined(__DOS__)
MT2D_C_BEGIN
void MT2D_Dos_Cursor_goto_topscreen(void);
void MT2D_Dos_Cursor_Set_Position(int line, int column);
MT2D_C_END
#endif

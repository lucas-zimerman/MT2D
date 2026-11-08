#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../../MT2D_Terminal_Define.h"
#if defined(linux) && !defined(__ANDROID__)
MT2D_C_BEGIN
void MT2D_Linux_Draw_Window(int which);
void MT2D_Linux_Clear_Main_Window(void);
MT2D_C_END
#endif
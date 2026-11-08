#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../MT2D_Terminal_Define.h"
#if defined(MT2D_DEBUG_ENABLED) && defined(WINDOWS_TARGET) && !defined(SDL_USE)
MT2D_C_BEGIN
void MT2D_Win_Ide_Printf(char *String);
MT2D_C_END
#else 
#define MT2D_Win_Ide_Printf(text)
#endif

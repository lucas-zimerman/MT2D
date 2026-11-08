#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../MT2D_Terminal_Define.h"
#if defined(SDL_USE) && !defined(SDL_DEBUG_H)
#define SDL_DEBUG_H
MT2D_C_BEGIN
void MT2D_SDL_Ide_Printf(const char *String);
MT2D_C_END
#endif
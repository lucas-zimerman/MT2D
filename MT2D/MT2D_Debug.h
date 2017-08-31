#include"MT2D_Terminal_Define.h"
#if defined(SDL_USE)
#include <MT2D/SDL/MT2D_SDL_Debug.h>
#elif defined(WINDOWS_TARGET)
#define MT2D_Ide_Printf MT2D_Win_Ide_Printf
#else
#error "code not developed for your IDE/OS"
#endif
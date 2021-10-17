#include <MT2D/MT2D_Terminal_Define.h>
#if defined(SDL_USE)
#include <MT2D/SDL/MT2D_SDL_Debug.h>
#define MT2D_Ide_Printf MT2D_SDL_Ide_Printf
#elif defined(WINDOWS_TARGET)
#include <MT2D/_WINDOWS/MT2D_Win_Debug.h>
#define MT2D_Ide_Printf MT2D_Win_Ide_Printf
#elif !defined(MT2D_DEBUG_ENABLED)
#define MT2D_Ide_Printf
#else
#error "code not developed for your IDE/OS"
#endif

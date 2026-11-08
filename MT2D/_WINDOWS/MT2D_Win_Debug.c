#include "../MT2D_Terminal_Define.h"
#if defined(MT2D_DEBUG_ENABLED) && defined(WINDOWS_TARGET) && !defined(SDL_USE)
#include <Windows.h>

void MT2D_Win_Ide_Printf(char *String) {
	OutputDebugStringA(String);
}

#endif
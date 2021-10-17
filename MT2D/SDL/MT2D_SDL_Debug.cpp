#include "../MT2D_Terminal_Define.h"
#if defined(MT2D_DEBUG_ENABLED) && defined(SDL_USE)
#include <SDL.h>
void MT2D_SDL_Ide_Printf(const char *String) {
	SDL_Log("%s\n", String);
}
#else
void MT2D_SDL_Ide_Printf(const char *String) {}
#endif
#include "../MT2D_Terminal_Define.h"
#if defined(_DEBUG) && defined(SDL_USE)
#include <SDL.h>
void MT2D_Ide_Printf(char *String) {
	SDL_Log("%s\n", String);
}
#else
void MT2D_Ide_Printf(char *String) {}
#endif
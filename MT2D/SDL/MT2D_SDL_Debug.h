#include "../MT2D_Terminal_Define.h"
#if defined(SDL_USE) && !defined(SDL_DEBUG_H)
#define SDL_DEBUG_H
void MT2D_SDL_Ide_Printf(const char *String);
#endif
#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Win_Init.h/ MT2D_Win_Init.cpp created By Lucas Zimerman Fraulob

This file handles the Windows specific code required to init the MT2D
engine.
===================================================================*/




#if defined(_WIN32) && !defined(MT2DWIN_INIT_H)
#define MT2DWIN_INIT_H

#include "../MT2D_Terminal_Define.h"

MT2D_C_BEGIN
bool MT2D_Win_Init(void);
MT2D_C_END

#ifdef SDL_USE //SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib
#include <SDL.h>
#undef main
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
//#pragma comment(lib, "SDL2_ttf.lib")
#pragma comment(lib, "SDL2_mixer.lib")
#endif

#endif
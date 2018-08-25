/*==============================================================================================
SDL_MT2D_Render.cpp / SDL_MT2D_Render.h

Created By Lucas Zimerman Fraulob
With this code you'll be able to render a terminal like user interface into an opengl window.

-Rotating on smartphones aren't quite supported so lock the screen view to only landscape.

==============================================================================================*/

#ifndef MTSDL_REnh
#define MTSDL_REnh

#include "../../MT2D.h"
#include "../../ObjectCore/Sprites.h"

void Clean_Render();
void Render_New(unsigned char BUFFER[][MAX_HOR]);
void SDL_Render();

void SDL_Add_ImagetoBuffer(struct Sprite *IMG, int X, int Y);
void SDL_Clear_RenderBuffers();

/**/
void MT2D_SDL_Clear_Main_Window();
void MT2D_SDL_Draw_Window(int which);

#endif
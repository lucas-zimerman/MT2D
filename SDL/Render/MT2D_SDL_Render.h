/*==============================================================================================
SDL_MT2D_Render.cpp / SDL_MT2D_Render.h

Created By Lucas Zimerman Fraulob
With this code you'll be able to render a terminal like user interface into an opengl window.

-Rotating on smartphones aren't quite supported so lock the screen view to only landscape.

==============================================================================================*/

#include <MT2D/MT2D.h>

void Clean_Render();
void Render_New(unsigned char BUFFER[][MAX_HOR]);
void SDL_Render();
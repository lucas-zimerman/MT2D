#include <MT2D/ObjectCore/MT2D_CDecl.h>
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

MT2D_C_BEGIN
void Clean_Render(void);
void Render_New(unsigned char BUFFER[][MAX_HOR]);
void SDL_Render(void);

void SDL_Add_ImagetoBuffer(struct Sprite *IMG, int X, int Y);
void SDL_Clear_RenderBuffers(void);

/* Image sprites from RGBA cells (bytes R,G,B,A), used by MT2D_Sprite_Create_From_Raw / _Alter_Block / _To_SpriteRaw.
   Create: NULL on failure. Update: writes a w*h block (rows pitch bytes apart) at (x,y), already clipped by the caller.
   Read: w*h packed RGBA into out, heavy (render target + SDL_RenderReadPixels). Update/Read return 0 on failure. */
MT2D_Internal_Image *MT2D_SDL_Image_Create_Rgba(int w, int h, const unsigned char *rgba);
int MT2D_SDL_Image_Update_Rgba(MT2D_Internal_Image *img, int x, int y, int w, int h, const unsigned char *rgba, int pitch);
int MT2D_SDL_Image_Read_Rgba(MT2D_Internal_Image *img, int w, int h, unsigned char *out);

/**/
void MT2D_SDL_Clear_Main_Window(void);
void MT2D_SDL_Draw_Window(unsigned char (*window)[MAX_HOR]); /* shows window, WINDOW1 when NULL */
MT2D_C_END

#endif
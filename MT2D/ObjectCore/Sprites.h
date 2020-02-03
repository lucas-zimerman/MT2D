#ifndef SPRITES_
#define SPRITES_
#ifndef __ANDROID__
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/MT2D_Terminal_Define.h>
#else
#warning  "#include \../Objectcore/Coordinates.h\ testar se funciona melhor agora"
#include "Coordinates.h"
#include "../MT2D_Terminal_Define.h"
#endif

struct Sprite {
	char **Data;
#ifdef SDL_USE
	char **RotatedTexture;
#endif
	struct Coord size;//The size of the sprite
	struct Coord scale;//the scale of the image applied to a 320x240 screen
	char type;/* ASCII image = 0, Image = 1*/
};

/**
	Loads an ASCII OEM850 formated text file
**/
struct Sprite *Load_Sprite(char *file);

/**
	Loads an image
**/
struct Sprite *Load_Sprite_Image(char *file,int ScaleX,int ScaleY);
#if defined(MT2D_USING_CONTAINER)
struct Sprite *Load_Sprite_Image_From_Container(char *file, int ScaleX, int ScaleY);
#else
#define Load_Sprite_Image_From_Container null

#endif
bool Sprite_Render_on_Window(struct Sprite *img, int witch_window, int pos_x, int pos_y);
void Sprite_Delete(struct Sprite *Me);
#endif	
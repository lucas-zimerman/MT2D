#ifndef SPRITES_
#define SPRITES_
#include <MT2D/Objectcore/Coordinates.h>
#include <MT2D/MT2D_Terminal_Define.h>

struct Sprite {
	char **Data;
	Coord size;//The size of the sprite
	Coord scale;//the scale of the image applied to a 320x240 screen
	char type;/* ASCII image = 0, Image = 1*/
};

/**
	Loads an ASCII OEM850 formated text file
**/
Sprite *Load_Sprite(char *file);

/**
	Loads an image
**/
Sprite *Load_Sprite_Image(char *file,int ScaleX,int ScaleY);
bool Sprite_Render_on_Window(Sprite *img, int witch_window, int pos_x, int pos_y);
#endif	
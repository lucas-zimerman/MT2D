#ifndef SPRITES_
#define SPRITES_
#include <MT2D/Objectcore/Coordinates.h>

struct Sprite {
	char **Data;
	Coord size;//The size of the sprite
	Coord scale;//Ajust the 
};

Sprite *Load_Sprite(char *file);
bool Sprite_Render_on_Window(Sprite *img, int witch_window, int pos_x, int pos_y);
#endif	
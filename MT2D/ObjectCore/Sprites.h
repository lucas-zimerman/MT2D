#ifndef SPRITES_
#define SPRITES_
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/MT2D_Terminal_Define.h>

struct Sprite{
        char **Data;
#ifdef SDL_USE
        char **RotatedTexture;
#endif
        int SizeX;//The size of the sprite
        int SizeY;//The size of the sprite
        int ScaleX;//the scale of the image applied to a 320x240 screen
        int ScaleY;//the scale of the image applied to a 320x240 screen
        char type;/* ASCII image = 0, Image = 1*/
        unsigned int refCount;//used for memory management
};

/**
        Loads an ASCII OEM850 formated text file
**/
Sprite *Load_Sprite(char *file);

/**
        Loads an image
**/
Sprite *Load_Sprite_Image(char *file,int ScaleX,int ScaleY);
#if defined(MT2D_USING_CONTAINER)
Sprite *Load_Sprite_Image_From_Container(char *file, int ScaleX, int ScaleY);
#else
#define Load_Sprite_Image_From_Container null

#endif
bool Sprite_Render_on_Window(Sprite *img, int witch_window, int pos_x, int pos_y);
void Sprite_Delete(Sprite *Me);
#endif  

#ifndef SPRITES_
#define SPRITES_
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/MT2D_Text_Image.h>
#include <MT2D/ObjectCore/MT2D_Internal_Image.h>

struct Sprite{
        union {
                MT2D_Text_Image *Text;       //type 0: ASCII image, compact slices, same on every platform
                MT2D_Internal_Image *Image;  //type 1: platform specific image (SDL only for now)
        };//which member is valid is defined by "type", keep them in sync
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
/**
        Creates an ASCII sprite from SizeX*SizeY cells (row major, 0 = transparent).
        Returns NULL if the sprite is too big (see MT2D_Text_Image) or there is no memory.
**/
Sprite *Sprite_Create_Ascii_From_Cells(int SizeX, int SizeY, const char *cells);
/**
        Returns the character at (x,y) of an ASCII sprite.
        Returns 0 (transparent) for null sprites, image sprites or out of range positions.
**/
char Sprite_Get_Char(const Sprite *Me, int x, int y);
bool Sprite_Render_on_Window(Sprite *img, int witch_window, int pos_x, int pos_y);
void Sprite_Delete(Sprite *Me);
#endif  

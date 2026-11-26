#ifndef SPRITES_
#define SPRITES_
#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D_Window.h>
#include <MT2D/ObjectCore/MT2D_Text_Image.h>
#include <MT2D/ObjectCore/MT2D_Internal_Image.h>
#include <MT2D/ObjectCore/SprRaw.h>

MT2D_C_BEGIN

struct Sprite{
        MT2D_Text_Image *Text; /* type 0: ASCII image, compact slices, same on every platform (0 for other types) */
        MT2D_Internal_Image *Image; /* type 1: platform specific image (SDL, VGA), 0 for other types */
        /* which member is valid is defined by "type", the other one is kept 0 (C89 has no anonymous unions) */
        int SizeX; /* The size of the sprite */
        int SizeY; /* The size of the sprite */
        int ScaleX; /* the scale of the image applied to a 320x240 screen */
        int ScaleY; /* the scale of the image applied to a 320x240 screen */
        char type;/* ASCII image = 0, Image = 1*/
        unsigned int refCount; /* used for memory management */
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

/*
        Editing sprites, see SprRaw.h for MT2D_SpriteRaw and the raw format each sprite accepts:
        ASCII sprites and MS-DOS VGA / Linux Sixel images take 1 byte cells, SDL images take RGBA cells.
*/

/*
        Creates a renderable sprite with the size and content of Raw (Raw is copied, it can be freed after).
        A blank sprite is made from a raw that was just created (all cells transparent).
        image: false = ASCII sprite, true = image sprite.
        ScaleX/ScaleY: drawn size on the 320x240 screen (same meaning as in Load_Sprite_Image),
        <= 0 uses the size of Raw. ASCII sprites ignore them (always 1).
        Returns NULL when:
          - Raw is NULL, or its format is not the one the sprite takes (see above),
          - image is true on a build without image sprites (only SDL, MS-DOS VGA and Linux Sixel have them),
          - Raw is too big for an ASCII image (more than 65535 non transparent cells),
          - there is no memory, or SDL fails to create the textures.
*/
Sprite *MT2D_Sprite_Create_From_Raw(const MT2D_SpriteRaw *Raw, bool image, int ScaleX, int ScaleY);

/*
        Writes Raw into Me with its top left corner at (x,y). Every cell is replaced, a transparent cell
        erases (it does not mean "keep"). The parts of Raw outside of Me are clipped (a raw at x < 0 or y < 0
        loses its first columns or rows). A single pixel is a 1x1 raw.
        Image sprites that are queued for the next MT2D_Draw_Window show the new content.
        Returns the number of cells written, 0 when Raw misses Me entirely, Me/Raw is NULL, the format of Raw
        is not the one Me takes, or there is no memory (ASCII sprites are rebuilt, the old content is then kept).
*/
int MT2D_Sprite_Alter_Block(Sprite *Me, int x, int y, const MT2D_SpriteRaw *Raw);

/*
        Returns a new raw copy of the content of Me (free it with MT2D_SpriteRaw_Free), in the format Me takes,
        so it can be edited and written back with MT2D_Sprite_Alter_Block. Check Raw->IsRgba:
          - ASCII sprite: 1 byte cells (characters)
          - MS-DOS VGA / Linux Sixel image: 1 byte cells (palette indexes)
          - SDL image: RGBA cells
        <!> Heavy call, depending on the platform:
          - ASCII: the compressed image is expanded, O(width * height).
          - VGA / Sixel: a copy of the pixels.
          - SDL: the pixels only exist on the GPU. The sprite is drawn into a temporary render target and read
            back (SDL_RenderReadPixels), which waits for the GPU to finish everything queued. Do not call it every
            frame. Keep the raw and edit it instead of reading the sprite again.
        Returns NULL when Me is NULL, the build can not read it (SDL without render target support), or there is
        no memory.
*/
MT2D_SpriteRaw *MT2D_Sprite_To_SpriteRaw(const Sprite *Me);

/* ASCII sprites are copied into window (WINDOW1 when NULL). Image sprites are queued for the next MT2D_Draw_Window,
   only when window is NULL or WINDOW1 (false otherwise). */
bool Sprite_Render_on_Window(Sprite *img, unsigned char (*window)[MAX_HOR], int pos_x, int pos_y);
void Sprite_Delete(Sprite *Me);
MT2D_C_END
#endif  

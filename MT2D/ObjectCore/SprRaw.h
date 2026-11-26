#ifndef MT2D_SPRRAW_H
#define MT2D_SPRRAW_H
#include <MT2D/ObjectCore/MT2D_CDecl.h>

/*
        MT2D_SpriteRaw: uncompressed, freely editable sprite content.

        A raw sprite can not be rendered. Make a Sprite from it (MT2D_Sprite_Create_From_Raw),
        write it into a Sprite (MT2D_Sprite_Alter_Block) or read a Sprite into one
        (MT2D_Sprite_To_SpriteRaw), see Sprites.h.

        Cells are stored row by row, top row first: cell (x,y) is
                Raw->Data.Cells[y * Raw->Width + x]   (IsRgba = false)
                Raw->Data.Rgba [y * Raw->Width + x]   (IsRgba = true)
        so the data has the layout of a C array cells[Height][Width].
        A cell of 0 (or A = 0 for RGBA) is transparent.

        Which raw a sprite accepts:
          - ASCII sprite:                    1 byte cells, characters
          - MS-DOS VGA / Linux Sixel image:  1 byte cells, palette indexes
          - SDL image:                       RGBA cells
*/

/* One RGBA cell, 4 bytes, no padding (checked at compile time in SprRaw.c). A: 0 = transparent, 255 = opaque. */
typedef struct {
        unsigned char R;
        unsigned char G;
        unsigned char B;
        unsigned char A;
} MT2D_RGBA;

typedef struct {
        int Width;
        int Height;
        bool IsRgba;                  /* false: 1 byte cells, true: MT2D_RGBA cells */
        union {
                unsigned char *Cells; /* IsRgba = false: Width * Height bytes */
                MT2D_RGBA *Rgba;      /* IsRgba = true: Width * Height pixels */
        } Data;
} MT2D_SpriteRaw;

MT2D_C_BEGIN

/*
        Creates a raw sprite of Width*Height cells, all 0 (transparent). The struct and the cells are one allocation.
        Returns NULL when Width/Height is <= 0 or > 32767, or there is no memory.
*/
MT2D_SpriteRaw *MT2D_SpriteRaw_Create(int Width, int Height, bool isRgba);

/*
        Returns a new raw with the W*H area of Raw whose top left corner is (x,y), in the same format as Raw.
        The area is clipped to Raw: the result is only the part that is inside, so it can be smaller than
        W*H (check its Width/Height).
        Returns NULL when Raw is NULL, W/H is <= 0, the area misses Raw entirely, or there is no memory.
*/
MT2D_SpriteRaw *MT2D_SpriteRaw_Clip(const MT2D_SpriteRaw *Raw, int x, int y, int W, int H);

/*
        Frees a raw made by MT2D_SpriteRaw_Create, MT2D_SpriteRaw_Clip or MT2D_Sprite_To_SpriteRaw (NULL is accepted).
        Do not call it on a raw you filled yourself (its Data pointing to your own buffer).
*/
void MT2D_SpriteRaw_Free(MT2D_SpriteRaw *Raw);

MT2D_C_END

#endif

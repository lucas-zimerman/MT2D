/**
    VGA SPRITE TILE, CREATED BY: LUCAS ZIMERMAN FRAULOB

    THIS IS A SIMPLE STRUCT FOR STORING WHAT SPRITES ARE
    GOING TO BE RENDERED UNDER A TEXT GRID.

    IT ALSO STORES WHAT WAS THE PREVIOUS SPRITES TO BE
    RENDERED TO AVOID REDRAWING THE SAME IMAGE AGAIN.
**/

#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_SPRITE_TILE_RENDER &&  SpritesTilesRenderH
#define SpritesTilesRenderH

typedef struct {
	unsigned int *Ids;/*A pointer for the sprite array list that are on the list of to be rendered*/
	unsigned int Count;/*The amount of sprites to be rendered in this specific tile*/
   unsigned int DEBUG_OLD_COUNT;
	unsigned int OldData; /*Store the previous sprites information that were rendered*/
	unsigned int NewData; /*Store the present sprites information that are going to be rendered*/
	/*Way of counting the Data:
		Old/New Data -> 16 Bits
		[5] bits of sum from all horizontal coords
		[5] bits of sum from all vertical coords
		[6] bits of sum from all sprite pointers
	*/
}SpritesTilesRender;

#endif
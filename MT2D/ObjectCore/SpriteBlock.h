#ifndef SPRITEBLOCK_
#define SPRITEBLOCK_

#include <MT2D/Objectcore/Sprites.h>

typedef struct {
        Sprite *sprites; //start = 0
        int cnt; // start = 0
} SpriteBlock;

SpriteBlock *SpriteBlock_Create();
bool SpriteBlock_Add_Sprite(SpriteBlock *Block, Sprite *Ref);

#endif

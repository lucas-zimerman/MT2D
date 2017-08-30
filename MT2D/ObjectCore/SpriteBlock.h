#ifndef SPRITEBLOCK_
#define SPRITEBLOCK_
#ifndef __ANDROID__
#include <MT2D/Objectcore/Sprites.h>
#else
#include "../Objectcore/Sprites.h"
#endif

struct SpriteBlock {
	Sprite *sprites; //start = 0
	int cnt; // start = 0
};

SpriteBlock *SpriteBlock_Create();
bool SpriteBlock_Add_Sprite(SpriteBlock *Block, Sprite *Ref);

#endif
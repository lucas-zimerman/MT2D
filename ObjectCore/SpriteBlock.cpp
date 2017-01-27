//#include "Sprites.h"
#include "SpriteBlock.h"
#include <stdlib.h>

SpriteBlock *SpriteBlock_Create() {//creates an empty sprite block
	SpriteBlock *SB = 0;
	SB = (SpriteBlock*)malloc(sizeof(SpriteBlock));
	SB->sprites = 0;
	SB->cnt = 0;
	return SB;
}

bool SpriteBlock_Add_Sprite(SpriteBlock *Block, Sprite *Ref) {// Output = false if Sprite is invalid or true if valid
	bool out = false;
	if (Ref) {
		out = true;
		if (Block->cnt > 0) {
			Block->sprites = (Sprite*)realloc(Block->sprites, (Block->cnt + 2)*sizeof(Sprite));
		}else{
			Block->sprites = (Sprite*)malloc(sizeof(Sprite));
		}
		Block->sprites[Block->cnt] = *Ref;
		Block->cnt++;
	}
	return out;
}
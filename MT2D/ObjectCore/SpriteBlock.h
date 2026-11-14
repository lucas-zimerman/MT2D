#ifndef SPRITEBLOCK_
#define SPRITEBLOCK_

#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/ObjectCore/Sprites.h>

typedef struct {
        Sprite *sprites; /* start = 0 */
        int cnt; /* start = 0 */
} SpriteBlock;

MT2D_C_BEGIN

SpriteBlock *SpriteBlock_Create(void);
bool SpriteBlock_Add_Sprite(SpriteBlock *Block, Sprite *Ref);

MT2D_C_END

#endif

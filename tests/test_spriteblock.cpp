// Tests for MT2D/ObjectCore/SpriteBlock.cpp
#include <mt2d_test.h>
#include <MT2D/ObjectCore/SpriteBlock.h>
#include <stdlib.h>

static Sprite make(int sx, int sy) {
    Sprite s;
    s.Text = 0;
    s.Image = 0;
    s.SizeX = sx;
    s.SizeY = sy;
    s.ScaleX = 1;
    s.ScaleY = 1;
    s.type = 0;
    s.refCount = 0;
    return s;
}

TEST(create_is_empty) {
    SpriteBlock *b = SpriteBlock_Create();
    CHECK(b != 0);
    CHECK_EQ(b->cnt, 0);
    CHECK(b->sprites == 0);
    free(b);
}

TEST(add_null_sprite_is_rejected) {
    SpriteBlock *b = SpriteBlock_Create();
    CHECK(!SpriteBlock_Add_Sprite(b, 0));
    CHECK_EQ(b->cnt, 0);
    free(b);
}

TEST(add_sprites_copies_them_in_order) {
    SpriteBlock *b = SpriteBlock_Create();
    Sprite a = make(3, 1), c = make(5, 2), d = make(7, 3);
    CHECK(SpriteBlock_Add_Sprite(b, &a));
    CHECK(SpriteBlock_Add_Sprite(b, &c));
    CHECK(SpriteBlock_Add_Sprite(b, &d));
    CHECK_EQ(b->cnt, 3);
    CHECK_EQ(b->sprites[0].SizeX, 3);
    CHECK_EQ(b->sprites[1].SizeX, 5);
    CHECK_EQ(b->sprites[2].SizeY, 3);
    // the block stores a copy of the struct
    a.SizeX = 99;
    CHECK_EQ(b->sprites[0].SizeX, 3);
    free(b->sprites);
    free(b);
}

MT2D_TEST_MAIN()

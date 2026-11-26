// Tests for MT2D/ObjectCore/SprRaw.c
#include <mt2d_test.h>
#include <MT2D/ObjectCore/SprRaw.h>
#include <string.h>

static void fill_index(MT2D_SpriteRaw *r) {
    // cell (x,y) = y * 10 + x + 1, so 0 (transparent) never appears
    for (int y = 0; y < r->Height; y++)
        for (int x = 0; x < r->Width; x++)
            r->Data.Cells[y * r->Width + x] = (unsigned char)(y * 10 + x + 1);
}

TEST(rgba_cell_is_4_bytes) {
    CHECK_EQ((int)sizeof(MT2D_RGBA), 4);
}

TEST(create_is_zeroed) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(3, 2, false);
    CHECK(r != 0);
    CHECK_EQ(r->Width, 3);
    CHECK_EQ(r->Height, 2);
    CHECK(!r->IsRgba);
    for (int i = 0; i < 6; i++) CHECK_EQ(r->Data.Cells[i], 0);
    MT2D_SpriteRaw_Free(r);

    r = MT2D_SpriteRaw_Create(2, 2, true);
    CHECK(r != 0);
    CHECK(r->IsRgba);
    for (int i = 0; i < 4; i++) {
        CHECK_EQ(r->Data.Rgba[i].R, 0);
        CHECK_EQ(r->Data.Rgba[i].A, 0);
    }
    // the last pixel is writable (the allocation holds 4 bytes per cell)
    r->Data.Rgba[3].A = 255;
    CHECK_EQ(r->Data.Cells[15], 255);
    MT2D_SpriteRaw_Free(r);
}

TEST(create_rejects_bad_sizes) {
    CHECK(MT2D_SpriteRaw_Create(0, 2, false) == 0);
    CHECK(MT2D_SpriteRaw_Create(2, 0, false) == 0);
    CHECK(MT2D_SpriteRaw_Create(-1, 2, true) == 0);
    CHECK(MT2D_SpriteRaw_Create(32768, 1, false) == 0);
    MT2D_SpriteRaw_Free(0); // null is accepted
}

TEST(clip_inside) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(5, 4, false);
    fill_index(r);
    MT2D_SpriteRaw *c = MT2D_SpriteRaw_Clip(r, 1, 2, 3, 2);
    CHECK(c != 0);
    CHECK_EQ(c->Width, 3);
    CHECK_EQ(c->Height, 2);
    CHECK_EQ(c->Data.Cells[0], 22); // (1,2)
    CHECK_EQ(c->Data.Cells[2], 24); // (3,2)
    CHECK_EQ(c->Data.Cells[3], 32); // (1,3)
    CHECK_EQ(c->Data.Cells[5], 34); // (3,3)
    MT2D_SpriteRaw_Free(c);
    MT2D_SpriteRaw_Free(r);
}

TEST(clip_shrinks_to_the_overlap) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(5, 4, false);
    fill_index(r);
    // top left outside
    MT2D_SpriteRaw *c = MT2D_SpriteRaw_Clip(r, -2, -1, 4, 3);
    CHECK(c != 0);
    CHECK_EQ(c->Width, 2);
    CHECK_EQ(c->Height, 2);
    CHECK_EQ(c->Data.Cells[0], 1);  // (0,0)
    CHECK_EQ(c->Data.Cells[3], 12); // (1,1)
    MT2D_SpriteRaw_Free(c);
    // bottom right outside
    c = MT2D_SpriteRaw_Clip(r, 3, 2, 100, 100);
    CHECK(c != 0);
    CHECK_EQ(c->Width, 2);
    CHECK_EQ(c->Height, 2);
    CHECK_EQ(c->Data.Cells[0], 24); // (3,2)
    CHECK_EQ(c->Data.Cells[3], 35); // (4,3)
    MT2D_SpriteRaw_Free(c);
    // bigger than the raw on every side: the whole raw
    c = MT2D_SpriteRaw_Clip(r, -10, -10, 100, 100);
    CHECK(c != 0);
    CHECK_EQ(c->Width, 5);
    CHECK_EQ(c->Height, 4);
    CHECK(memcmp(c->Data.Cells, r->Data.Cells, 20) == 0);
    MT2D_SpriteRaw_Free(c);
    MT2D_SpriteRaw_Free(r);
}

TEST(clip_misses) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(5, 4, false);
    CHECK(MT2D_SpriteRaw_Clip(r, 5, 0, 2, 2) == 0);
    CHECK(MT2D_SpriteRaw_Clip(r, 0, 4, 2, 2) == 0);
    CHECK(MT2D_SpriteRaw_Clip(r, -2, 0, 2, 2) == 0);
    CHECK(MT2D_SpriteRaw_Clip(r, 0, 0, 0, 2) == 0);
    CHECK(MT2D_SpriteRaw_Clip(r, 0, 0, 2, -1) == 0);
    CHECK(MT2D_SpriteRaw_Clip(0, 0, 0, 2, 2) == 0);
    // x + W overflowing an int does not wrap around
    CHECK(MT2D_SpriteRaw_Clip(r, 2147483647, 0, 2147483647, 2) == 0);
    MT2D_SpriteRaw_Free(r);
}

TEST(clip_rgba_keeps_the_format) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(3, 3, true);
    r->Data.Rgba[4].R = 10; // (1,1)
    r->Data.Rgba[4].G = 20;
    r->Data.Rgba[4].B = 30;
    r->Data.Rgba[4].A = 40;
    r->Data.Rgba[8].A = 99; // (2,2)
    MT2D_SpriteRaw *c = MT2D_SpriteRaw_Clip(r, 1, 1, 5, 5);
    CHECK(c != 0);
    CHECK(c->IsRgba);
    CHECK_EQ(c->Width, 2);
    CHECK_EQ(c->Height, 2);
    CHECK_EQ(c->Data.Rgba[0].R, 10);
    CHECK_EQ(c->Data.Rgba[0].G, 20);
    CHECK_EQ(c->Data.Rgba[0].B, 30);
    CHECK_EQ(c->Data.Rgba[0].A, 40);
    CHECK_EQ(c->Data.Rgba[3].A, 99);
    MT2D_SpriteRaw_Free(c);
    MT2D_SpriteRaw_Free(r);
}

MT2D_TEST_MAIN()

// Tests for MT2D/ObjectCore/MT2D_Text_Image.cpp (slice based ASCII image)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/MT2D_Text_Image.h>
#include <string.h>

TEST(type_sizes_are_fixed) {
    CHECK_EQ(sizeof(MT2D_U8), 1);
    CHECK_EQ(sizeof(MT2D_U16), 2);
    CHECK_EQ(sizeof(MT2D_U32), 4);
}

TEST(rejects_invalid_input) {
    char c[4] = {1, 1, 1, 1};
    CHECK(MT2D_Text_Image_Create(0, 1, c) == 0);
    CHECK(MT2D_Text_Image_Create(1, 0, c) == 0);
    CHECK(MT2D_Text_Image_Create(2, 2, 0) == 0);
}

TEST(round_trips_every_cell) {
    const char cells[] = {'A', 0, 'B', 'C',
                          0, 0, 0, 0,
                          'D', 'E', 'F', 0};
    MT2D_Text_Image *img = MT2D_Text_Image_Create(4, 3, cells);
    CHECK(img != 0);
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 4; x++)
            CHECK_EQ(MT2D_Text_Image_Get(img, x, y), cells[y * 4 + x]);
    CHECK_EQ(img->sliceCount, 3); // "A", "BC", "DEF"
    CHECK_EQ(img->dataSize, 6);
    MT2D_Text_Image_Free(img);
}

TEST(get_out_of_range_is_transparent) {
    const char cells[] = {'A', 'B'};
    MT2D_Text_Image *img = MT2D_Text_Image_Create(2, 1, cells);
    CHECK_EQ(MT2D_Text_Image_Get(img, -1, 0), 0);
    CHECK_EQ(MT2D_Text_Image_Get(img, 2, 0), 0);
    CHECK_EQ(MT2D_Text_Image_Get(img, 0, 1), 0);
    CHECK_EQ(MT2D_Text_Image_Get(0, 0, 0), 0);
    MT2D_Text_Image_Free(img);
}

TEST(fully_transparent_image_has_no_slices) {
    const char cells[6] = {0};
    MT2D_Text_Image *img = MT2D_Text_Image_Create(3, 2, cells);
    CHECK(img != 0);
    CHECK_EQ(img->sliceCount, 0);
    CHECK_EQ(MT2D_Text_Image_Get(img, 1, 1), 0);
    MT2D_Text_Image_Free(img);
}

TEST(blit_clips_and_skips_transparent) {
    const char cells[] = {'A', 0, 'C'};
    MT2D_Text_Image *img = MT2D_Text_Image_Create(3, 1, cells);
    unsigned char dst[2][4];
    memset(dst, '.', sizeof(dst));
    MT2D_Text_Image_Blit(img, &dst[0][0], 4, 2, 0, 1);
    CHECK_EQ(dst[1][0], 'A');
    CHECK_EQ(dst[1][1], '.'); // transparent
    CHECK_EQ(dst[1][2], 'C');
    CHECK_EQ(dst[0][0], '.');
    // clipped on the left: 'A' is outside, 'C' lands on column 1
    memset(dst, '.', sizeof(dst));
    MT2D_Text_Image_Blit(img, &dst[0][0], 4, 2, -1, 0);
    CHECK_EQ(dst[0][0], '.');
    CHECK_EQ(dst[0][1], 'C');
    // clipped on the right, must not wrap into the next row
    memset(dst, '.', sizeof(dst));
    MT2D_Text_Image_Blit(img, &dst[0][0], 4, 2, 3, 0);
    CHECK_EQ(dst[0][3], 'A');
    CHECK_EQ(dst[1][0], '.');
    MT2D_Text_Image_Free(img);
}

TEST(sparse_sprite_uses_less_memory_than_raw_cells) {
    // 40x20 sprite: mostly transparent, a small opaque block in the middle
    char cells[40 * 20] = {0};
    for (int y = 8; y < 12; y++)
        for (int x = 18; x < 22; x++) cells[y * 40 + x] = '#';
    MT2D_Text_Image *img = MT2D_Text_Image_Create(40, 20, cells);
    CHECK(img != 0);
    CHECK(MT2D_Text_Image_Memory_Size(img) < sizeof(cells));
    MT2D_Text_Image_Free(img);
}
MT2D_TEST_MAIN()

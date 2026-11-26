// Tests for MT2D/ObjectCore/Sprites.c
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void clear_windows() {
    memset(WINDOW1, 0, sizeof(WINDOW1));
}

static Sprite *make_sprite(const char *row1, const char *row2) {
    int w = (int)strlen(row1);
    char cells[64] = {0};
    memcpy(cells, row1, w);
    if (row2) memcpy(cells + w, row2, w);
    return Sprite_Create_Ascii_From_Cells(w, row2 ? 2 : 1, cells);
}

TEST(load_sprite_from_missing_file_returns_null) {
    CHECK(Load_Sprite((char *)"does_not_exist.txt") == 0);
}

TEST(load_sprite_reads_ascii_rows) {
    // MT2D ascii sprites use CR LF line endings
    FILE *f = fopen("sprite.txt", "wb");
    CHECK(f != 0);
    fputs("AB\r\nCD\r\n", f);
    fclose(f);

    Sprite *s = Load_Sprite((char *)"sprite.txt");
    CHECK(s != 0);
    if (s) {
        CHECK_EQ(s->type, 0);
        CHECK_EQ(s->ScaleX, 1);
        CHECK_EQ(s->ScaleY, 1);
        CHECK(s->SizeX >= 2);
        CHECK(s->SizeY >= 2);
        CHECK_EQ(Sprite_Get_Char(s, 0, 0), 'A');
        CHECK_EQ(Sprite_Get_Char(s, 1, 0), 'B');
        CHECK_EQ(Sprite_Get_Char(s, 0, 1), 'C');
        CHECK_EQ(Sprite_Get_Char(s, 1, 1), 'D');
        Sprite_Delete(s);
    }
    remove("sprite.txt");
}

TEST(load_sprite_image_without_sdl_returns_placeholder) {
    Sprite *s = Load_Sprite_Image((char *)"any.png", 4, 5);
    CHECK(s != 0);
    CHECK_EQ(s->type, 0);
    CHECK_EQ(s->ScaleX, 4);
    CHECK_EQ(s->ScaleY, 5);
    CHECK_EQ(s->SizeX, 3);
    CHECK_EQ(s->SizeY, 1);
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), '<');
    CHECK_EQ(Sprite_Get_Char(s, 1, 0), '!');
    CHECK_EQ(Sprite_Get_Char(s, 2, 0), '>');
}

TEST(render_on_window1) {
    clear_windows();
    Sprite *s = make_sprite("AB", "CD");
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, 10, 4));
    CHECK_EQ(WINDOW1[4][10], 'A');
    CHECK_EQ(WINDOW1[4][11], 'B');
    CHECK_EQ(WINDOW1[5][10], 'C');
    CHECK_EQ(WINDOW1[5][11], 'D');
}

TEST(render_on_custom_window) {
    static MT2D_Window custom;
    clear_windows();
    memset(custom, 0, sizeof(custom));
    Sprite *s = make_sprite("AB", 0);
    CHECK(Sprite_Render_on_Window(s, custom, 1, 1));
    CHECK_EQ(custom[1][1], 'A');
    CHECK_EQ(custom[1][2], 'B');
    CHECK_EQ(WINDOW1[1][1], 0);
}

TEST(render_skips_null_characters) {
    clear_windows();
    WINDOW1[2][2] = 'z';
    Sprite *s = Sprite_Create_Ascii_From_Cells(3, 1, "A\0C"); // middle cell is transparent
    Sprite_Render_on_Window(s, DISPLAY_WINDOW1, 1, 2);
    CHECK_EQ(WINDOW1[2][1], 'A');
    CHECK_EQ(WINDOW1[2][2], 'z');
    CHECK_EQ(WINDOW1[2][3], 'C');
}

TEST(render_clips_left_of_screen) {
    clear_windows();
    Sprite *s = make_sprite("ABC", 0);
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, -1, 0));
    // the first column is cut off, nothing is written outside of the buffer
    CHECK_EQ(WINDOW1[0][0], 'B');
    CHECK_EQ(WINDOW1[0][1], 'C');
}

TEST(render_clips_right_of_screen) {
    clear_windows();
    Sprite *s = make_sprite("ABC", 0);
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, MAX_HOR - 2, 0));
    CHECK_EQ(WINDOW1[0][MAX_HOR - 2], 'A');
    CHECK_EQ(WINDOW1[0][MAX_HOR - 1], 'B');
    CHECK_EQ(WINDOW1[1][0], 0); // no wrap around into the next row
}

TEST(render_clips_top_and_bottom) {
    clear_windows();
    Sprite *s = make_sprite("AB", "CD");
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, 0, -1));
    CHECK_EQ(WINDOW1[0][0], 'C');
    CHECK_EQ(WINDOW1[0][1], 'D');
    clear_windows();
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, 3, MAX_VER - 1));
    CHECK_EQ(WINDOW1[MAX_VER - 1][3], 'A');
    CHECK_EQ(WINDOW1[MAX_VER - 1][4], 'B');
}

TEST(render_fully_offscreen_writes_nothing) {
    clear_windows();
    Sprite *s = make_sprite("AB", "CD");
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, MAX_HOR + 5, MAX_VER + 5));
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, -10, -10));
    CHECK_EQ(WINDOW1[0][0], 0);
}

TEST(char_accessors_check_bounds) {
    Sprite *s = make_sprite("AB", 0);
    CHECK_EQ(Sprite_Get_Char(s, 2, 0), 0);
    CHECK_EQ(Sprite_Get_Char(s, 0, 1), 0);
    CHECK_EQ(Sprite_Get_Char(s, -1, 0), 0);
    CHECK_EQ(Sprite_Get_Char(0, 0, 0), 0);
    Sprite_Delete(s);
}

TEST(render_null_sprite_returns_false) {
    CHECK(!Sprite_Render_on_Window(0, DISPLAY_WINDOW1, 0, 0));
}

TEST(render_image_sprite_is_not_drawn_in_terminal_mode) {
    clear_windows();
    Sprite *s = make_sprite("AB", 0);
    s->type = 1;
    CHECK(!Sprite_Render_on_Window(s, DISPLAY_WINDOW1, 0, 0));
    CHECK_EQ(WINDOW1[0][0], 0);
}

TEST(delete_sprite_respects_refcount) {
    Sprite *s = make_sprite("A", 0);
    s->refCount = 2;
    Sprite_Delete(s);
    CHECK_EQ(s->refCount, 1); // still alive
    Sprite_Delete(s);         // freed now
    Sprite_Delete(0);         // null is accepted
}

// ---- editing with MT2D_SpriteRaw (terminal build: ASCII sprites only) ----

static MT2D_SpriteRaw *raw_from(int w, int h, const char *cells) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(w, h, false);
    memcpy(r->Data.Cells, cells, (size_t)(w * h));
    return r;
}

TEST(create_from_raw_ascii) {
    MT2D_SpriteRaw *r = raw_from(3, 2, "AB\0D\0F");
    Sprite *s = MT2D_Sprite_Create_From_Raw(r, false, 7, 7);
    MT2D_SpriteRaw_Free(r); // the sprite has its own copy
    CHECK(s != 0);
    CHECK_EQ(s->type, 0);
    CHECK_EQ(s->SizeX, 3);
    CHECK_EQ(s->SizeY, 2);
    CHECK_EQ(s->ScaleX, 1); // ignored by ASCII sprites
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), 'A');
    CHECK_EQ(Sprite_Get_Char(s, 1, 0), 'B');
    CHECK_EQ(Sprite_Get_Char(s, 2, 0), 0);
    CHECK_EQ(Sprite_Get_Char(s, 0, 1), 'D');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), 0);
    CHECK_EQ(Sprite_Get_Char(s, 2, 1), 'F');
    Sprite_Delete(s);
}

TEST(create_from_blank_raw_is_transparent) {
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(4, 3, false);
    Sprite *s = MT2D_Sprite_Create_From_Raw(r, false, 0, 0);
    MT2D_SpriteRaw_Free(r);
    CHECK(s != 0);
    CHECK_EQ(s->SizeX, 4);
    CHECK_EQ(s->SizeY, 3);
    clear_windows();
    WINDOW1[0][0] = 'z';
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW1, 0, 0));
    CHECK_EQ(WINDOW1[0][0], 'z'); // nothing drawn
    Sprite_Delete(s);
}

TEST(create_from_raw_rejects_wrong_formats) {
    MT2D_SpriteRaw *rgba = MT2D_SpriteRaw_Create(2, 2, true);
    MT2D_SpriteRaw *cells = MT2D_SpriteRaw_Create(2, 2, false);
    CHECK(MT2D_Sprite_Create_From_Raw(0, false, 0, 0) == 0);
    CHECK(MT2D_Sprite_Create_From_Raw(rgba, false, 0, 0) == 0); // ASCII needs 1 byte cells
    // the terminal build has no image sprites
    CHECK(MT2D_Sprite_Create_From_Raw(cells, true, 0, 0) == 0);
    CHECK(MT2D_Sprite_Create_From_Raw(rgba, true, 0, 0) == 0);
    MT2D_SpriteRaw_Free(rgba);
    MT2D_SpriteRaw_Free(cells);
}

TEST(alter_block_inside) {
    Sprite *s = Sprite_Create_Ascii_From_Cells(4, 3, "abcd" "efgh" "ijkl");
    MT2D_SpriteRaw *r = raw_from(2, 2, "XY" "Z\0");
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 1, 1, r), 4);
    CHECK_EQ(Sprite_Get_Char(s, 0, 1), 'e');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), 'X');
    CHECK_EQ(Sprite_Get_Char(s, 2, 1), 'Y');
    CHECK_EQ(Sprite_Get_Char(s, 3, 1), 'h');
    CHECK_EQ(Sprite_Get_Char(s, 1, 2), 'Z');
    CHECK_EQ(Sprite_Get_Char(s, 2, 2), 0); // a transparent cell erases
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), 'a');
    MT2D_SpriteRaw_Free(r);
    Sprite_Delete(s);
}

TEST(alter_block_single_cell) {
    Sprite *s = Sprite_Create_Ascii_From_Cells(2, 1, "ab");
    MT2D_SpriteRaw *r = raw_from(1, 1, "Q");
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 1, 0, r), 1);
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), 'a');
    CHECK_EQ(Sprite_Get_Char(s, 1, 0), 'Q');
    MT2D_SpriteRaw_Free(r);
    Sprite_Delete(s);
}

TEST(alter_block_clips_top_left) {
    Sprite *s = Sprite_Create_Ascii_From_Cells(3, 3, "abc" "def" "ghi");
    MT2D_SpriteRaw *r = raw_from(3, 3, "123" "456" "789");
    // only 5 6 / 8 9 land on the sprite, at (0,0)
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, -1, -1, r), 4);
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), '5');
    CHECK_EQ(Sprite_Get_Char(s, 1, 0), '6');
    CHECK_EQ(Sprite_Get_Char(s, 2, 0), 'c');
    CHECK_EQ(Sprite_Get_Char(s, 0, 1), '8');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), '9');
    CHECK_EQ(Sprite_Get_Char(s, 2, 2), 'i');
    MT2D_SpriteRaw_Free(r);
    Sprite_Delete(s);
}

TEST(alter_block_clips_bottom_right) {
    Sprite *s = Sprite_Create_Ascii_From_Cells(3, 3, "abc" "def" "ghi");
    MT2D_SpriteRaw *r = raw_from(3, 3, "123" "456" "789");
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 2, 1, r), 2);
    CHECK_EQ(Sprite_Get_Char(s, 2, 1), '1');
    CHECK_EQ(Sprite_Get_Char(s, 2, 2), '4');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), 'e');
    MT2D_SpriteRaw_Free(r);
    Sprite_Delete(s);
}

TEST(alter_block_misses_or_rejects) {
    Sprite *s = Sprite_Create_Ascii_From_Cells(2, 2, "abcd");
    MT2D_SpriteRaw *r = raw_from(2, 2, "WXYZ");
    MT2D_SpriteRaw *rgba = MT2D_SpriteRaw_Create(2, 2, true);
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 2, 0, r), 0);
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 0, -2, r), 0);
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 2147483647, 0, r), 0); // no int overflow
    CHECK_EQ(MT2D_Sprite_Alter_Block(0, 0, 0, r), 0);
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 0, 0, 0), 0);
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 0, 0, rgba), 0); // ASCII takes 1 byte cells
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), 'a');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), 'd');
    MT2D_SpriteRaw_Free(rgba);
    MT2D_SpriteRaw_Free(r);
    Sprite_Delete(s);
}

TEST(alter_block_with_own_buffer) {
    // a raw filled by hand, pointing to a 2D array of the caller
    unsigned char block[2][3] = { { 'p', 'q', 'r' }, { 's', 0, 'u' } };
    MT2D_SpriteRaw r;
    r.Width = 3;
    r.Height = 2;
    r.IsRgba = false;
    r.Data.Cells = &block[0][0];
    Sprite *s = Sprite_Create_Ascii_From_Cells(3, 2, "abcdef");
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 0, 0, &r), 6);
    CHECK_EQ(Sprite_Get_Char(s, 2, 0), 'r');
    CHECK_EQ(Sprite_Get_Char(s, 0, 1), 's');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), 0);
    Sprite_Delete(s);
}

TEST(to_sprite_raw_ascii_round_trip) {
    Sprite *s = Sprite_Create_Ascii_From_Cells(3, 2, "a\0c" "def");
    MT2D_SpriteRaw *r = MT2D_Sprite_To_SpriteRaw(s);
    CHECK(r != 0);
    CHECK(!r->IsRgba);
    CHECK_EQ(r->Width, 3);
    CHECK_EQ(r->Height, 2);
    CHECK(memcmp(r->Data.Cells, "a\0c" "def", 6) == 0);
    // edit the raw and write it back
    r->Data.Cells[1] = 'B';
    CHECK_EQ(MT2D_Sprite_Alter_Block(s, 0, 0, r), 6);
    CHECK_EQ(Sprite_Get_Char(s, 1, 0), 'B');
    MT2D_SpriteRaw_Free(r);
    CHECK(MT2D_Sprite_To_SpriteRaw(0) == 0);
    Sprite_Delete(s);
}

TEST(to_sprite_raw_then_clip_makes_a_tile) {
    Sprite *sheet = Sprite_Create_Ascii_From_Cells(4, 2, "AABB" "AABB");
    MT2D_SpriteRaw *all = MT2D_Sprite_To_SpriteRaw(sheet);
    MT2D_SpriteRaw *tile = MT2D_SpriteRaw_Clip(all, 2, 0, 2, 2);
    Sprite *s = MT2D_Sprite_Create_From_Raw(tile, false, 0, 0);
    CHECK(s != 0);
    CHECK_EQ(s->SizeX, 2);
    CHECK_EQ(Sprite_Get_Char(s, 0, 0), 'B');
    CHECK_EQ(Sprite_Get_Char(s, 1, 1), 'B');
    MT2D_SpriteRaw_Free(tile);
    MT2D_SpriteRaw_Free(all);
    Sprite_Delete(s);
    Sprite_Delete(sheet);
}

MT2D_TEST_MAIN()

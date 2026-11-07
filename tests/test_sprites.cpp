// Tests for MT2D/ObjectCore/Sprites.cpp
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void clear_windows() {
    memset(WINDOW1, 0, sizeof(WINDOW1));
    memset(WINDOW2, 0, sizeof(WINDOW2));
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
    CHECK_EQ(WINDOW2[4][10], 0); // other buffer untouched
}

TEST(render_on_window2) {
    clear_windows();
    Sprite *s = make_sprite("AB", 0);
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW2, 1, 1));
    CHECK_EQ(WINDOW2[1][1], 'A');
    CHECK_EQ(WINDOW2[1][2], 'B');
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
    CHECK(Sprite_Render_on_Window(s, DISPLAY_WINDOW2, 3, MAX_VER - 1));
    CHECK_EQ(WINDOW2[MAX_VER - 1][3], 'A');
    CHECK_EQ(WINDOW2[MAX_VER - 1][4], 'B');
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

MT2D_TEST_MAIN()

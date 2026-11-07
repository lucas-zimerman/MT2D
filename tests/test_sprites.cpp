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
    Sprite *s = (Sprite *)malloc(sizeof(Sprite));
    s->SizeX = (int)strlen(row1);
    s->SizeY = row2 ? 2 : 1;
    s->ScaleX = s->ScaleY = 1;
    s->type = 0;
    s->refCount = 0;
    s->Data = (char **)malloc(s->SizeY * sizeof(char *));
    s->Data[0] = (char *)malloc(s->SizeX + 1);
    strcpy(s->Data[0], row1);
    if (row2) {
        s->Data[1] = (char *)malloc(s->SizeX + 1);
        strcpy(s->Data[1], row2);
    }
    return s;
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
        CHECK_EQ(s->Data[0][0], 'A');
        CHECK_EQ(s->Data[0][1], 'B');
        CHECK_EQ(s->Data[1][0], 'C');
        CHECK_EQ(s->Data[1][1], 'D');
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
    CHECK_EQ(s->Data[0][0], '<');
    CHECK_EQ(s->Data[0][1], '!');
    CHECK_EQ(s->Data[0][2], '>');
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
    Sprite *s = make_sprite("A_C", 0);
    s->Data[0][1] = 0; // transparent cell
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

// Tests for MT2D/MT2D_Layout.cpp and MT2D/Tools/Video/MT2D_Generic_Line.cpp
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <MT2D/MT2D_Layout.h>
#include <MT2D/Tools/Video/MT2D_Generic_Line.h>
#include <string.h>

// code page 437 box characters used by the engine
enum {
    TOP_LEFT = 201, TOP_RIGHT = 187, BOTTOM_LEFT = 200, BOTTOM_RIGHT = 188,
    HORIZONTAL = 205, VERTICAL = 186, SHADOW = 176
};

static void clear_windows() {
    memset(WINDOW1, 0, sizeof(WINDOW1));
    memset(WINDOW2, 0, sizeof(WINDOW2));
}

TEST(window_draws_border_corners_and_background) {
    clear_windows();
    MT2D_Create_Window_Layout(5, 2, 10, 6, false, false, false, ' ', DISPLAY_WINDOW1);

    CHECK_EQ(WINDOW1[2][5], TOP_LEFT);
    CHECK_EQ(WINDOW1[2][15], TOP_RIGHT);
    CHECK_EQ(WINDOW1[8][5], BOTTOM_LEFT);
    CHECK_EQ(WINDOW1[8][15], BOTTOM_RIGHT);
    for (int x = 6; x < 15; x++) {
        CHECK_EQ(WINDOW1[2][x], HORIZONTAL);
        CHECK_EQ(WINDOW1[8][x], HORIZONTAL);
    }
    for (int y = 3; y < 8; y++) {
        CHECK_EQ(WINDOW1[y][5], VERTICAL);
        CHECK_EQ(WINDOW1[y][15], VERTICAL);
    }
    // inside is filled with the background char
    CHECK_EQ(WINDOW1[3][6], ' ');
    CHECK_EQ(WINDOW1[7][14], ' ');
    // nothing is written on the other buffer
    CHECK_EQ(WINDOW2[2][5], 0);
}

TEST(window_draws_on_window2) {
    clear_windows();
    MT2D_Create_Window_Layout(0, 0, 4, 4, false, false, false, '.', DISPLAY_WINDOW2);
    CHECK_EQ(WINDOW2[0][0], TOP_LEFT);
    CHECK_EQ(WINDOW2[0][4], TOP_RIGHT);
    CHECK_EQ(WINDOW2[4][0], BOTTOM_LEFT);
    CHECK_EQ(WINDOW2[4][4], BOTTOM_RIGHT);
    CHECK_EQ(WINDOW2[1][1], '.');
    CHECK_EQ(WINDOW1[0][0], 0);
}

TEST(window_without_shadow_leaves_outside_untouched) {
    clear_windows();
    MT2D_Create_Window_Layout(5, 2, 10, 6, false, false, false, ' ', DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[3][16], 0);
    CHECK_EQ(WINDOW1[9][6], 0);
}

TEST(window_shadow_is_drawn_at_the_right_and_bottom) {
    clear_windows();
    MT2D_Create_Window_Layout(5, 2, 10, 6, true, false, false, ' ', DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[3][16], SHADOW);
    CHECK_EQ(WINDOW1[9][16], SHADOW);
    CHECK_EQ(WINDOW1[9][6], SHADOW);
    CHECK_EQ(WINDOW1[9][15], SHADOW);
}

TEST(window_title_adds_a_separator_line) {
    clear_windows();
    MT2D_Create_Window_Layout(5, 2, 10, 8, false, true, false, ' ', DISPLAY_WINDOW1);
    for (int x = 6; x < 15; x++) {
        CHECK_EQ(WINDOW1[4][x], HORIZONTAL); // pos_y + 2
    }
    CHECK_EQ(WINDOW1[5][6], ' ');
}

TEST(window_option_adds_a_separator_line_near_the_bottom) {
    clear_windows();
    MT2D_Create_Window_Layout(5, 2, 10, 8, false, false, true, ' ', DISPLAY_WINDOW1);
    for (int x = 6; x < 15; x++) {
        CHECK_EQ(WINDOW1[8][x], HORIZONTAL); // pos_y + tam_y - 2
    }
    CHECK_EQ(WINDOW1[7][6], ' ');
}

TEST(window_is_clipped_to_the_screen_width) {
    clear_windows();
    // would be wider than MAX_HOR, must not write past the row
    MT2D_Create_Window_Layout(MAX_HOR - 5, 0, 20, 3, false, false, false, ' ', DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[0][MAX_HOR - 5], TOP_LEFT);
    CHECK_EQ(WINDOW1[0][MAX_HOR - 4], HORIZONTAL);
    CHECK_EQ(WINDOW1[0][MAX_HOR - 1], HORIZONTAL);
}

// The names of the line helpers describe the line direction the other way
// around from what they draw: "Vertical" fills a whole row and "Horizontal"
// fills a whole column. The tests document the current behaviour.
TEST(line_draw_vertical_fills_one_row) {
    clear_windows();
    MT2D_Line_Draw_Vertical(3, '=');
    for (int x = 0; x < MAX_HOR; x++) {
        CHECK_EQ(WINDOW1[3][x], '=');
    }
    CHECK_EQ(WINDOW1[2][0], 0);
    CHECK_EQ(WINDOW1[4][0], 0);
}

TEST(line_draw_horizontal_fills_one_column) {
    clear_windows();
    MT2D_Line_Draw_Horizontal(7, '|');
    for (int y = 0; y < MAX_VER; y++) {
        CHECK_EQ(WINDOW1[y][7], '|');
    }
    CHECK_EQ(WINDOW1[0][6], 0);
    CHECK_EQ(WINDOW1[0][8], 0);
}

MT2D_TEST_MAIN()

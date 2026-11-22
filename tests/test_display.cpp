// Tests for the display buffers in MT2D/MT2D.cpp
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <string.h>

static void clear_windows() {
    memset(WINDOW1, 0, sizeof(WINDOW1));
}

TEST(insert_string_on_window1) {
    clear_windows();
    insert_string_on_display((char *)"Hello", 2, 5, DISPLAY_WINDOW1);
    CHECK(memcmp(&WINDOW1[2][5], "Hello", 5) == 0);
    CHECK_EQ(WINDOW1[2][4], 0);
    CHECK_EQ(WINDOW1[2][10], 0);
}

TEST(insert_empty_string_changes_nothing) {
    clear_windows();
    insert_string_on_display((char *)"", 1, 1, DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[1][1], 0);
}

TEST(insert_number_positive) {
    clear_windows();
    insert_number_on_display(1234, 3, 10, DISPLAY_WINDOW1);
    CHECK(memcmp(&WINDOW1[3][10], "1234", 4) == 0);
    CHECK_EQ(WINDOW1[3][14], 0); // no terminator or padding is written
}

TEST(insert_number_zero) {
    clear_windows();
    insert_number_on_display(0, 0, 0, DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[0][0], '0');
    CHECK_EQ(WINDOW1[0][1], 0);
}

TEST(insert_number_negative) {
    clear_windows();
    insert_number_on_display(-56, 4, 2, DISPLAY_WINDOW1);
    CHECK(memcmp(&WINDOW1[4][2], "-56", 3) == 0);
}

TEST(save_and_restore_area_puts_back_the_cells) {
    clear_windows();
    WINDOW1[3][4] = 'A';
    WINDOW1[4][6] = 'B';
    unsigned char saved[3 * 2];
    MT2D_Window_Save_Area(saved, 4, 3, 3, 2, DISPLAY_WINDOW1);
    WINDOW1[3][4] = 'x';
    WINDOW1[4][6] = 'y';
    WINDOW1[4][7] = 'z'; // outside the rectangle
    MT2D_Window_Restore_Area(saved, 4, 3, 3, 2, DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[3][4], 'A');
    CHECK_EQ(WINDOW1[4][6], 'B');
    CHECK_EQ(WINDOW1[4][5], 0);
    CHECK_EQ(WINDOW1[4][7], 'z');
}

TEST(save_and_restore_area_clips_to_the_screen) {
    clear_windows();
    WINDOW1[0][0] = 'A';
    WINDOW1[MAX_VER][MAX_HOR - 1] = 'B';
    unsigned char top_left[4 * 4];
    unsigned char bottom_right[4 * 4];
    memset(top_left, '#', sizeof(top_left));
    memset(bottom_right, '#', sizeof(bottom_right));
    MT2D_Window_Save_Area(top_left, -2, -2, 4, 4, DISPLAY_WINDOW1);
    MT2D_Window_Save_Area(bottom_right, MAX_HOR - 1, MAX_VER, 4, 4, DISPLAY_WINDOW1);
    CHECK_EQ(top_left[2 * 4 + 2], 'A');
    CHECK_EQ(top_left[0], '#'); // off screen cells are not touched
    CHECK_EQ(bottom_right[0], 'B');
    CHECK_EQ(bottom_right[1], '#');
    WINDOW1[0][0] = 'x';
    WINDOW1[MAX_VER][MAX_HOR - 1] = 'y';
    MT2D_Window_Restore_Area(top_left, -2, -2, 4, 4, DISPLAY_WINDOW1);
    MT2D_Window_Restore_Area(bottom_right, MAX_HOR - 1, MAX_VER, 4, 4, DISPLAY_WINDOW1);
    CHECK_EQ(WINDOW1[0][0], 'A');
    CHECK_EQ(WINDOW1[MAX_VER][MAX_HOR - 1], 'B');
}

static MT2D_Window custom;

TEST(insert_string_on_custom_window) {
    clear_windows();
    memset(custom, 0, sizeof(custom));
    insert_string_on_display((char *)"Hi", 0, 0, custom);
    CHECK_EQ(custom[0][0], 'H');
    CHECK_EQ(custom[0][1], 'i');
    CHECK_EQ(WINDOW1[0][0], 0);
}

TEST(insert_number_on_custom_window) {
    clear_windows();
    memset(custom, 0, sizeof(custom));
    insert_number_on_display(-7, 1, 1, custom);
    CHECK_EQ(custom[1][1], '-');
    CHECK_EQ(custom[1][2], '7');
    CHECK_EQ(WINDOW1[1][1], 0);
}

TEST(null_window_is_window1) {
    clear_windows();
    insert_string_on_display((char *)"A", 2, 2, NULL);
    insert_string_on_display((char *)"B", 2, 3, WINDOW1);
    CHECK_EQ(WINDOW1[2][2], 'A');
    CHECK_EQ(WINDOW1[2][3], 'B');
}

TEST(save_and_restore_area_on_custom_window) {
    clear_windows();
    memset(custom, 0, sizeof(custom));
    custom[5][5] = 'A';
    unsigned char saved[1];
    MT2D_Window_Save_Area(saved, 5, 5, 1, 1, custom);
    custom[5][5] = 'x';
    MT2D_Window_Restore_Area(saved, 5, 5, 1, 1, custom);
    CHECK_EQ(custom[5][5], 'A');
    CHECK_EQ(WINDOW1[5][5], 0);
}

MT2D_TEST_MAIN()

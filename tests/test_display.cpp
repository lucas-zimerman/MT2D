// Tests for the display buffers in MT2D/MT2D.cpp
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <string.h>

static void clear_windows() {
    memset(WINDOW1, 0, sizeof(WINDOW1));
    memset(WINDOW2, 0, sizeof(WINDOW2));
}

TEST(insert_string_on_window1) {
    clear_windows();
    insert_string_on_display((char *)"Hello", 2, 5, DISPLAY_WINDOW1);
    CHECK(memcmp(&WINDOW1[2][5], "Hello", 5) == 0);
    CHECK_EQ(WINDOW1[2][4], 0);
    CHECK_EQ(WINDOW1[2][10], 0);
    CHECK_EQ(WINDOW2[2][5], 0);
}

TEST(insert_string_on_window2) {
    clear_windows();
    insert_string_on_display((char *)"Hi", 0, 0, DISPLAY_WINDOW2);
    CHECK_EQ(WINDOW2[0][0], 'H');
    CHECK_EQ(WINDOW2[0][1], 'i');
    CHECK_EQ(WINDOW1[0][0], 0);
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

TEST(insert_number_on_window2) {
    clear_windows();
    insert_number_on_display(7, 1, 1, DISPLAY_WINDOW2);
    CHECK_EQ(WINDOW2[1][1], '7');
    CHECK_EQ(WINDOW1[1][1], 0);
}

TEST(transfer_window1_to_window2_copies_the_screen) {
    clear_windows();
    WINDOW1[0][0] = 'A';
    WINDOW1[10][40] = 'B';
    WINDOW1[MAX_VER - 2][MAX_HOR - 1] = 'C';
    transfer_window1_to_window2();
    CHECK_EQ(WINDOW2[0][0], 'A');
    CHECK_EQ(WINDOW2[10][40], 'B');
    CHECK_EQ(WINDOW2[MAX_VER - 2][MAX_HOR - 1], 'C');
}

TEST(transfer_window2_to_window1_copies_the_screen) {
    clear_windows();
    WINDOW2[5][5] = 'X';
    WINDOW1[5][5] = 'o';
    transfer_window2_to_window1();
    CHECK_EQ(WINDOW1[5][5], 'X');
}

TEST(transfer_overwrites_the_destination) {
    clear_windows();
    WINDOW2[3][3] = 'o';
    transfer_window1_to_window2(); // WINDOW1 is blank there
    CHECK_EQ(WINDOW2[3][3], 0);
}

MT2D_TEST_MAIN()

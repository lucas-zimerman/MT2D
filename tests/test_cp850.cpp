// Tests for the CP850 <-> Unicode conversion of the Linux/macOS terminal backend
#include <mt2d_test.h>
#include "../MT2D/_LINUX/MT2D_Linux_CP850.h"

TEST(ascii_is_unchanged) {
    for (int c = 32; c < 127; c++) {
        CHECK_EQ(MT2D_CP850_To_Unicode((unsigned char)c), (unsigned int)c);
        CHECK_EQ(MT2D_Unicode_To_CP850((unsigned int)c), c);
    }
}

TEST(known_characters) {
    CHECK_EQ(MT2D_CP850_To_Unicode(0x80), 0x00C7u); // Ç
    CHECK_EQ(MT2D_CP850_To_Unicode(0x87), 0x00E7u); // ç
    CHECK_EQ(MT2D_CP850_To_Unicode(0xA0), 0x00E1u); // á
    CHECK_EQ(MT2D_CP850_To_Unicode(0xC6), 0x00E3u); // ã (CP437 has ╞ here)
    CHECK_EQ(MT2D_CP850_To_Unicode(0xC9), 0x2554u); // ╔
    CHECK_EQ(MT2D_CP850_To_Unicode(0xCD), 0x2550u); // ═
    CHECK_EQ(MT2D_CP850_To_Unicode(0xDB), 0x2588u); // █
    CHECK_EQ(MT2D_CP850_To_Unicode(0xE1), 0x00DFu); // ß
}

TEST(blank_and_symbols) {
    CHECK_EQ(MT2D_CP850_To_Unicode(0), 0x20u);
    CHECK_EQ(MT2D_CP850_To_Unicode(1), 0x263Au);
    CHECK_EQ(MT2D_CP850_To_Unicode(24), 0x2191u);
    CHECK_EQ(MT2D_CP850_To_Unicode(127), 0x2302u);
    CHECK_EQ(MT2D_CP850_To_Unicode(255), 0xA0u);
}

TEST(high_half_round_trips) {
    for (int c = 0x80; c <= 0xFF; c++) {
        unsigned int u = MT2D_CP850_To_Unicode((unsigned char)c);
        CHECK(u != 0);
        CHECK_EQ(MT2D_Unicode_To_CP850(u), c);
    }
}

TEST(high_half_is_unique) {
    for (int a = 0x80; a <= 0xFF; a++)
        for (int b = a + 1; b <= 0xFF; b++)
            CHECK(MT2D_CP850_To_Unicode((unsigned char)a) != MT2D_CP850_To_Unicode((unsigned char)b));
}

TEST(unmappable_characters) {
    CHECK_EQ(MT2D_Unicode_To_CP850(0x20AC), -1); // euro
    CHECK_EQ(MT2D_Unicode_To_CP850(0x4E2D), -1);
    CHECK_EQ(MT2D_Unicode_To_CP850(0x0A), -1);
}

MT2D_TEST_MAIN()

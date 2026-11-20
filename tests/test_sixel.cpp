// Tests for the Sixel encoder and the image sprites of the Linux terminal graphics mode
#include <mt2d_test.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "../MT2D/_LINUX/Render/MT2D_Linux_Sixel.h"
#include "../MT2D/_LINUX/Render/MT2D_Linux_Image.h"

struct Frame {
    std::vector<unsigned char> pixels;
    MT2D_Sixel_Canvas canvas;
    Frame(const std::vector<unsigned char> &cells, int cols, int rows) : pixels(cols * 8 * rows * 16) {
        MT2D_Sixel_Draw_Text(&canvas, pixels.data(), cells.data(), cols, rows);
    }
    std::string Encode() {
        std::vector<char> out(2 * 1024 * 1024);
        unsigned int len = MT2D_Sixel_Encode(&canvas, out.data(), (unsigned int)out.size());
        CHECK(len > 0);
        CHECK_EQ((size_t)len, strlen(out.data()));
        return std::string(out.data(), len);
    }
};

static size_t Count(const std::string &s, char c) {
    size_t n = 0;
    for (size_t i = 0; i < s.size(); i++) if (s[i] == c) n++;
    return n;
}

TEST(header_and_footer) {
    Frame f(std::vector<unsigned char>(80 * 24, ' '), 80, 24);
    std::string s = f.Encode();
    CHECK(s.compare(0, 8, "\033P0;0;0q") == 0);
    CHECK(s.find("\"1;1;640;384") != std::string::npos);
    CHECK(s.find("#0;2;0;0;0#1;2;85;85;85") != std::string::npos);
    CHECK(s.compare(s.size() - 2, 2, "\033\\") == 0);
}

TEST(blank_screen_has_no_pixels) {
    // 384 pixel rows = 64 bands, each one is only "-": no color is used, so no pass is written
    Frame f(std::vector<unsigned char>(80 * 24, ' '), 80, 24);
    std::string s = f.Encode();
    CHECK_EQ(Count(s, '-'), (size_t)64);
    CHECK(s.find('?') == std::string::npos);
}

TEST(solid_block_is_run_length_encoded) {
    // CP850 0xDB is a full block: 8 pixels wide, 16 tall = 2 full bands ('~' = 6 pixels) + a band of 4 rows ('N')
    Frame f(std::vector<unsigned char>(1, 0xDB), 1, 1);
    CHECK(f.Encode().find("#1!8~-#1!8~-#1!8N-") != std::string::npos);
}

TEST(glyph_has_pixels_in_every_text_band) {
    Frame f(std::vector<unsigned char>(1, 'A'), 1, 1);
    std::string s = f.Encode();
    size_t start = s.find("#1;2;85;85;85") + 13;
    int bandsWithPixels = 0;
    bool any = false;
    for (size_t i = start; i < s.size(); i++) {
        if (s[i] == '-') { if (any) bandsWithPixels++; any = false; }
        else if (s[i] >= '@' && s[i] <= '~') any = true;
    }
    CHECK_EQ(Count(s, '-'), (size_t)3);
    CHECK(bandsWithPixels >= 2);
}

TEST(small_buffer_is_refused) {
    Frame f(std::vector<unsigned char>(1, 0xDB), 1, 1);
    char out[16];
    CHECK_EQ(MT2D_Sixel_Encode(&f.canvas, out, sizeof(out)), 0u);
    CHECK_EQ(MT2D_Sixel_Encode(&f.canvas, NULL, 100), 0u);
    // exactly the needed size works, one byte less does not
    std::string s = f.Encode();
    std::vector<char> exact(s.size() + 1);
    CHECK_EQ((size_t)MT2D_Sixel_Encode(&f.canvas, exact.data(), (unsigned int)exact.size()), s.size());
    CHECK_EQ(MT2D_Sixel_Encode(&f.canvas, exact.data(), (unsigned int)exact.size() - 1), 0u);
}

TEST(add_color_reuses_and_falls_back) {
    Frame f(std::vector<unsigned char>(1, ' '), 1, 1);
    CHECK_EQ(MT2D_Sixel_Add_Color(&f.canvas, 0, 0, 0), 0);
    CHECK_EQ(MT2D_Sixel_Add_Color(&f.canvas, 217, 217, 217), 1);
    int red = MT2D_Sixel_Add_Color(&f.canvas, 255, 0, 0);
    CHECK_EQ(red, 2);
    CHECK_EQ(MT2D_Sixel_Add_Color(&f.canvas, 255, 0, 0), 2);
    CHECK_EQ(f.canvas.Colors, 3);
    for (int i = 0; f.canvas.Colors < MT2D_SIXEL_COLORS; i++) MT2D_Sixel_Add_Color(&f.canvas, i % 256, (i / 256) % 256, 7);
    // full palette: the nearest color is returned instead of a new one
    int n = MT2D_Sixel_Add_Color(&f.canvas, 254, 1, 1);
    CHECK(n < MT2D_SIXEL_COLORS);
    CHECK_EQ(f.canvas.Colors, MT2D_SIXEL_COLORS);
}

TEST(image_is_drawn_over_text_with_transparency) {
    Frame f(std::vector<unsigned char>(1, 0xDB), 1, 1); // 8x16, all index 1
    const unsigned char pal[3][3] = { {0, 255, 255}, {255, 0, 0}, {0, 0, 255} };
    const unsigned char img[4] = { 0, 1, 2, 1 }; // 2x2, index 0 transparent
    MT2D_Sixel_Draw_Image(&f.canvas, img, pal, 3, 2, 2, 0, 0, 4, 4);
    // each source pixel is a 2x2 block: (0,0) transparent keeps the text, (1,0) red, (0,1) blue, (1,1) red
    unsigned char *px = f.canvas.Pixels;
    int red = MT2D_Sixel_Add_Color(&f.canvas, 255, 0, 0), blue = MT2D_Sixel_Add_Color(&f.canvas, 0, 0, 255);
    CHECK_EQ((int)px[0 * 8 + 0], 1);
    CHECK_EQ((int)px[0 * 8 + 2], red);
    CHECK_EQ((int)px[2 * 8 + 0], blue);
    CHECK_EQ((int)px[3 * 8 + 3], red);
    CHECK_EQ((int)px[4 * 8 + 4], 1); // outside the image
    std::string s = f.Encode();
    CHECK(s.find("#2;2;100;0;0") != std::string::npos);
}

TEST(image_is_clipped) {
    Frame f(std::vector<unsigned char>(1, ' '), 1, 1);
    const unsigned char pal[2][3] = { {0, 0, 0}, {0, 255, 0} };
    const unsigned char img[4] = { 1, 1, 1, 1 };
    MT2D_Sixel_Draw_Image(&f.canvas, img, pal, 2, 2, 2, -3, -3, 6, 6); // only the 3x3 corner is inside
    MT2D_Sixel_Draw_Image(&f.canvas, img, pal, 2, 2, 2, 100, 100, 6, 6); // completely outside
    MT2D_Sixel_Draw_Image(&f.canvas, img, pal, 2, 2, 2, 0, 0, 0, 0); // empty
    int green = MT2D_Sixel_Add_Color(&f.canvas, 0, 255, 0);
    CHECK_EQ((int)f.canvas.Pixels[2 * 8 + 2], green);
    CHECK_EQ((int)f.canvas.Pixels[3 * 8 + 3], 0);
}

// ---- BMP loader ----
static void Le(std::vector<unsigned char> &v, size_t at, unsigned long x, int bytes) {
    for (int i = 0; i < bytes; i++) v[at + i] = (unsigned char)(x >> (8 * i));
}

// 3x2 8 bit BMP: pixel rows (top first) {1,2,3} and {4,5,6}, palette color i = (i*10, i*20, i*30)
static std::vector<unsigned char> MakeBmp() {
    const int colors = 8, width = 3, height = 2, stride = 4;
    std::vector<unsigned char> b(54 + colors * 4 + stride * height, 0);
    b[0] = 'B'; b[1] = 'M';
    Le(b, 10, 54 + colors * 4, 4);
    Le(b, 14, 40, 4);
    Le(b, 18, width, 4);
    Le(b, 22, height, 4);
    Le(b, 26, 1, 2);
    Le(b, 28, 8, 2);
    Le(b, 46, colors, 4);
    for (int i = 0; i < colors; i++) { // B, G, R, 0
        b[54 + i * 4] = (unsigned char)(i * 30);
        b[54 + i * 4 + 1] = (unsigned char)(i * 20);
        b[54 + i * 4 + 2] = (unsigned char)(i * 10);
    }
    size_t data = 54 + colors * 4;
    const unsigned char bottom[3] = { 4, 5, 6 }, top[3] = { 1, 2, 3 }; // stored bottom up
    memcpy(&b[data], bottom, 3);
    memcpy(&b[data + stride], top, 3);
    return b;
}

static FILE *TempFile(const std::vector<unsigned char> &bytes) {
    FILE *f = tmpfile();
    fwrite(bytes.data(), 1, bytes.size(), f);
    rewind(f);
    return f;
}

TEST(bmp_loads_pixels_and_palette) {
    FILE *f = TempFile(MakeBmp());
    Sprite s;
    memset(&s, 0, sizeof(s));
    CHECK_EQ(MT2D_Linux_Load_BMP(f, &s), 1);
    fclose(f);
    CHECK_EQ(s.SizeX, 3);
    CHECK_EQ(s.SizeY, 2);
    CHECK_EQ((int)s.type, 1);
    const unsigned char expected[6] = { 1, 2, 3, 4, 5, 6 };
    CHECK(memcmp(s.Image->Pixels, expected, 6) == 0);
    CHECK_EQ(s.Image->Colors, 8);
    CHECK_EQ((int)s.Image->Palette[3][0], 30);
    CHECK_EQ((int)s.Image->Palette[3][1], 60);
    CHECK_EQ((int)s.Image->Palette[3][2], 90);
    MT2D_Linux_Image_Free(s.Image);
}

TEST(bmp_rejects_bad_files) {
    Sprite s;
    memset(&s, 0, sizeof(s));
    std::vector<unsigned char> bad = MakeBmp();
    bad[0] = 'X';
    FILE *f = TempFile(bad);
    CHECK_EQ(MT2D_Linux_Load_BMP(f, &s), 0);
    fclose(f);
    bad = MakeBmp();
    Le(bad, 28, 24, 2); // 24 bpp is not supported
    f = TempFile(bad);
    CHECK_EQ(MT2D_Linux_Load_BMP(f, &s), 0);
    fclose(f);
    bad = MakeBmp();
    bad.resize(bad.size() - 3); // truncated pixels
    f = TempFile(bad);
    CHECK_EQ(MT2D_Linux_Load_BMP(f, &s), 0);
    fclose(f);
    CHECK(s.Image == NULL);
}

TEST(image_path) {
    char out[32];
    CHECK(strcmp(MT2D_Linux_Image_Path("assets/Planet_Blue.png", out, sizeof(out)), "d/planet_b.bmp") == 0);
    CHECK(strcmp(MT2D_Linux_Image_Path("ship.png", out, sizeof(out)), "d/ship.bmp") == 0);
    CHECK(strcmp(MT2D_Linux_Image_Path("x/ship.bmp", out, sizeof(out)), "x/ship.bmp") == 0);
}

TEST(device_attributes_reply) {
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?62;4;22c"), 1);
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?4c"), 1);
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?4;6c"), 1);
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("junk\033[?1;2;4c"), 1);
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?64;1;2;6;9;15;18;21;22c"), 0); // no 4
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?1;2c"), 0);
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?14;24c"), 0); // 14 and 24 are not 4
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[?62;4"), 0); // cut before the end
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel("\033[>0;95;0c"), 0); // secondary attributes, not ours
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel(""), 0);
    CHECK_EQ(MT2D_Sixel_Reply_Supports_Sixel(NULL), 0);
}

MT2D_TEST_MAIN()

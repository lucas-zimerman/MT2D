// Tests for the Sixel encoder and the image sprites of the Linux terminal graphics mode
#include <mt2d_test.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "../MT2D/_LINUX/Render/MT2D_Linux_Sixel.h"
#include "../MT2D/_LINUX/Render/MT2D_Linux_Image.h"

static const int BG = 16, FG = 31; // the text colors of the renderer (MS-DOS: black and white of the VGA palette)

static void DefaultPalette() {
    int pal[MT2D_SIXEL_COLORS];
    MT2D_Sixel_Default_Palette(pal);
    MT2D_Sixel_Palette_Set_Rgb(0, pal, MT2D_SIXEL_COLORS);
}

struct Frame {
    std::vector<unsigned char> pixels;
    MT2D_Sixel_Canvas canvas;
    Frame(const std::vector<unsigned char> &cells, int cols, int rows) : pixels(cols * 8 * rows * 16) {
        DefaultPalette();
        MT2D_Sixel_Draw_Text(&canvas, pixels.data(), cells.data(), cols, rows, BG, FG);
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

static int Rgb(int index) {
    const unsigned char *c = MT2D_Sixel_Palette_Entry(index);
    return c ? (c[0] << 16) | (c[1] << 8) | c[2] : -1;
}

TEST(default_palette_is_the_vga_one) {
    int pal[MT2D_SIXEL_COLORS];
    MT2D_Sixel_Default_Palette(pal);
    CHECK_EQ(pal[0], 0x000000);    // EGA colors
    CHECK_EQ(pal[1], 0x0000AA);
    CHECK_EQ(pal[6], 0xAA5500);    // brown
    CHECK_EQ(pal[7], 0xAAAAAA);
    CHECK_EQ(pal[15], 0xFFFFFF);
    CHECK_EQ(pal[16], 0x000000);   // grays: 16 is the text background, 31 the text color
    CHECK_EQ(pal[17], 0x141414);
    CHECK_EQ(pal[31], 0xFFFFFF);
    CHECK_EQ(pal[32], 0x0000FF);   // first ramp: blue, then red rises
    CHECK_EQ(pal[33], 0x4100FF);
    CHECK_EQ(pal[36], 0xFF00FF);
    CHECK_EQ(pal[44], 0xFFFF00);
    CHECK_EQ(pal[56], 0x7D7DFF);   // second group: the lowest level is 31
    CHECK_EQ(pal[104], 0x000071);  // dark group, highest level 28: blue...
    CHECK_EQ(pal[108], 0x710071);  // ...and magenta four steps later
    for (int i = 248; i < 256; i++) CHECK_EQ(pal[i], 0);
}

TEST(palette_setters) {
    int pal[3] = { 0x112233, 0x445566, 0x778899 };
    MT2D_Sixel_Palette_Set_Rgb(10, pal, 3);
    CHECK_EQ(Rgb(10), 0x112233);
    CHECK_EQ(Rgb(12), 0x778899);
    int high[2] = { (int)0xFF000102, 0x0A0B0C }; // only the low 24 bits are used
    MT2D_Sixel_Palette_Set_Rgb(0, high, 2);
    CHECK_EQ(Rgb(0), 0x000102);
    int tail[2] = { 1, 2 };
    MT2D_Sixel_Palette_Set_Rgb(255, tail, 2); // the second one does not fit: ignored
    CHECK_EQ(Rgb(255), 1);
    const unsigned char bgra[8] = { 0x30, 0x20, 0x10, 0, 0x60, 0x50, 0x40, 0 }; // B,G,R,0
    MT2D_Sixel_Palette_Set_Bgra(bgra, 2);
    CHECK_EQ(Rgb(0), 0x102030);
    CHECK_EQ(Rgb(1), 0x405060);
    CHECK_EQ(Rgb(12), 0x778899); // the rest is not touched
    CHECK(MT2D_Sixel_Palette_Entry(-1) == NULL);
    CHECK(MT2D_Sixel_Palette_Entry(256) == NULL);
}

TEST(header_and_footer) {
    Frame f(std::vector<unsigned char>(80 * 24, ' '), 80, 24);
    std::string s = f.Encode();
    CHECK(s.compare(0, 8, "\033P0;0;0q") == 0);
    CHECK(s.find("\"1;1;640;384") != std::string::npos);
    CHECK(s.compare(s.size() - 2, 2, "\033\\") == 0);
}

TEST(only_the_used_colors_are_defined) {
    Frame f(std::vector<unsigned char>(80 * 24, ' '), 80, 24);
    std::string s = f.Encode();
    CHECK(s.find("#16;2;0;0;0") != std::string::npos); // the background
    CHECK_EQ(Count(s, ';') - 6, (size_t)3); // header: 0;0;0 and "1;1;W;H (6 with the size), then the one color definition: 2;0;0;0 -> 3 more
    CHECK(s.find("#31;") == std::string::npos); // no text, the foreground is not used
    CHECK(s.find("#0;") == std::string::npos);
}

TEST(blank_screen_is_one_run_per_band) {
    // 384 pixel rows = 64 bands, each one a single color, 640 pixels wide
    Frame f(std::vector<unsigned char>(80 * 24, ' '), 80, 24);
    std::string s = f.Encode();
    CHECK_EQ(Count(s, '-'), (size_t)64);
    CHECK(s.find("#16!640~-#16!640~-") != std::string::npos);
}

TEST(solid_block_is_run_length_encoded) {
    // CP850 0xDB is a full block: 8 pixels wide, 16 tall = 2 full bands ('~' = 6 pixels) + a band of 4 rows ('N')
    Frame f(std::vector<unsigned char>(1, 0xDB), 1, 1);
    std::string s = f.Encode();
    CHECK(s.find("#31!8~-#31!8~-#31!8N-") != std::string::npos);
    CHECK(s.find("#16;") == std::string::npos); // only the foreground is on the canvas
}

TEST(glyph_uses_the_background_and_the_foreground) {
    Frame f(std::vector<unsigned char>(1, 'A'), 1, 1);
    std::string s = f.Encode();
    CHECK(s.find("#16;2;0;0;0") != std::string::npos);
    CHECK(s.find("#31;2;100;100;100") != std::string::npos);
    CHECK_EQ(Count(s, '-'), (size_t)3);
    CHECK(s.find("$#") != std::string::npos); // a band has more than one color pass
}

TEST(text_colors_follow_the_palette) {
    Frame f(std::vector<unsigned char>(1, 0xDB), 1, 1);
    int red = 0xFF0000;
    MT2D_Sixel_Palette_Set_Rgb(FG, &red, 1); // a new palette after the draw is not in this canvas...
    CHECK_EQ(f.canvas.Palette[FG][0], 255);
    CHECK(f.Encode().find("#31;2;100;100;100") != std::string::npos);
    MT2D_Sixel_Draw_Text(&f.canvas, f.pixels.data(), std::vector<unsigned char>(1, 0xDB).data(), 1, 1, BG, FG); // ...the next frame has it
    CHECK(f.Encode().find("#31;2;100;0;0") != std::string::npos);
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

TEST(image_is_drawn_over_text_with_transparency) {
    Frame f(std::vector<unsigned char>(1, 0xDB), 1, 1); // 8x16, all FG (31)
    const unsigned char img[4] = { 0, 40, 41, 40 }; // 2x2 palette indexes, 0 is transparent
    MT2D_Sixel_Draw_Image(&f.canvas, img, 2, 2, 0, 0, 4, 4);
    // each source pixel is a 2x2 block: (0,0) transparent keeps the text, (1,0) = 40, (0,1) = 41, (1,1) = 40
    unsigned char *px = f.canvas.Pixels;
    CHECK_EQ((int)px[0 * 8 + 0], FG);
    CHECK_EQ((int)px[0 * 8 + 2], 40);
    CHECK_EQ((int)px[2 * 8 + 0], 41);
    CHECK_EQ((int)px[3 * 8 + 3], 40);
    CHECK_EQ((int)px[4 * 8 + 4], FG); // outside the image
    std::string s = f.Encode();
    CHECK(s.find("#40;2;") != std::string::npos);
    CHECK(s.find("#41;2;") != std::string::npos);
}

TEST(image_index_is_the_palette_index) {
    Frame f(std::vector<unsigned char>(1, ' '), 1, 1);
    int c = 0x00FF00;
    MT2D_Sixel_Palette_Set_Rgb(77, &c, 1);
    MT2D_Sixel_Draw_Text(&f.canvas, f.pixels.data(), std::vector<unsigned char>(1, ' ').data(), 1, 1, BG, FG);
    const unsigned char img[1] = { 77 };
    MT2D_Sixel_Draw_Image(&f.canvas, img, 1, 1, 0, 0, 8, 16);
    CHECK_EQ((int)f.canvas.Pixels[5 * 8 + 5], 77);
    CHECK(f.Encode().find("#77;2;0;100;0") != std::string::npos);
}

TEST(image_is_clipped) {
    Frame f(std::vector<unsigned char>(1, ' '), 1, 1);
    const unsigned char img[4] = { 50, 50, 50, 50 };
    MT2D_Sixel_Draw_Image(&f.canvas, img, 2, 2, -3, -3, 6, 6); // only the 3x3 corner is inside
    MT2D_Sixel_Draw_Image(&f.canvas, img, 2, 2, 100, 100, 6, 6); // completely outside
    MT2D_Sixel_Draw_Image(&f.canvas, img, 2, 2, 0, 0, 0, 0); // empty
    CHECK_EQ((int)f.canvas.Pixels[2 * 8 + 2], 50);
    CHECK_EQ((int)f.canvas.Pixels[3 * 8 + 3], BG);
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
    DefaultPalette();
    int before8 = Rgb(8);
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
    CHECK_EQ(Rgb(3), (30 << 16) | (60 << 8) | 90); // the palette of the file is the palette of the screen
    CHECK_EQ(Rgb(7), (70 << 16) | (140 << 8) | 210);
    CHECK_EQ(Rgb(8), before8); // a color the file does not have is not touched
    CHECK_EQ(Rgb(31), 0xFFFFFF);
    MT2D_Linux_Image_Free(s.Image);
}

TEST(bmp_rejects_bad_files) {
    DefaultPalette();
    int before = Rgb(3);
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
    CHECK_EQ(Rgb(3), before); // a bad file does not change the palette
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

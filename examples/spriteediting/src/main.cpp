/*
   Sprite editing: every call of the MT2D_SpriteRaw API (MT2D/ObjectCore/SprRaw.h and Sprites.h), page by page.
   TAB: next page, ESC: quit. The keys of each page are written on it.

     1 Paint       image sprite made blank from a raw, edited with MT2D_Sprite_Alter_Block: 1x1 pixels and an 8x8 brush
     2 Clipping    a stamp written partly (or fully) outside of a sprite, the return value is the number of cells written
     3 Read back   MT2D_Sprite_To_SpriteRaw (timed, it is the heavy call), tiles cut with MT2D_SpriteRaw_Clip, a clip that
                   shrinks to the overlap, and a read / edit / write back round trip
     4 Alpha       RGBA cells with semi transparency (SDL), or the proof that palette builds refuse RGBA raws
     5 ASCII       the same calls on an ASCII sprite (every build, also the text only one)

   Which raw a sprite takes depends on the build: SDL image sprites take RGBA cells, MS-DOS VGA and Linux Sixel image
   sprites take 1 byte palette indexes, ASCII sprites take 1 byte characters. The program finds it out at the start
   (detect_image_format) and paints with the colors of the table below in both formats.
   Nothing is loaded from files: every sprite is made in code.
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "example.h"
#include <MT2D/ObjectCore/Sprites.h>

#define PAGES 5
#define ROWS MAX_VER

enum ImageFormat { IMAGES_NONE, IMAGES_PALETTE, IMAGES_RGBA };
static ImageFormat g_format = IMAGES_NONE;

/* The first 16 colors of the default VGA palette (MT2D_Init gives it to the Sixel backend too), with their RGB */
struct Color {
    unsigned char index, r, g, b;
    const char *name;
};
static const Color COLORS[] = {
    {4, 170, 0, 0, "red"},         {2, 0, 170, 0, "green"},     {1, 0, 0, 170, "blue"},
    {14, 255, 255, 85, "yellow"},  {3, 0, 170, 170, "cyan"},    {5, 170, 0, 170, "magenta"},
    {6, 170, 85, 0, "brown"},      {12, 255, 85, 85, "pink"},   {10, 85, 255, 85, "light green"},
    {9, 85, 85, 255, "light blue"}, {7, 170, 170, 170, "gray"}, {15, 255, 255, 255, "white"},
};
#define COLOR_COUNT ((int)(sizeof(COLORS) / sizeof(COLORS[0])))
#define TRANSPARENT (-1)

/* ---------------------------------------------------------------------------------------------- helpers */

static void text(int row, int col, const char *fmt, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    insert_string_on_display(buf, row, col, DISPLAY_WINDOW1);
}

/* A raw in the format the image sprites of this build take */
static MT2D_SpriteRaw *image_raw(int w, int h)
{
    return MT2D_SpriteRaw_Create(w, h, g_format == IMAGES_RGBA);
}

/* Sets cell (x,y) of an image raw to a color of the table (or TRANSPARENT), alpha is only used by RGBA raws */
static void put(MT2D_SpriteRaw *r, int x, int y, int color, int alpha)
{
    int i = y * r->Width + x;
    if (x < 0 || y < 0 || x >= r->Width || y >= r->Height) return;
    if (r->IsRgba) {
        MT2D_RGBA *p = &r->Data.Rgba[i];
        if (color == TRANSPARENT) {
            p->R = p->G = p->B = p->A = 0;
        } else {
            p->R = COLORS[color].r;
            p->G = COLORS[color].g;
            p->B = COLORS[color].b;
            p->A = (unsigned char)alpha;
        }
    } else {
        r->Data.Cells[i] = color == TRANSPARENT ? 0 : COLORS[color].index;
    }
}

static MT2D_SpriteRaw *filled_raw(int w, int h, int color)
{
    MT2D_SpriteRaw *r = image_raw(w, h);
    int x, y;
    if (r)
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++) put(r, x, y, color, 255);
    return r;
}

/* Writes a w*h block of one color into s at (x,y), returns what MT2D_Sprite_Alter_Block returned */
static int paint_block(Sprite *s, int x, int y, int w, int h, int color)
{
    MT2D_SpriteRaw *r = filled_raw(w, h, color);
    int written = MT2D_Sprite_Alter_Block(s, x, y, r);
    MT2D_SpriteRaw_Free(r);
    return written;
}

static Sprite *image_from(MT2D_SpriteRaw *r)
{
    return MT2D_Sprite_Create_From_Raw(r, true, 0, 0); /* scale 0: drawn at its own size */
}

/* Image sprites take RGBA on SDL and palette indexes on VGA / Sixel: try both with a 1x1 sprite */
static ImageFormat detect_image_format(void)
{
    ImageFormat found = IMAGES_NONE;
    MT2D_SpriteRaw *rgba = MT2D_SpriteRaw_Create(1, 1, true);
    MT2D_SpriteRaw *cells = MT2D_SpriteRaw_Create(1, 1, false);
    Sprite *s = MT2D_Sprite_Create_From_Raw(rgba, true, 0, 0);
    if (s) {
        found = IMAGES_RGBA;
    } else if ((s = MT2D_Sprite_Create_From_Raw(cells, true, 0, 0)) != 0) {
        found = IMAGES_PALETTE;
    }
    Sprite_Delete(s);
    MT2D_SpriteRaw_Free(rgba);
    MT2D_SpriteRaw_Free(cells);
    return found;
}

/* Text cells behind an image area, so its transparent pixels show something. Text rows are 240/25 pixels high
   on SDL and 200/25 on VGA, the area is a bit bigger than the image to cover both. */
static void checker(int px, int py, int pw, int ph)
{
    int col, row;
    int c0 = px / 4, c1 = (px + pw) / 4 + 1;
    int r0 = py / 10, r1 = (py + ph) / 8 + 1;
    for (row = r0; row <= r1 && row < ROWS; row++)
        for (col = c0; col <= c1 && col < MAX_HOR; col++)
            WINDOW1[row][col] = (unsigned char)(((row + col) & 1) ? 176 : ' ');
}

static void header(int page, const char *title)
{
    static const char *names[PAGES] = {"Paint", "Clipping", "Read back", "Alpha", "ASCII"};
    int i, col = 1;
    for (i = 0; i < PAGES; i++) {
        text(0, col, i == page ? "[%d %s]" : " %d %s ", i + 1, names[i]);
        col += (int)strlen(names[i]) + 5;
    }
    text(1, 1, "%s", title);
    text(ROWS - 1, 1, "TAB next page   ESC quit   image sprites: %s",
         g_format == IMAGES_RGBA ? "RGBA (SDL)" : g_format == IMAGES_PALETTE ? "palette indexes (VGA / Sixel)" : "none in this build");
}

static bool no_images(void)
{
    if (g_format != IMAGES_NONE) return false;
    text(6, 4, "This build has no image sprites, MT2D_Sprite_Create_From_Raw(raw, true, ...)");
    text(7, 4, "returns NULL. Build the SDL version, or the MS-DOS VGA / Linux Sixel one.");
    text(9, 4, "Page 5 (ASCII) works everywhere.");
    return true;
}

/* ---------------------------------------------------------------------------------------------- 1 paint */

#define PAINT_X 24
#define PAINT_Y 40
#define PAINT_W 96
#define PAINT_H 64

static Sprite *g_paint, *g_cursor, *g_swatch;
static int g_cx = PAINT_W / 2, g_cy = PAINT_H / 2, g_color = 0, g_paint_result = -1;
static const char *g_paint_action = "";

static void paint_init(void)
{
    MT2D_SpriteRaw *blank = image_raw(PAINT_W, PAINT_H); /* all cells 0: a blank sprite */
    MT2D_SpriteRaw *cursor = image_raw(5, 5);
    int i;
    g_paint = image_from(blank);
    for (i = 0; i < 5; i++) { /* a hollow cross, the painted pixel stays visible in its center */
        if (i == 2) continue;
        put(cursor, i, 2, COLOR_COUNT - 1, 255);
        put(cursor, 2, i, COLOR_COUNT - 1, 255);
    }
    g_cursor = image_from(cursor);
    MT2D_SpriteRaw_Free(blank);
    MT2D_SpriteRaw_Free(cursor);
}

static void paint_swatch(void)
{
    MT2D_SpriteRaw *r = filled_raw(12, 12, g_color);
    Sprite_Delete(g_swatch);
    g_swatch = image_from(r);
    MT2D_SpriteRaw_Free(r);
}

static void paint_draw(void)
{
    header(0, "MT2D_Sprite_Create_From_Raw (blank 96x64) + MT2D_Sprite_Alter_Block");
    if (no_images()) return;
    checker(PAINT_X, PAINT_Y, PAINT_W, PAINT_H);
    if (!g_swatch) paint_swatch();
    Sprite_Render_on_Window(g_paint, DISPLAY_WINDOW1, PAINT_X, PAINT_Y);
    Sprite_Render_on_Window(g_cursor, DISPLAY_WINDOW1, PAINT_X + g_cx - 2, PAINT_Y + g_cy - 2);
    Sprite_Render_on_Window(g_swatch, DISPLAY_WINDOW1, 140, 40);
    text(4, 40, "color: %-11s", COLORS[g_color].name);
    text(6, 40, "arrows  move (%2d,%2d)", g_cx, g_cy);
    text(7, 40, "SPACE   paint a pixel (1x1 raw)");
    text(8, 40, "E       erase a pixel (transparent)");
    text(9, 40, "B       8x8 brush, clipped at the edges");
    text(10, 40, "C       next color");
    text(11, 40, "X       clear (a blank 96x64 raw)");
    if (g_paint_result >= 0) text(13, 40, "%s: %d cells written", g_paint_action, g_paint_result);
}

static void paint_key(int key)
{
    if (g_format == IMAGES_NONE) return;
    if (key == ' ') {
        g_paint_action = "pixel";
        g_paint_result = paint_block(g_paint, g_cx, g_cy, 1, 1, g_color);
    } else if (key == 'e' || key == 'E') {
        g_paint_action = "erase";
        g_paint_result = paint_block(g_paint, g_cx, g_cy, 1, 1, TRANSPARENT);
    } else if (key == 'b' || key == 'B') {
        g_paint_action = "brush";
        g_paint_result = paint_block(g_paint, g_cx - 4, g_cy - 4, 8, 8, g_color);
    } else if (key == 'c' || key == 'C') {
        g_color = (g_color + 1) % COLOR_COUNT;
        paint_swatch();
    } else if (key == 'x' || key == 'X') {
        g_paint_action = "clear";
        g_paint_result = paint_block(g_paint, 0, 0, PAINT_W, PAINT_H, TRANSPARENT);
    }
}

/* ---------------------------------------------------------------------------------------------- 2 clipping */

#define CLIP_X 48
#define CLIP_Y 56
#define CLIP_W 64
#define CLIP_H 48
#define STAMP 24

static Sprite *g_clip, *g_stamp_view;
static MT2D_SpriteRaw *g_clip_base, *g_stamp;
static int g_clip_tick = 0;
static bool g_clip_paused = false;

static void clip_init(void)
{
    int x, y;
    g_clip_base = image_raw(CLIP_W, CLIP_H);
    for (y = 0; y < CLIP_H; y++)
        for (x = 0; x < CLIP_W; x++) put(g_clip_base, x, y, ((x / 8 + y / 8) & 1) ? 10 : 2, 255);
    g_clip = image_from(g_clip_base);
    g_stamp = image_raw(STAMP, STAMP);
    for (y = 0; y < STAMP; y++)
        for (x = 0; x < STAMP; x++) {
            bool border = x == 0 || y == 0 || x == STAMP - 1 || y == STAMP - 1;
            /* the stamp has a hole in its middle: transparent cells erase the sprite under them */
            bool hole = x >= 8 && x < 16 && y >= 8 && y < 16;
            put(g_stamp, x, y, hole ? TRANSPARENT : border ? 3 : 0, 255);
        }
    g_stamp_view = image_from(g_stamp);
}

static void clip_draw(void)
{
    int sx, sy, written, t;
    header(1, "MT2D_Sprite_Alter_Block clips what falls outside of the sprite");
    if (no_images()) return;
    /* a looping curve: the stamp is fully outside, crosses every edge and corner, and is fully inside */
    t = g_clip_tick;
    sx = (CLIP_W - STAMP) / 2 + (int)(44.0 * sin(t * 0.031));
    sy = (CLIP_H - STAMP) / 2 + (int)(36.0 * sin(t * 0.047 + 1.0));
    if (!g_clip_paused) g_clip_tick++;

    MT2D_Sprite_Alter_Block(g_clip, 0, 0, g_clip_base); /* back to the checker board */
    written = MT2D_Sprite_Alter_Block(g_clip, sx, sy, g_stamp);

    checker(CLIP_X, CLIP_Y, CLIP_W, CLIP_H);
    Sprite_Render_on_Window(g_clip, DISPLAY_WINDOW1, CLIP_X, CLIP_Y);
    Sprite_Render_on_Window(g_stamp_view, DISPLAY_WINDOW1, 280, CLIP_Y);
    text(4, 34, "right: the stamp, 24x24, its middle");
    text(5, 34, "is transparent so it erases");
    text(10, 34, "Alter_Block(sprite, %3d, %3d, stamp)", sx, sy);
    text(11, 34, "written: %3d of %d cells   ", written, STAMP * STAMP);
    text(13, 34, "0 when it misses the sprite,");
    text(14, 34, "%d when it is fully inside.", STAMP * STAMP);
    text(16, 34, "SPACE pause, arrows step: %s", g_clip_paused ? "paused " : "running");
}

static void clip_key(int key)
{
    if (key == ' ') g_clip_paused = !g_clip_paused;
}

/* the arrow codes are also letters (72 = 'H'...), they only come here after the 224 prefix */
static void clip_step(int arrow)
{
    g_clip_paused = true;
    if (arrow == arrow_right_pressed || arrow == arrow_down_pressed) g_clip_tick++;
    else if (g_clip_tick > 0) g_clip_tick--;
}

/* ---------------------------------------------------------------------------------------------- 3 read back */

#define SHEET_X 24
#define SHEET_Y 48
#define TILE 16

static Sprite *g_sheet, *g_tiles[4], *g_shrunk;
static int g_shrunk_w, g_shrunk_h;
static double g_read_ms = -1;
static int g_reads = 0;

static void sheet_init(void)
{
    MT2D_SpriteRaw *r = image_raw(TILE * 4, TILE);
    int x, y;
    for (y = 0; y < TILE; y++)
        for (x = 0; x < TILE; x++) {
            put(r, x, y, (x / 4 + y / 4) & 1 ? 0 : 11, 255);                           /* checker */
            put(r, TILE + x, y, x == y || x == TILE - 1 - y ? 3 : TRANSPARENT, 255);      /* X */
            put(r, 2 * TILE + x, y, (x - 8) * (x - 8) + (y - 8) * (y - 8) < 40 ? 4 : TRANSPARENT, 255); /* ball */
            put(r, 3 * TILE + x, y, x < y ? 5 : 6, 255);                               /* two triangles */
        }
    g_sheet = image_from(r);
    MT2D_SpriteRaw_Free(r);
}

/* Reads the sheet back (the heavy call) and cuts it again */
static void sheet_read(bool mirror)
{
    clock_t start = clock();
    MT2D_SpriteRaw *all = MT2D_Sprite_To_SpriteRaw(g_sheet), *part;
    int i, x, y;
    g_read_ms = (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC;
    g_reads++;
    if (!all) return;
    if (mirror) {
        /* round trip: edit the raw that was read and write it back */
        size_t cell = all->IsRgba ? 4 : 1;
        unsigned char tmp[4];
        for (y = 0; y < all->Height; y++)
            for (x = 0; x < all->Width / 2; x++) {
                unsigned char *a = all->Data.Cells + ((size_t)y * all->Width + x) * cell;
                unsigned char *b = all->Data.Cells + ((size_t)y * all->Width + (all->Width - 1 - x)) * cell;
                memcpy(tmp, a, cell);
                memcpy(a, b, cell);
                memcpy(b, tmp, cell);
            }
        MT2D_Sprite_Alter_Block(g_sheet, 0, 0, all);
    }
    for (i = 0; i < 4; i++) {
        part = MT2D_SpriteRaw_Clip(all, i * TILE, 0, TILE, TILE);
        Sprite_Delete(g_tiles[i]);
        g_tiles[i] = image_from(part);
        MT2D_SpriteRaw_Free(part);
    }
    /* asks for 32x32 at (56,8): only 8x8 of it is inside the 64x16 sheet */
    part = MT2D_SpriteRaw_Clip(all, 56, 8, 32, 32);
    Sprite_Delete(g_shrunk);
    g_shrunk = image_from(part);
    g_shrunk_w = part ? part->Width : 0;
    g_shrunk_h = part ? part->Height : 0;
    MT2D_SpriteRaw_Free(part);
    MT2D_SpriteRaw_Free(all);
}

static void sheet_draw(void)
{
    int i;
    header(2, "MT2D_Sprite_To_SpriteRaw + MT2D_SpriteRaw_Clip");
    if (no_images()) return;
    if (g_reads == 0) sheet_read(false);
    checker(SHEET_X - 4, SHEET_Y - 4, 140, 110);
    Sprite_Render_on_Window(g_sheet, DISPLAY_WINDOW1, SHEET_X, SHEET_Y);
    for (i = 0; i < 4; i++) Sprite_Render_on_Window(g_tiles[i], DISPLAY_WINDOW1, SHEET_X + i * (TILE + 12), SHEET_Y + 40);
    Sprite_Render_on_Window(g_shrunk, DISPLAY_WINDOW1, SHEET_X, SHEET_Y + 84);
    text(4, 42, "top: the sheet sprite (64x16)");
    text(6, 42, "middle: 4 sprites made from");
    text(7, 42, "  Clip(raw, n*16, 0, 16, 16)");
    text(9, 42, "bottom: Clip(raw, 56, 8, 32, 32)");
    text(10, 42, "  shrinks to the overlap: %dx%d", g_shrunk_w, g_shrunk_h);
    text(12, 42, "To_SpriteRaw took %.2f ms (%d reads)", g_read_ms, g_reads);
    text(13, 42, "  heavy on SDL: GPU read back");
    text(15, 42, "R  read the sheet back again");
    text(16, 42, "M  read, mirror the raw, write it");
    text(17, 42, "   back (round trip), cut again");
}

static void sheet_key(int key)
{
    if (g_format == IMAGES_NONE) return;
    if (key == 'r' || key == 'R') sheet_read(false);
    if (key == 'm' || key == 'M') sheet_read(true);
}

/* ---------------------------------------------------------------------------------------------- 4 alpha */

#define ALPHA_X 24
#define ALPHA_Y 48
#define ALPHA_W 128
#define ALPHA_H 48

static Sprite *g_alpha, *g_palette_target;
static Sprite *g_refused_create;
static int g_refused_alter = -1;

static void alpha_init(void)
{
    MT2D_SpriteRaw *rgba = MT2D_SpriteRaw_Create(ALPHA_W, ALPHA_H, true);
    int x, y;
    /* 4 color bands, alpha 0 on the left to 255 on the right */
    for (y = 0; y < ALPHA_H; y++)
        for (x = 0; x < ALPHA_W; x++) {
            MT2D_RGBA *p = &rgba->Data.Rgba[y * ALPHA_W + x];
            const Color &c = COLORS[(y / 12) == 0 ? 0 : (y / 12) == 1 ? 1 : (y / 12) == 2 ? 2 : 11];
            p->R = c.r;
            p->G = c.g;
            p->B = c.b;
            p->A = (unsigned char)(x * 255 / (ALPHA_W - 1));
        }
    if (g_format == IMAGES_RGBA) {
        g_alpha = MT2D_Sprite_Create_From_Raw(rgba, true, 0, 0);
    } else if (g_format == IMAGES_PALETTE) {
        /* palette image sprites only take 1 byte cells: both calls refuse the RGBA raw */
        MT2D_SpriteRaw *cells = filled_raw(8, 8, 1);
        g_refused_create = MT2D_Sprite_Create_From_Raw(rgba, true, 0, 0);
        g_palette_target = image_from(cells);
        g_refused_alter = MT2D_Sprite_Alter_Block(g_palette_target, 0, 0, rgba);
        MT2D_SpriteRaw_Free(cells);
    }
    MT2D_SpriteRaw_Free(rgba);
}

static void alpha_draw(void)
{
    int row, col;
    header(3, "RGBA cells: MT2D_RGBA { R, G, B, A }");
    if (no_images()) return;
    if (g_format == IMAGES_PALETTE) {
        text(5, 4, "Image sprites of this build take palette indexes (1 byte cells).");
        text(6, 4, "An RGBA raw is refused:");
        text(8, 6, "MT2D_Sprite_Create_From_Raw(rgba raw, true, 0, 0) = %s",
             g_refused_create ? "a sprite (unexpected!)" : "NULL");
        text(9, 6, "MT2D_Sprite_Alter_Block(palette sprite, 0, 0, rgba raw) = %d", g_refused_alter);
        text(11, 4, "There is no semi transparency here: index 0 is transparent,");
        text(12, 4, "the other indexes are opaque. Run the SDL build to see this page.");
        return;
    }
    /* text behind the gradient: it shows through where alpha is low */
    for (row = 4; row <= 12; row++)
        for (col = 4; col < 42; col++) WINDOW1[row][col] = (unsigned char)("MT2D "[(row + col) % 5]);
    Sprite_Render_on_Window(g_alpha, DISPLAY_WINDOW1, ALPHA_X, ALPHA_Y);
    text(4, 44, "alpha 0 (left) to 255 (right)");
    text(6, 44, "red, green, blue and white bands,");
    text(7, 44, "the text behind shows through");
    text(8, 44, "where A is low.");
    text(10, 44, "SDL image sprites take RGBA raws");
    text(11, 44, "only (1 byte cells are refused).");
}

/* ---------------------------------------------------------------------------------------------- 5 ASCII */

#define TXT_COL 4
#define TXT_ROW 5
#define TXT_W 30
#define TXT_H 10

static Sprite *g_txt, *g_txt_copy, *g_txt_zoom;
static int g_tx = 0, g_ty = 0, g_txt_result = -1, g_blink = 0;
static const char *g_txt_action = "";

static void txt_init(void)
{
    MT2D_SpriteRaw *blank = MT2D_SpriteRaw_Create(TXT_W, TXT_H, false);
    const char *hello = "Type here!";
    memcpy(blank->Data.Cells + 1 * TXT_W + 2, hello, strlen(hello));
    g_txt = MT2D_Sprite_Create_From_Raw(blank, false, 0, 0);
    MT2D_SpriteRaw_Free(blank);
}

static void txt_draw(void)
{
    MT2D_SpriteRaw *all, *zoom;
    int row, col;
    header(4, "The same calls on an ASCII sprite (1 byte cells = characters)");
    /* dots behind the sprite: transparent cells (0) show them */
    for (row = TXT_ROW - 1; row <= TXT_ROW + TXT_H; row++)
        for (col = TXT_COL - 1; col <= TXT_COL + TXT_W; col++) WINDOW1[row][col] = '.';
    Sprite_Render_on_Window(g_txt, DISPLAY_WINDOW1, TXT_COL, TXT_ROW);
    if ((g_blink++ / 8) & 1) WINDOW1[TXT_ROW + g_ty][TXT_COL + g_tx] = 219; /* cursor */

    /* the whole sprite read back and a 10x4 clip around the cursor, every frame (cheap for ASCII) */
    all = MT2D_Sprite_To_SpriteRaw(g_txt);
    zoom = MT2D_SpriteRaw_Clip(all, g_tx - 5, g_ty - 2, 10, 4);
    Sprite_Delete(g_txt_copy);
    Sprite_Delete(g_txt_zoom);
    g_txt_copy = MT2D_Sprite_Create_From_Raw(all, false, 0, 0);
    g_txt_zoom = MT2D_Sprite_Create_From_Raw(zoom, false, 0, 0);
    text(3, 40, "To_SpriteRaw + Create_From_Raw:");
    Sprite_Render_on_Window(g_txt_copy, DISPLAY_WINDOW1, 40, 4);
    text(15, 40, "Clip(raw, cursor-5, cursor-2, 10, 4)");
    text(16, 40, "= %dx%d (shrinks at the edges):", zoom ? zoom->Width : 0, zoom ? zoom->Height : 0);
    Sprite_Render_on_Window(g_txt_zoom, DISPLAY_WINDOW1, 50, 17);
    MT2D_SpriteRaw_Free(zoom);
    MT2D_SpriteRaw_Free(all);

    text(TXT_ROW + TXT_H + 2, 2, "arrows move (%2d,%d)  letters: 1x1 raw", g_tx, g_ty);
    text(TXT_ROW + TXT_H + 3, 2, "BACKSPACE erase  ENTER stamp a 7x3 box");
    if (g_txt_result >= 0) text(TXT_ROW + TXT_H + 5, 2, "%s: %d cells written   ", g_txt_action, g_txt_result);
}

static int txt_write(int x, int y, int w, int h, const char *cells)
{
    MT2D_SpriteRaw *r = MT2D_SpriteRaw_Create(w, h, false);
    int written;
    memcpy(r->Data.Cells, cells, (size_t)(w * h));
    written = MT2D_Sprite_Alter_Block(g_txt, x, y, r);
    MT2D_SpriteRaw_Free(r);
    return written;
}

static void txt_key(int key)
{
    if (key == back_space_pressed || key == 127) {
        if (g_tx > 0) g_tx--;
        g_txt_action = "erase";
        g_txt_result = txt_write(g_tx, g_ty, 1, 1, "\0");
    } else if (key == enter_pressed || key == '\n') {
        /* 218 191 192 217 196 179: CP850 box corners and lines */
        static const char box[] = "\xDA\xC4\xC4\xC4\xC4\xC4\xBF"
                                  "\xB3 MT2D\xB3"
                                  "\xC0\xC4\xC4\xC4\xC4\xC4\xD9";
        g_txt_action = "box (clipped)";
        g_txt_result = txt_write(g_tx - 3, g_ty - 1, 7, 3, box);
    } else if (key >= 32 && key < 127) {
        char c = (char)key;
        g_txt_action = "char";
        g_txt_result = txt_write(g_tx, g_ty, 1, 1, &c);
        if (g_tx < TXT_W - 1) g_tx++;
    }
}

/* ---------------------------------------------------------------------------------------------- main */

static void move_cursor(int page, int arrow)
{
    int dx = arrow == arrow_left_pressed ? -1 : arrow == arrow_right_pressed ? 1 : 0;
    int dy = arrow == arrow_up_pressed ? -1 : arrow == arrow_down_pressed ? 1 : 0;
    if (page == 0) {
        g_cx += dx * 2;
        g_cy += dy * 2;
        if (g_cx < 0) g_cx = 0;
        if (g_cy < 0) g_cy = 0;
        if (g_cx >= PAINT_W) g_cx = PAINT_W - 1;
        if (g_cy >= PAINT_H) g_cy = PAINT_H - 1;
    } else if (page == 1) {
        clip_step(arrow);
    } else if (page == 4) {
        g_tx += dx;
        g_ty += dy;
        if (g_tx < 0) g_tx = 0;
        if (g_ty < 0) g_ty = 0;
        if (g_tx >= TXT_W) g_tx = TXT_W - 1;
        if (g_ty >= TXT_H) g_ty = TXT_H - 1;
    }
}

int main(int argc, char **argv)
{
    int page = 0, key, i;

    (void)argc;
    (void)argv;
    MT2D_Init();
    g_format = detect_image_format();
    if (g_format != IMAGES_NONE) {
        paint_init();
        clip_init();
        sheet_init();
    }
    alpha_init();
    txt_init();

    for (;;) {
        MT2D_Clear_Main_Window(); /* also clears the images of the last frame */
        if (page == 0) paint_draw();
        else if (page == 1) clip_draw();
        else if (page == 2) sheet_draw();
        else if (page == 3) alpha_draw();
        else txt_draw();
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        MT2D_System_Delay(30);

        while (MT2D_Keyboard_touched()) {
            key = MT2D_Keyboard_keytouched();
            if (key == escape_pressed) goto quit;
            if (key == tab_pressed) {
                page = (page + 1) % PAGES;
            } else if (key == arrow_key_pressed) {
                move_cursor(page, MT2D_Keyboard_keytouched());
            } else if (page == 0) {
                paint_key(key);
            } else if (page == 1) {
                clip_key(key);
            } else if (page == 2) {
                sheet_key(key);
            } else if (page == 4) {
                txt_key(key);
            }
        }
    }
quit:
    Sprite_Delete(g_paint);
    Sprite_Delete(g_cursor);
    Sprite_Delete(g_swatch);
    Sprite_Delete(g_clip);
    Sprite_Delete(g_stamp_view);
    MT2D_SpriteRaw_Free(g_clip_base);
    MT2D_SpriteRaw_Free(g_stamp);
    Sprite_Delete(g_sheet);
    for (i = 0; i < 4; i++) Sprite_Delete(g_tiles[i]);
    Sprite_Delete(g_shrunk);
    Sprite_Delete(g_alpha);
    Sprite_Delete(g_palette_target);
    Sprite_Delete(g_refused_create);
    Sprite_Delete(g_txt);
    Sprite_Delete(g_txt_copy);
    Sprite_Delete(g_txt_zoom);
    return 0;
}

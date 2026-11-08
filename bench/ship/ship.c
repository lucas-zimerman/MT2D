/* Benchmark: the main loop of examples/graphics (the spaceship game) on the MS-DOS VGA renderer, without the 30 ms
   delay, the beep and the keyboard: the ship follows a fixed path, so every build draws the same frames.
   It prints the time of a frame, the allocation calls, the heap growth (sbrk) and a checksum of the screen.
   Build: see bench/README.md. Only for DJGPP with MSDOS_USE_GRAPHICS. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/nearptr.h>
#include "example.h"
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>
#include <MT2D/_MSDOS/Render/DosImg.h>
#include <MT2D/_MSDOS/Render/font.h>
extern unsigned char VGA_ASCII_COLORS[2];

#define SCREEN_W 320
#define SCREEN_H 240
#define COINS 5
#define FRAMES 400

static unsigned long g_malloc = 0, g_realloc = 0;
void *__real_malloc(size_t n);
void *__real_realloc(void *p, size_t n);
void *__wrap_malloc(size_t n) { g_malloc++; return __real_malloc(n); }
void *__wrap_realloc(void *p, size_t n) { g_realloc++; return __real_realloc(p, n); }

/* Reference painter: every sprite drawn in the last frame is logged, then painted pixel by pixel over the text. */
typedef struct { const unsigned char *pix; int w, h, x, y; } LogSprite;
static LogSprite slog[64];
static int slog_n = 0;
static unsigned char refbuf[64000];
static void RENDER(Sprite *sp, int x, int y)
{
    if (slog_n < 64) { slog[slog_n].pix = sp->Image->Pixels; slog[slog_n].w = sp->SizeX; slog[slog_n].h = sp->SizeY; slog[slog_n].x = x; slog[slog_n].y = y; slog_n++; }
    Sprite_Render_on_Window(sp, DISPLAY_WINDOW1, x, y);
}
static long verify(const unsigned char *screen)
{
    int tx, ty, i, r, c, k, x, y;
    long bad = 0;
    for (ty = 0; ty < 25; ty++) for (tx = 0; tx < 80; tx++) {
        unsigned char ch = WINDOW1[ty][tx];
        unsigned off = (ch & 31) + ((ch >> 5) << 7);
        for (i = 0; i < 4; i++) {
            unsigned char b = VGAFONT[off + 32 * i];
            for (r = 0; r < 2; r++) for (c = 0; c < 4; c++)
                refbuf[(ty * 8 + i * 2 + r) * 320 + tx * 4 + c] = VGA_ASCII_COLORS[(b >> (r == 0 ? 7 - c : 3 - c)) & 1];
        }
    }
    for (k = 0; k < slog_n; k++)
        for (y = 0; y < slog[k].h; y++) for (x = 0; x < slog[k].w; x++) {
            int sx = slog[k].x + x, sy = slog[k].y + y;
            unsigned char v = slog[k].pix[y * slog[k].w + x];
            if (v && sx >= 0 && sx < 320 && sy >= 0 && sy < 200) refbuf[sy * 320 + sx] = v;
        }
    for (i = 0; i < 64000; i++) if (refbuf[i] != screen[i]) bad++;
    return bad;
}

static Sprite *load(const char *name)
{
    Sprite *s = Load_Sprite_Image((char *)name, 0, 0);
    if (s) { s->ScaleX = s->SizeX; s->ScaleY = s->SizeY; }
    return s;
}

static void scroll_layer(Sprite *s, int offset)
{
    int x = -(offset % SCREEN_W);
    RENDER(s, x, 0);
    RENDER(s, x + SCREEN_W, 0);
}

int main(void)
{
    Sprite *far_stars, *near_stars, *ship, *red, *blue, *rock, *coin[4];
    int ship_x = 20, ship_y = 110, coin_x[COINS], coin_y[COINS];
    int score = 0, tick = 0, i;
    unsigned long heap0 = (unsigned long)sbrk(0), calls0, h = 2166136261UL;
    unsigned char *screen;
    double t0, t1;
    long bad;
    char text[40];

    MT2D_Init();
    far_stars = load("stars_far.png"); near_stars = load("stars_near.png"); ship = load("ship.png");
    red = load("planet_red.png"); blue = load("planet_blue.png"); rock = load("rock.png");
    for (i = 0; i < 4; i++) { char name[16]; sprintf(name, "coin%d.png", i); coin[i] = load(name); }
    if (!far_stars || !ship || !coin[0]) { MT2D_Dos_VGA_Close(); printf("could not load the images\n"); return 1; }
    for (i = 0; i < COINS; i++) { coin_x[i] = 120 + i * 40; coin_y[i] = 30 + (i * 53) % 170; }

    calls0 = g_malloc + g_realloc;
    t0 = (double)uclock() * 1000.0 / UCLOCKS_PER_SEC;
    for (tick = 0; tick < FRAMES; ) {
        slog_n = 0;
        MT2D_Clear_Main_Window();
        scroll_layer(far_stars, tick / 3);
        scroll_layer(near_stars, tick);
        RENDER(red, SCREEN_W - (tick / 2) % (SCREEN_W + 120), 30);
        RENDER(blue, SCREEN_W - (tick / 2 + 150) % (SCREEN_W + 120), 160);
        RENDER(rock, SCREEN_W - (tick * 2) % (SCREEN_W + 60), 100 + (tick / 40 % 5) * 10);
        for (i = 0; i < COINS; i++)
            RENDER(coin[(tick / 4 + i) % 4], coin_x[i], coin_y[i]);
        RENDER(ship, ship_x, ship_y);
        sprintf(text, " SCORE %03d ", score);
        insert_string_on_display(text, 0, 1, DISPLAY_WINDOW1);
        insert_string_on_display((char *)" arrows: move   ESC: quit ", 23, 27, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);

        /* scripted ship: it sweeps the screen like a player would */
        ship_x = 20 + (tick * 2) % 260;
        ship_y = 30 + (tick * 3) % 140;
        for (i = 0; i < COINS; i++) {
            coin_x[i] -= 1;
            if (coin_x[i] < -16) { coin_x[i] = SCREEN_W + (i * 37) % 60; coin_y[i] = 20 + (tick * 7 + i * 41) % 190; }
            if (ship_x < coin_x[i] + 16 && ship_x + 32 > coin_x[i] && ship_y < coin_y[i] + 16 && ship_y + 16 > coin_y[i]) {
                score++; coin_x[i] = SCREEN_W + 20; coin_y[i] = 20 + (tick * 13 + i * 29) % 190;
            }
        }
        tick++;
    }
    t1 = (double)uclock() * 1000.0 / UCLOCKS_PER_SEC;
    screen = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
    for (i = 0; i < 64000; i++) { h ^= screen[i]; h *= 16777619UL; }
    bad = verify(screen);
    MT2D_Dos_VGA_Close();
    printf("RESULT ship ms/frame=%8.3f  sum=%08lX  badpixels=%ld  score=%d  alloc/frame=%.1f\n", (t1 - t0) / FRAMES, h, bad, score,
           (double)((g_malloc + g_realloc) - calls0) / FRAMES);
    printf("RESULT HEAP heap_growth_bytes=%lu\n", (unsigned long)sbrk(0) - heap0);
    printf("DONE\n");
    return 0;
}

/*
   DiscoPallete: animation made only with the palette (VGA mode, build with -DMT2D_DOS_VGA=ON).

   A spiral is written once to the video memory, using all the 256 colors (index = distance + angle).
   After that nothing is drawn again: every frame we send a whole new palette to the VGA with
   MT2D_SET_PALETTE, { op, 0xRRGGBB x 256 }, and the colors move through the spiral.

   SPACE changes the effect (it also changes by itself), ESC quits.
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pc.h>
#include "example.h"
#include <MT2D/MT2D_Settings.h>
#include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>

#define SCREEN_W 320
#define SCREEN_H 200
#define ARMS 3                 /* spiral arms, must be an integer so the angle wraps without a seam */
#define EFFECTS 4
#define FRAMES_PER_EFFECT 360
#define DISCO_PI 3.14159265358979

extern unsigned char *VGA;     /* the video memory, set by MT2D_Init in VGA mode */

/* hue 0..1, saturation 0..1, value 0..1 -> 0xRRGGBB */
static int hsv(double h, double s, double v)
{
    double r = 0, g = 0, b = 0, f;
    int i, p, q, t;
    h = (h - floor(h)) * 6.0;
    i = (int)h;
    f = h - i;
    p = (int)(255 * v * (1 - s));
    q = (int)(255 * v * (1 - s * f));
    t = (int)(255 * v * (1 - s * (1 - f)));
    {
        int vv = (int)(255 * v);
        switch (i) {
        case 0: r = vv; g = t;  b = p;  break;
        case 1: r = q;  g = vv; b = p;  break;
        case 2: r = p;  g = vv; b = t;  break;
        case 3: r = p;  g = q;  b = vv; break;
        case 4: r = t;  g = p;  b = vv; break;
        default: r = vv; g = p; b = q;  break;
        }
    }
    return ((int)r << 16) | ((int)g << 8) | (int)b;
}

static void draw_spiral(void)
{
    int x, y;
    for (y = 0; y < SCREEN_H; y++) {
        for (x = 0; x < SCREEN_W; x++) {
            double dx = x - SCREEN_W / 2;
            double dy = (y - SCREEN_H / 2) * 1.2; /* the pixels are not square (320x200 on a 4:3 screen) */
            double d = sqrt(dx * dx + dy * dy);
            double a = atan2(dy, dx);
            VGA[y * SCREEN_W + x] = (unsigned char)((int)(d * 1.5 + a * ARMS * 256.0 / (2 * DISCO_PI)) & 255);
        }
    }
}

/* Fills pal[1..256] (pal[0] is the operation) for the frame t of an effect. */
static void make_palette(int *pal, int effect, int t)
{
    int i;
    for (i = 0; i < 256; i++) {
        double u = i / 256.0;
        int color;
        switch (effect) {
        case 0: /* rainbow that turns */
            color = hsv(u - t / 128.0, 1.0, 1.0);
            break;
        case 1: /* the rainbow only shows in moving bands */
            color = hsv(u * 2 + t / 300.0, 1.0, 0.5 + 0.5 * sin(u * 2 * DISCO_PI * 6 - t * 0.2));
            break;
        case 2: { /* strobe: 8 bands change color on every beat, and fade between the beats */
            int beat = t / 15;
            double fade = 1.0 - (t % 15) / 15.0;
            color = hsv(((i / 32) * 5 + beat * 3) / 8.0, 1.0, 0.25 + 0.75 * fade);
            break;
        }
        default: { /* fire and ice: red and blue waves going opposite ways */
            double a = 0.5 + 0.5 * sin(u * 2 * DISCO_PI * 3 - t * 0.12);
            double b = 0.5 + 0.5 * sin(u * 2 * DISCO_PI * 3 + t * 0.12);
            color = ((int)(255 * a) << 16) | ((int)(120 * a * b) << 8) | (int)(255 * b);
            break;
        }
        }
        pal[1 + i] = color;
    }
}

static void wait_retrace(void)
{
    while (inportb(0x3DA) & 8) {}   /* wait for the end of a retrace, then for the next one */
    while (!(inportb(0x3DA) & 8)) {}
}

int main(int argc, char **argv)
{
    int pal[1 + 256];
    int effect = 0, t = 0, key;

    (void)argc;
    (void)argv;
    MT2D_Init();                    /* mode 13h */
    draw_spiral();
    pal[0] = MT2D_SET_PALETTE;

    for (;;) {
        if (MT2D_Keyboard_touched()) {
            key = MT2D_Keyboard_keytouched();
            if (key == escape_pressed) break;
            if (key == ' ') { effect = (effect + 1) % EFFECTS; t = 0; }
        }
        make_palette(pal, effect, t);
        wait_retrace();
        MT2D_Set_Settings_Argv(1 + 256, pal);
        if (++t >= FRAMES_PER_EFFECT) { effect = (effect + 1) % EFFECTS; t = 0; }
    }
    MT2D_Dos_VGA_Close();           /* back to the text mode */
    return 0;
}

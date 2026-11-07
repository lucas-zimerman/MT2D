/*
   ASCII graphics: three real time effects drawn straight into the WINDOW1 text buffer.
   1 = plasma, 2 = starfield, 3 = rotating sine tunnel/spiral, ESC or Q = quit.
*/
#include <math.h>
#include <stdlib.h>
#include "example.h"

#define ROWS 24

/* light to dark, MS-DOS block characters (the engine maps them on every platform) */
static const unsigned char ramp[] = { ' ', '.', ':', 176, 177, 178, 219 };
#define RAMP_SIZE 7

static unsigned char shade(double v) /* v in 0..1 */
{
    int i = (int)(v * (RAMP_SIZE - 1) + 0.5);
    if (i < 0) i = 0;
    if (i >= RAMP_SIZE) i = RAMP_SIZE - 1;
    return ramp[i];
}

static void plasma(double t)
{
    int x, y;
    for (y = 0; y < ROWS; y++) {
        for (x = 0; x < MAX_HOR; x++) {
            double v = sin(x * 0.18 + t) + sin(y * 0.35 - t * 1.3)
                     + sin((x * 0.12 + y * 0.25) + t * 0.7)
                     + sin(sqrt((x - 40.0) * (x - 40.0) / 4.0 + (y - 12.0) * (y - 12.0)) * 0.5 - t);
            WINDOW1[y][x] = shade((v + 4.0) / 8.0);
        }
    }
}

#define STARS 60
static double star_x[STARS], star_y[STARS], star_z[STARS];

static void stars_init(void)
{
    int i;
    for (i = 0; i < STARS; i++) {
        star_x[i] = (rand() % 2000 - 1000) / 100.0;
        star_y[i] = (rand() % 2000 - 1000) / 100.0;
        star_z[i] = 1.0 + (rand() % 1000) / 100.0;
    }
}

static void starfield(void)
{
    int i, x, y;
    for (y = 0; y < ROWS; y++)
        for (x = 0; x < MAX_HOR; x++)
            WINDOW1[y][x] = ' ';
    for (i = 0; i < STARS; i++) {
        star_z[i] -= 0.15;
        if (star_z[i] < 0.3) {
            star_x[i] = (rand() % 2000 - 1000) / 100.0;
            star_y[i] = (rand() % 2000 - 1000) / 100.0;
            star_z[i] = 10.0;
        }
        x = 40 + (int)(star_x[i] / star_z[i] * 12.0);
        y = 12 + (int)(star_y[i] / star_z[i] * 5.0);
        if (x >= 0 && x < MAX_HOR && y >= 0 && y < ROWS)
            WINDOW1[y][x] = star_z[i] < 3.0 ? 219 : (star_z[i] < 6.0 ? 'o' : '.');
    }
}

static void spiral(double t)
{
    int x, y;
    for (y = 0; y < ROWS; y++) {
        for (x = 0; x < MAX_HOR; x++) {
            double dx = (x - 40.0) / 2.0, dy = y - 12.0;
            double d = sqrt(dx * dx + dy * dy) + 0.001;
            double a = atan2(dy, dx);
            double v = sin(8.0 / d * 3.0 - t * 2.0 + a * 3.0);
            WINDOW1[y][x] = shade((v + 1.0) / 2.0 * (d > 1.5 ? 1.0 : d / 1.5));
        }
    }
}

int main(int argc, char **argv)
{
    int mode = 1;
    double t = 0.0;
    int key;

    (void)argc;
    (void)argv;
    MT2D_Init();
    stars_init();

    for (;;) {
        if (mode == 1) plasma(t);
        else if (mode == 2) starfield();
        else spiral(t);
        insert_string_on_display((char *)" 1 plasma  2 stars  3 spiral  ESC quit ", 0, 20, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        t += 0.12;
        MT2D_System_Delay(40);

        if (MT2D_Keyboard_touched()) {
            key = MT2D_Keyboard_keytouched();
            if (key == escape_pressed || key == 'q' || key == 'Q') break;
            if (key >= '1' && key <= '3') mode = key - '0';
        }
    }
    return 0;
}

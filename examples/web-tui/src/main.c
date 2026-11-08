/*
   Web TUI: a text mode MT2D game made to run in the browser (Emscripten + SDL2), with PC speaker effects.

   Catch the falling '$' with the paddle and dodge the 'X' bombs. Arrow keys or A / D move, Enter restarts.
   Only the bombs cost a life, a missed '$' is fine. On a phone use the buttons under the screen (common/web_shell.html).

   What is different from the desktop examples:
   * A browser tab cannot run a "for (;;)" loop: it would freeze the page. The game is a function
     called once per frame (frame) and emscripten_set_main_loop calls it at the screen refresh rate.
   * Native browser features are reached with EM_ASM: the best score is kept in localStorage and the
     score is shown in the page title.
   * MT2D_System_Beep is the PC speaker: the browser plays it as a square-ish tone made by the engine. On the web
     the call does not wait for the tone to end (a tab cannot block), the beeps are queued and play in order.
     Browsers stay silent until the page gets a click or a key press.
   The font is preloaded into the virtual file system by CMake (assets/msdos_fnt.png).
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <emscripten.h>
#include "example.h"

#define COLS 80
#define FIELD_TOP 2
#define PADDLE_ROW 23
#define PADDLE_W 13
#define MAX_ITEMS 12
#define START_LIVES 5
#define SLOWEST_FALL 6 /* the game logic runs every TICK_FRAMES frames, items fall every 6 logic ticks at the start... */
#define FASTEST_FALL 3 /* ...and every 3 when the score is high */
#define TICK_FRAMES 3

typedef struct {
    int active;
    int x, y;
    int bomb;
} Item;

static Item items[MAX_ITEMS];
static int paddle_x, score, best, lives, game_over, frames, ticks;

/* PC speaker effects (frequency in Hz, duration in ms) */
static void sfx_catch(void) { MT2D_System_Beep(1320, 40); }
static void sfx_bomb(void) { MT2D_System_Beep(220, 150); MT2D_System_Beep(110, 200); }
static void sfx_start(void) { MT2D_System_Beep(523, 80); MT2D_System_Beep(659, 80); MT2D_System_Beep(784, 120); }
static void sfx_game_over(void)
{
    MT2D_System_Beep(494, 160);
    MT2D_System_Beep(440, 160);
    MT2D_System_Beep(392, 160);
    MT2D_System_Beep(262, 400);
}

/* ---- browser side --------------------------------------------------------------------------- */

static int load_best(void)
{
    return EM_ASM_INT({
        try {
            return parseInt(window.localStorage.getItem('mt2d_best') || '0', 10) || 0;
        } catch (e) {
            return 0; /* storage blocked (private window) */
        }
    });
}

static void save_best(int value)
{
    EM_ASM({
        try {
            window.localStorage.setItem('mt2d_best', String($0));
        } catch (e) {
        }
    }, value);
}

static void set_page_title(void)
{
    EM_ASM({
        document.title = 'MT2D web - score ' + $0;
    }, score);
}

/* ---- game ----------------------------------------------------------------------------------- */

static void put_char(char c, int row, int col)
{
    char s[2];
    s[0] = c;
    s[1] = 0;
    insert_string_on_display(s, row, col, DISPLAY_WINDOW1);
}

static void new_game(void)
{
    memset(items, 0, sizeof(items));
    paddle_x = (COLS - PADDLE_W) / 2;
    score = 0;
    lives = START_LIVES;
    game_over = 0;
    ticks = 0;
    set_page_title();
}

static void spawn(void)
{
    int i;
    for (i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) {
            items[i].active = 1;
            items[i].x = 1 + rand() % (COLS - 2);
            items[i].y = FIELD_TOP;
            items[i].bomb = (rand() % 6) == 0;
            return;
        }
    }
}

static void read_keys(void)
{
    int key;
    while (MT2D_Keyboard_touched()) {
        key = MT2D_Keyboard_keytouched();
        if (key == arrow_key_pressed) {
            key = MT2D_Keyboard_keytouched();
            if (key == arrow_left_pressed) key = 'a';
            else if (key == arrow_right_pressed) key = 'd';
        }
        if (game_over) {
            if (key == enter_pressed) {
                new_game();
                sfx_start();
            }
        } else if (key == 'a' || key == 'A') {
            paddle_x -= 4;
        } else if (key == 'd' || key == 'D') {
            paddle_x += 4;
        }
    }
    if (paddle_x < 1) paddle_x = 1;
    if (paddle_x > COLS - 1 - PADDLE_W) paddle_x = COLS - 1 - PADDLE_W;
}

static void tick(void)
{
    int i, per_fall = SLOWEST_FALL - score / 15, fall;

    if (per_fall < FASTEST_FALL) per_fall = FASTEST_FALL;
    fall = (ticks % per_fall) == 0;

    ticks++;
    if (!fall) return;
    if (rand() % 4 == 0) spawn();
    for (i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) continue;
        items[i].y++;
        if (items[i].y < PADDLE_ROW) continue;
        /* reached the paddle row */
        if (items[i].x >= paddle_x && items[i].x < paddle_x + PADDLE_W) {
            if (items[i].bomb) {
                lives--;
                sfx_bomb();
            } else {
                score++;
                sfx_catch();
                set_page_title();
            }
        }
        items[i].active = 0;
    }
    if (lives <= 0) {
        game_over = 1;
        sfx_game_over();
        if (score > best) {
            best = score;
            save_best(best);
        }
    }
}

static void draw(void)
{
    char text[COLS + 1];
    int i;

    MT2D_Clear_Main_Window();
    sprintf(text, " SCORE %03d    BEST %03d    LIVES %d ", score, best, lives);
    insert_string_on_display(text, 0, 2, DISPLAY_WINDOW1);
    for (i = 0; i < COLS; i++) put_char((char)205, 1, i); /* double line */
    for (i = 0; i < MAX_ITEMS; i++)
        if (items[i].active) put_char(items[i].bomb ? 'X' : '$', items[i].y, items[i].x);
    insert_string_on_display((char *)"<===========>", PADDLE_ROW, paddle_x, DISPLAY_WINDOW1);
    insert_string_on_display((char *)" arrows / A D: move ", 24, 29, DISPLAY_WINDOW1);
    if (game_over) {
        insert_string_on_display((char *)"  GAME OVER  ", 10, 33, DISPLAY_WINDOW1);
        insert_string_on_display((char *)" Enter: play again ", 12, 30, DISPLAY_WINDOW1);
    }
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

/* Called by the browser once per frame. Must return quickly. */
static void frame(void)
{
    read_keys();
    if (!game_over && frames % TICK_FRAMES == 0) tick();
    frames++;
    draw();
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    srand((unsigned)time(NULL));
    MT2D_Init();
    best = load_best();
    new_game();
    sfx_start();
    /* 0 = use requestAnimationFrame, 1 = do not return from main (the loop keeps the program alive) */
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}

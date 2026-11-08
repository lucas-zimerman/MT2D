/*
   Web GUI: an image (PNG) MT2D game made to run in the browser (Emscripten + SDL2), with sound effects and music.

   Move the robot with the arrow keys (or A / D, or the buttons under the screen), catch the spinning coins and
   dodge the ghosts. Only the ghosts cost a life, a missed coin is fine. Enter restarts after the game over.

   * Images are drawn on a 320x240 virtual screen on top of the text screen (Sprite_Render_on_Window), the HUD is text.
   * The robot is moved with the joystick code (held keys = smooth movement), like examples/graphics.
   * MT2D_Play_Music loops the song, MT2D_Play_Audio plays a sound effect (SDL2_mixer). Browsers stay silent until the
     page gets a click or a key press, so the music starts at the first key.
   * A browser tab cannot run a "for (;;)" loop: the game is the function frame(), called once per screen refresh.
   * The images and sounds are the ones of the sprites, playsound and playmusic examples, CMake preloads them.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <emscripten.h>
#include "example.h"
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/Audio/MT2D_Audio_core.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define SIZE 24 /* every sprite is drawn 24x24 on the 320x240 screen (the images are 16x16) */
#define MAX_ITEMS 8
#define START_LIVES 5
#define SLOW 3 /* the game is 3 times slower than "1 pixel per frame": positions are kept in 1/SLOW pixels */
#define ROBOT_SPEED 4 /* 1/SLOW pixels per frame at full stick */

typedef struct {
    int active;
    int x, y; /* y in 1/SLOW pixels */
    int ghost;
} Item;

static Sprite *robot[2], *coin[4], *ghost[2];
static Item items[MAX_ITEMS];
static int robot_x, score, /* robot_x in 1/SLOW pixels */ best, lives, game_over, frame_count, music_started;

/* ---- browser side --------------------------------------------------------------------------- */

static int load_best(void)
{
    return EM_ASM_INT({
        try {
            return parseInt(window.localStorage.getItem('mt2d_gui_best') || '0', 10) || 0;
        } catch (e) {
            return 0; /* storage blocked (private window) */
        }
    });
}

static void save_best(int value)
{
    EM_ASM({
        try {
            window.localStorage.setItem('mt2d_gui_best', String($0));
        } catch (e) {
        }
    }, value);
}

static void set_page_title(void)
{
    EM_ASM({
        document.title = 'MT2D web GUI - score ' + $0;
    }, score);
}

/* ---- game ----------------------------------------------------------------------------------- */

static Sprite *load(const char *name)
{
    Sprite *s = Load_Sprite_Image((char *)name, SIZE, SIZE);
    if (!s) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"Could not load the images.", 12, 27, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        emscripten_cancel_main_loop();
    }
    return s;
}

static void new_game(void)
{
    memset(items, 0, sizeof(items));
    robot_x = (SCREEN_W - SIZE) / 2 * SLOW;
    score = 0;
    lives = START_LIVES;
    game_over = 0;
    set_page_title();
}

static void spawn(void)
{
    int i;
    for (i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) {
            items[i].active = 1;
            items[i].x = rand() % (SCREEN_W - SIZE);
            items[i].y = 12 * SLOW; /* under the HUD */
            items[i].ghost = (rand() % 5) == 0;
            return;
        }
    }
}

static int touching(int ax, int ay, int bx, int by)
{
    return ax < bx + SIZE && ax + SIZE > bx && ay < by + SIZE && ay + SIZE > by;
}

static void update(void)
{
    int i, axis, robot_y = SCREEN_H - SIZE - 4, fall = 1 + score / 20; /* 1/SLOW pixels per frame, a bit faster with the score */

    if (fall > 2) fall = 2;
    MT2D_Joystick_Update(); /* reads the window events and the held keys */
    if (game_over) {
        if (MT2D_Joystick_Button_Touched(1)) {
            new_game();
            MT2D_Play_Audio((char *)"powerup.wav");
        }
        return;
    }
    if (!music_started) {
        /* the first key or click unlocks the browser audio, the music starts then */
        if (MT2D_Joystick_Touched()) {
            music_started = 1;
            MT2D_Play_Music((char *)"ode.ogg");
        }
    }

    axis = MT2D_Joystick_Get_LeftAxisX();
    if (axis < -8000) robot_x -= ROBOT_SPEED;
    if (axis > 8000) robot_x += ROBOT_SPEED;
    if (robot_x < 0) robot_x = 0;
    if (robot_x > (SCREEN_W - SIZE) * SLOW) robot_x = (SCREEN_W - SIZE) * SLOW;

    if (frame_count % (50 * SLOW) == 0) spawn(); /* about one item every 3 seconds */
    for (i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) continue;
        items[i].y += fall;
        if (touching(robot_x / SLOW, robot_y, items[i].x, items[i].y / SLOW)) {
            items[i].active = 0;
            if (items[i].ghost) {
                lives--;
                MT2D_Play_Audio((char *)"explosion.ogg");
            } else {
                score++;
                MT2D_Play_Audio((char *)"coin.wav");
                set_page_title();
            }
        } else if (items[i].y / SLOW > SCREEN_H) {
            items[i].active = 0; /* missed, no penalty */
        }
    }
    if (lives <= 0) {
        game_over = 1;
        if (score > best) {
            best = score;
            save_best(best);
        }
    }
}

static void draw(void)
{
    char text[60];
    int i;

    MT2D_Clear_Main_Window(); /* also clears the images of the last frame */
    for (i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) continue;
        if (items[i].ghost)
            Sprite_Render_on_Window(ghost[(frame_count / (12 * SLOW)) % 2], DISPLAY_WINDOW1, items[i].x, items[i].y / SLOW);
        else
            Sprite_Render_on_Window(coin[(frame_count / (6 * SLOW) + i) % 4], DISPLAY_WINDOW1, items[i].x, items[i].y / SLOW);
    }
    Sprite_Render_on_Window(robot[(frame_count / (10 * SLOW)) % 2], DISPLAY_WINDOW1, robot_x / SLOW, SCREEN_H - SIZE - 4);

    sprintf(text, " SCORE %03d    BEST %03d    LIVES %d ", score, best, lives);
    insert_string_on_display(text, 0, 2, DISPLAY_WINDOW1);
    if (!music_started)
        insert_string_on_display((char *)" press a key to start the music ", 24, 24, DISPLAY_WINDOW1);
    if (game_over) {
        insert_string_on_display((char *)"  GAME OVER  ", 10, 33, DISPLAY_WINDOW1);
        insert_string_on_display((char *)" Enter: play again ", 12, 30, DISPLAY_WINDOW1);
    }
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

/* Called by the browser once per frame. Must return quickly. */
static void frame(void)
{
    update();
    frame_count++;
    draw();
}

int main(int argc, char **argv)
{
    int i;
    char name[16];

    (void)argc;
    (void)argv;
    srand((unsigned)time(NULL));
    MT2D_Init();
    MT2D_Joystick_Init();
    MT2D_Joystick_Map_Button01(enter_pressed); /* button 1 = the Enter key */
    MT2D_Audio_Init();

    robot[0] = load("robot0.png");
    robot[1] = load("robot1.png");
    ghost[0] = load("ghost0.png");
    ghost[1] = load("ghost1.png");
    for (i = 0; i < 4; i++) {
        sprintf(name, "coin%d.png", i);
        coin[i] = load(name);
    }
    if (!robot[0] || !robot[1] || !ghost[0] || !ghost[1] || !coin[0] || !coin[1] || !coin[2] || !coin[3]) return 1;

    best = load_best();
    new_game();
    /* 0 = use requestAnimationFrame, 1 = do not return from main (the loop keeps the program alive) */
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}

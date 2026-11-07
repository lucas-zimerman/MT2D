/*
   Sprites: animated image sprites (PNG).
   Move the robot with the arrow keys, grab the spinning coins and avoid the ghosts. ESC quits.

   * Load_Sprite_Image loads a PNG (cyan = transparent). ScaleX/ScaleY is the size on the 320x240 screen.
   * An ObjectState is a list of frames (sprite + how many ticks to show it), an Object plays it.
   * ObjectScene_Goto_NextSteps advances every animation, ObjectScene_Render draws them.
   * ASCII sprites (Sprite_Create_Ascii_From_Cells) work the same way, the HUD below is plain text.
   The images are made by tools/make_assets.py.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "example.h"
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Object.h> /* keep this one before State.h (they include each other) */
#include <MT2D/ObjectCore/State.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define SIZE 16 /* every sprite is 16x16 pixels */
#define COINS 4
#define GHOSTS 3

static Sprite *load(const char *name)
{
    Sprite *s = Load_Sprite_Image((char *)name, SIZE, SIZE);
    if (!s) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"Could not load the images, run the example from its bin folder.", 12, 8, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        MT2D_Keyboard_keytouched();
        exit(1);
    }
    return s;
}

/* An object with a single looping animation. */
static Object *make_animated(ObjectState *state, int x, int y)
{
    ObjectState **states = (ObjectState **)malloc(sizeof(ObjectState *));
    states[0] = state;
    return Object_Create(false, false, SIZE, SIZE, x, y, states, 1);
}

static bool touching(int ax, int ay, int bx, int by)
{
    return ax < bx + SIZE && ax + SIZE > bx && ay < by + SIZE && ay + SIZE > by;
}

int main(int argc, char **argv)
{
    Sprite *robot[2], *coin_s[4], *ghost_s[2];
    ObjectState *coin_state, *ghost_state;
    ObjectScene *scene;
    Object *coins[COINS], *ghosts[GHOSTS];
    int px = SCREEN_W / 2, py = SCREEN_H / 2, steps = 0, score = 0, tick = 0, i, key;
    char name[16], text[40];

    (void)argc;
    (void)argv;
    MT2D_Init();

    robot[0] = load("robot0.png");
    robot[1] = load("robot1.png");
    ghost_s[0] = load("ghost0.png");
    ghost_s[1] = load("ghost1.png");
    for (i = 0; i < 4; i++) {
        sprintf(name, "coin%d.png", i);
        coin_s[i] = load(name);
    }

    /* the states are shared by all the objects that use them */
    coin_state = MT2D_OBJECT_CREATE_STATE((char *)"Spin");
    for (i = 0; i < 4; i++)
        MT2D_ObjectState_ADD(coin_state, coin_s[i], 3, 0);
    ghost_state = MT2D_OBJECT_CREATE_STATE((char *)"Float");
    MT2D_ObjectState_ADD(ghost_state, ghost_s[0], 6, 0);
    MT2D_ObjectState_ADD(ghost_state, ghost_s[1], 6, 0);

    scene = ObjectScene_CreateEmpty();
    for (i = 0; i < COINS; i++) {
        coins[i] = make_animated(coin_state, 40 + i * 70, 30 + (i * 53) % 160);
        coins[i]->ActualFrame = i % 4; /* start each coin on another frame so they do not spin in sync */
        ObjectScene_Add(scene, coins[i]);
    }
    for (i = 0; i < GHOSTS; i++) {
        ghosts[i] = make_animated(ghost_state, 100 + i * 50, 60 + i * 40);
        ObjectScene_Add(scene, ghosts[i]);
    }

    for (;;) {
        MT2D_Clear_Main_Window(); /* also clears the images of the last frame */
        ObjectScene_Render(scene);
        Sprite_Render_on_Window(robot[(steps / 2) % 2], DISPLAY_WINDOW1, px, py);
        sprintf(text, " SCORE %03d ", score);
        insert_string_on_display(text, 0, 2, DISPLAY_WINDOW1);
        insert_string_on_display((char *)" arrows: move   ESC: quit ", 23, 26, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);

        while (MT2D_Keyboard_touched()) {
            key = MT2D_Keyboard_keytouched();
            if (key == escape_pressed) return 0;
            if (key == arrow_key_pressed) {
                key = MT2D_Keyboard_keytouched();
                if (key == arrow_up_pressed && py > 8) py -= 4;
                if (key == arrow_down_pressed && py < SCREEN_H - SIZE - 12) py += 4;
                if (key == arrow_left_pressed && px > 0) px -= 4;
                if (key == arrow_right_pressed && px < SCREEN_W - SIZE) px += 4;
                steps++; /* walking animation: one step per key press */
            }
        }

        ObjectScene_Goto_NextSteps(scene); /* plays the animations */

        for (i = 0; i < GHOSTS; i++) { /* ghosts drift in a wave */
            ObjectPosX(ghosts[i]) = SCREEN_W / 2 - SIZE / 2 + (int)(120.0 * sin((tick + i * 60) * 0.03 * (i + 1)));
            ObjectPosY(ghosts[i]) = SCREEN_H / 2 - SIZE / 2 + (int)(70.0 * sin((tick + i * 35) * 0.05));
            if (touching(px, py, ObjectPosX(ghosts[i]), ObjectPosY(ghosts[i]))) {
                px = SCREEN_W / 2;
                py = SCREEN_H / 2;
                if (score > 0) score--;
                MT2D_System_Beep(200, 60);
            }
        }
        for (i = 0; i < COINS; i++) {
            if (touching(px, py, ObjectPosX(coins[i]), ObjectPosY(coins[i]))) {
                score++;
                ObjectPosX(coins[i]) = 10 + (tick * 7 + i * 71) % (SCREEN_W - 40);
                ObjectPosY(coins[i]) = 16 + (tick * 3 + i * 37) % (SCREEN_H - 50);
                MT2D_System_Beep(1200, 30);
            }
        }
        tick++;
        MT2D_System_Delay(30);
    }
}

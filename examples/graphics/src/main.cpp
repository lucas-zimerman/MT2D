/*
   Graphics: image sprites (PNG) drawn on a 320x240 virtual screen on top of the text screen.
   Arrow keys (or a game controller) move the ship, collect the coins, ESC quits.

   The ship is moved with the joystick code instead of MT2D_Keyboard_keytouched: it keeps track of the
   keys that are being held, so the movement is smooth (no waiting for the keyboard repeat) and several
   keys work at the same time. With SDL the arrow keys act as the left stick.
   The images are made by tools/make_assets.py (cyan = transparent).
   On a touch screen (SDL: Android, the browser) a stick and an ESC button show up after the first touch,
   see MT2D/Touch/MT2D_Touch.h: they are objects that write into the same joystick state.
*/
#include <stdio.h>
#include <stdlib.h>
#include "example.h"
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/Touch/MT2D_Touch.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Vars.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define COINS 5
#define SHIP_SPEED 3 /* pixels per frame at full stick */

/* Converts a stick value (-32768..32767) to pixels, ignoring the small values near the center. */
static int stick_to_pixels(int axis)
{
    if (axis > -8000 && axis < 8000) return 0;
    return axis * SHIP_SPEED / 32767;
}

static Sprite *load(const char *name)
{
    Sprite *s = Load_Sprite_Image((char *)name, 0, 0);
    if (s) {
        s->ScaleX = s->SizeX; /* size on the 320x240 screen: 1 image pixel = 1 screen pixel */
        s->ScaleY = s->SizeY;
    }
    return s;
}

/* One state showing an ASCII sprite (cells: w*h characters, row major) */
static ObjectState *touch_state(const char *name, int w, int h, const char *cells)
{
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)name);
    MT2D_ObjectState_ADD(st, Sprite_Create_Ascii_From_Cells(w, h, cells), 1, 0);
    return st;
}

/* Touch stick: the "area" sprite is where the thumb moves, the object itself is the thumb */
static Object *touch_stick(void)
{
    ObjectState **states = (ObjectState **)malloc(3 * sizeof(ObjectState *));
    Object *o;
    states[0] = touch_state("up", 3, 1, "(o)");
    states[1] = touch_state("down", 3, 1, "(O)");
    states[2] = touch_state("area", 11, 5,
                            ".---------."
                            "|         |"
                            "|         |"
                            "|         |"
                            "'---------'");
    o = Object_Create(false, false, 0, 0, 1, 18, states, 3);
    Object_Add_Variable(o, MT2D_Object_Create_Var_String("Btn_Type", "Axis"));
    Object_Add_Variable(o, MT2D_Object_Create_Var_String("Ax_Style", "Reactive"));
    Object_Add_Variable(o, MT2D_Object_Create_Var_String("Ax_Area", "area"));
    return o;
}

/* Touch button 1 (mapped to ESC below) */
static Object *touch_button(void)
{
    ObjectState **states = (ObjectState **)malloc(2 * sizeof(ObjectState *));
    states[0] = touch_state("up", 7, 3, ".-----.| ESC |'-----'");
    states[1] = touch_state("down", 7, 3, ".=====.[ ESC ]'====='");
    return Object_Create(false, false, 0, 0, 71, 19, states, 2); /* Btn_Type and Btn_Id default to button 1 */
}

/* Note: images are always drawn on top of the text screen, so the score text only
   shows where there are no images. That is why there is no full screen background. */

/* Draws a layer twice so it can scroll forever. */
static void scroll_layer(Sprite *s, int offset)
{
    int x = -(offset % SCREEN_W);
    Sprite_Render_on_Window(s, DISPLAY_WINDOW1, x, 0);
    Sprite_Render_on_Window(s, DISPLAY_WINDOW1, x + SCREEN_W, 0);
}

int main(int argc, char **argv)
{
    Sprite *far_stars, *near_stars, *ship, *red, *blue, *rock, *coin[4];
    int ship_x = 20, ship_y = 110;
    int coin_x[COINS], coin_y[COINS];
    int score = 0, tick = 0, i;
    char text[40];

    (void)argc;
    (void)argv;
    MT2D_Init();
    MT2D_Joystick_Init();
    MT2D_Joystick_Map_Button01(escape_pressed); /* button 1 = the ESC key */
    MT2D_Touch_Init();
    MT2D_Touch_Bind(touch_stick());
    MT2D_Touch_Bind(touch_button());

    far_stars = load("stars_far.png");
    near_stars = load("stars_near.png");
    ship = load("ship.png");
    red = load("planet_red.png");
    blue = load("planet_blue.png");
    rock = load("rock.png");
    for (i = 0; i < 4; i++) {
        char name[16];
        sprintf(name, "coin%d.png", i);
        coin[i] = load(name);
    }
    if (!far_stars || !ship || !coin[0]) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"Could not load the images, run the example from its bin folder.", 12, 8, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);
        MT2D_Keyboard_keytouched();
        return 1;
    }

    for (i = 0; i < COINS; i++) {
        coin_x[i] = 120 + i * 40;
        coin_y[i] = 30 + (i * 53) % 170;
    }

    for (;;) {
        MT2D_Clear_Main_Window(); /* also clears the images of the last frame */

        /* back to front */
        scroll_layer(far_stars, tick / 3);
        scroll_layer(near_stars, tick);
        Sprite_Render_on_Window(red, DISPLAY_WINDOW1, SCREEN_W - (tick / 2) % (SCREEN_W + 120), 30);
        Sprite_Render_on_Window(blue, DISPLAY_WINDOW1, SCREEN_W - (tick / 2 + 150) % (SCREEN_W + 120), 160);
        Sprite_Render_on_Window(rock, DISPLAY_WINDOW1, SCREEN_W - (tick * 2) % (SCREEN_W + 60), 100 + (tick / 40 % 5) * 10);
        for (i = 0; i < COINS; i++)
            Sprite_Render_on_Window(coin[(tick / 4 + i) % 4], DISPLAY_WINDOW1, coin_x[i], coin_y[i]);
        Sprite_Render_on_Window(ship, DISPLAY_WINDOW1, ship_x, ship_y);

        /* the text screen is still there, use it for the HUD */
        sprintf(text, " SCORE %03d ", score);
        insert_string_on_display(text, 0, 1, DISPLAY_WINDOW1);
        insert_string_on_display((char *)" arrows: move   ESC: quit ", 23, 27, DISPLAY_WINDOW1);
        MT2D_Touch_Render(); /* the touch controls, last so they are on top of the text */
        MT2D_Draw_Window(DISPLAY_WINDOW1);

        MT2D_Joystick_Update(); /* reads the window events and the held keys */
        MT2D_Touch_Update();    /* after the joystick update: adds the touch stick and button */
        if (MT2D_Joystick_Button_Touched(1)) break;
        ship_x += stick_to_pixels(MT2D_Joystick_Get_LeftAxisX());
        ship_y += stick_to_pixels(MT2D_Joystick_Get_LeftAxisY());
        if (ship_x < 0) ship_x = 0;
        if (ship_x > SCREEN_W - 32) ship_x = SCREEN_W - 32;
        if (ship_y < 4) ship_y = 4;
        if (ship_y > SCREEN_H - 24) ship_y = SCREEN_H - 24;

        for (i = 0; i < COINS; i++) { /* collect coins, they come back from the right */
            coin_x[i] -= 1;
            if (coin_x[i] < -16) {
                coin_x[i] = SCREEN_W + (i * 37) % 60;
                coin_y[i] = 20 + (tick * 7 + i * 41) % 190;
            }
            if (ship_x < coin_x[i] + 16 && ship_x + 32 > coin_x[i] && ship_y < coin_y[i] + 16 && ship_y + 16 > coin_y[i]) {
                score++;
                coin_x[i] = SCREEN_W + 20;
                coin_y[i] = 20 + (tick * 13 + i * 29) % 190;
                MT2D_System_Beep(1200, 20);
            }
        }
        tick++;
        MT2D_System_Delay(30);
    }
    MT2D_Touch_Close();
    MT2D_Joystick_Close();
    return 0;
}

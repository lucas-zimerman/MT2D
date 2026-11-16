/*
   Welcome: the home page of MT2D, written with MT2D itself (Emscripten + SDL2).

   One program, five screens, all of them drawn by the engine:
     menu     text mode menu (arrows / number keys / tap)
     text     text mode effects drawn straight into the 80x25 buffer (plasma, fire, stars, spiral)
     graphics PNG sprites on the 320x240 virtual screen (space flight, desert parallax, sprite zoo)
     sound    sound effects (MT2D_Play_Audio) and a piano of in-memory WAV notes (MT2D_PlayAudioFromMemory)
     music    MT2D_Play_Music with a visualizer fed by the mixer output
     about    what MT2D is and where it runs

   A browser tab cannot run a "for (;;)" loop: the program is the function frame(), called once per
   screen refresh by emscripten_set_main_loop (see examples/web-tui). Browsers keep the audio locked until the
   page gets a click or a key press, so the music starts at the first key.
   New code is ANSI C (C89), see AGENTS.md.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <emscripten.h>
#include <SDL2/SDL_mixer.h>
#include "example.h"
#include "web_timing.h"
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <MT2D/MT2D_Settings.h>

#define ROWS 25
#define COLS 80
#define SCREEN_W 320
#define SCREEN_H 240

/* key codes: ASCII for characters, the arrows are made of the two part code of the engine */
#define K_UP 1001
#define K_DOWN 1002
#define K_LEFT 1003
#define K_RIGHT 1004

/* CP850 characters of the screen buffer */
#define C_BLOCK 219
#define C_LOWER 220
#define C_UPPER 223
#define C_SHADE1 176
#define C_SHADE2 177
#define C_SHADE3 178
#define C_DH 205 /* double horizontal */
#define C_DV 186
#define C_DTL 201
#define C_DTR 187
#define C_DBL 200
#define C_DBR 188
#define C_SH 196 /* single horizontal */
#define C_SV 179
#define C_STL 218
#define C_STR 191
#define C_SBL 192
#define C_SBR 217
#define C_ARROW 16

#define SCENE_MENU 0
#define SCENE_TEXT 1
#define SCENE_GFX 2
#define SCENE_SOUND 3
#define SCENE_MUSIC 4
#define SCENE_ABOUT 5

#define MENU_ITEMS 5
#define TRACKS 3

static int scene = SCENE_MENU;
static int frame_count;
static int audio_unlocked, muted = 1; /* starts muted: the browser needs a key press before it plays sound anyway */

/* ---------------------------------------------------------------------------------------------- */
/* small drawing helpers                                                                          */
/* ---------------------------------------------------------------------------------------------- */

static void put(const char *s, int row, int col)
{
    insert_string_on_display((char *)s, row, col, DISPLAY_WINDOW1);
}

static void put_center(const char *s, int row)
{
    put(s, row, (COLS - (int)strlen(s)) / 2);
}

static void cell(int row, int col, int ch)
{
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) WINDOW1[row][col] = (unsigned char)ch;
}

static void fill(int row, int col, int h, int w, int ch)
{
    int r, c;
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) cell(row + r, col + c, ch);
}

/* a box with the double (dbl = 1) or the single line characters, the inside is cleared */
static void box(int row, int col, int h, int w, int dbl)
{
    int r, c;
    int tl = dbl ? C_DTL : C_STL, tr = dbl ? C_DTR : C_STR, bl = dbl ? C_DBL : C_SBL, br = dbl ? C_DBR : C_SBR;
    int hz = dbl ? C_DH : C_SH, vt = dbl ? C_DV : C_SV;

    fill(row, col, h, w, ' ');
    for (c = 1; c < w - 1; c++) {
        cell(row, col + c, hz);
        cell(row + h - 1, col + c, hz);
    }
    for (r = 1; r < h - 1; r++) {
        cell(row + r, col, vt);
        cell(row + r, col + w - 1, vt);
    }
    cell(row, col, tl);
    cell(row, col + w - 1, tr);
    cell(row + h - 1, col, bl);
    cell(row + h - 1, col + w - 1, br);
}

/* title bar on the first row and key help on the last one */
static void chrome(const char *title, const char *help)
{
    char line[COLS + 1];

    fill(0, 0, 1, COLS, C_DH);
    sprintf(line, " %s ", title);
    put(line, 0, 2);
    fill(ROWS - 1, 0, 1, COLS, ' ');
    put(help, ROWS - 1, 1);
    if (muted) put(" MUTED ", 0, COLS - 10);
}

/* ---------------------------------------------------------------------------------------------- */
/* sound                                                                                          */
/* ---------------------------------------------------------------------------------------------- */

static const char *track_file[TRACKS] = { "desert.ogg", "ode.ogg", "elise.ogg" };
static const char *track_name[TRACKS] = { "Desert drift  (soundtrack of the desert example)", "Ode to Joy   (Beethoven, public domain)",
                                          "Fur Elise     (Beethoven, public domain)" };
static int track = -1, music_paused, volume = 80, music_frames;

/* mixer output seen by the visualizer: the last 32 levels */
#define LEVELS 32
static int levels[LEVELS];
static int level_head;

static void post_mix(void *udata, Uint8 *stream, int len)
{
    Sint16 *s = (Sint16 *)stream;
    int n = len / 2, i, peak = 0, v;

    (void)udata;
    for (i = 0; i < n; i++) {
        v = s[i];
        if (v < 0) v = -v;
        if (v > peak) peak = v;
    }
    level_head = (level_head + 1) % LEVELS;
    levels[level_head] = peak;
}

static void apply_volume(void)
{
    Mix_VolumeMusic(muted ? 0 : volume);
    Mix_Volume(-1, muted ? 0 : 96);
}

static void play_track(int t)
{
    track = t;
    music_paused = 0;
    music_frames = 0;
    MT2D_Play_Music((char *)track_file[t]);
    apply_volume();
}

static void sfx(const char *file)
{
    if (!muted) MT2D_Play_Audio((char *)file);
}

/* ---- piano notes ----
   MT2D_System_Beep (the PC speaker) and the audio files cannot be used in the same run with the SDL backend (the
   first one asked wins, see Documentation/general/audio.md), and this program plays files. So every piano note
   is a tiny WAV made in memory when the program starts and played with MT2D_PlayAudioFromMemory. */
#define NOTE_RATE 22050
#define NOTE_SAMPLES 7000 /* about 0.32 s */
#define NOTE_BYTES (44 + NOTE_SAMPLES * 2)
#define NOTE_COUNT 14

static unsigned char *note_wav[NOTE_COUNT];

static void put_u32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v & 255);
    p[1] = (unsigned char)((v >> 8) & 255);
    p[2] = (unsigned char)((v >> 16) & 255);
    p[3] = (unsigned char)((v >> 24) & 255);
}

static void put_u16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v & 255);
    p[1] = (unsigned char)((v >> 8) & 255);
}

/* mono 16 bit WAV of a plucked note: the tone and two soft overtones, a quick attack and a decay */
static unsigned char *make_note(double hz)
{
    unsigned char *w = (unsigned char *)malloc(NOTE_BYTES);
    int i, v;
    double t, env, x;

    if (!w) return NULL;
    memcpy(w, "RIFF", 4);
    put_u32(w + 4, NOTE_BYTES - 8);
    memcpy(w + 8, "WAVEfmt ", 8);
    put_u32(w + 16, 16);
    put_u16(w + 20, 1); /* PCM */
    put_u16(w + 22, 1); /* mono */
    put_u32(w + 24, NOTE_RATE);
    put_u32(w + 28, NOTE_RATE * 2);
    put_u16(w + 32, 2);
    put_u16(w + 34, 16);
    memcpy(w + 36, "data", 4);
    put_u32(w + 40, NOTE_SAMPLES * 2);
    for (i = 0; i < NOTE_SAMPLES; i++) {
        t = (double)i / NOTE_RATE;
        env = exp(-t * 7.0);
        if (i < 110) env *= (double)i / 110.0; /* no click at the start */
        if (i > NOTE_SAMPLES - 400) env *= (double)(NOTE_SAMPLES - i) / 400.0;
        x = sin(6.2831853 * hz * t) + 0.35 * sin(6.2831853 * hz * 2.0 * t) + 0.15 * sin(6.2831853 * hz * 3.0 * t);
        v = (int)(x * env * 9000.0);
        put_u16(w + 44 + i * 2, (unsigned)(v & 0xFFFF));
    }
    return w;
}

static void play_note(unsigned char *wav)
{
    if (wav && !muted) MT2D_PlayAudioFromMemory(wav, NOTE_BYTES, 0);
}

static void unlock_audio(void)
{
    if (audio_unlocked) return;
    audio_unlocked = 1;
    play_track(0);
}

/* ---------------------------------------------------------------------------------------------- */
/* browser side                                                                                   */
/* ---------------------------------------------------------------------------------------------- */

static void set_page_title(void)
{
    static const char *names[6] = { "", "Text mode", "Graphics", "Sound", "Music", "About" };
    EM_ASM({
        var n = UTF8ToString($0);
        document.title = n ? 'MT2D - ' + n : 'MT2D - Monochrome Text 2D Engine';
        if (window.mt2dScene) window.mt2dScene($1);
    }, names[scene], scene);
}

static void goto_scene(int s)
{
    scene = s;
    set_page_title();
}

/* ---------------------------------------------------------------------------------------------- */
/* menu                                                                                           */
/* ---------------------------------------------------------------------------------------------- */

static const char *banner[5] = {
    "#   # ##### ####  ####  ",
    "## ##   #       # #   # ",
    "# # #   #    ###  #   # ",
    "#   #   #   #     #   # ",
    "#   #   #   ##### ####  "
};
static const char *menu_text[MENU_ITEMS] = { "Text mode       effects drawn with characters", "Graphics        sprites on top of the text",
                                             "Sound lab       samples and a piano", "Music player    a song with a visualizer",
                                             "About MT2D      what this is, where it runs" };
static int menu_sel;

static void draw_menu(void)
{
    int r, c, i, x;
    char line[COLS + 1];

    /* a slow starfield behind everything */
    for (i = 0; i < 40; i++) {
        x = (i * 37 + frame_count / (2 + i % 3)) % COLS;
        cell(1 + (i * 11) % 23, COLS - 1 - x, (i % 3) == 0 ? '+' : '.');
    }

    box(1, 12, 9, 56, 1);
    for (r = 0; r < 5; r++) {
        for (c = 0; banner[r][c]; c++) {
            if (banner[r][c] == '#') {
                cell(2 + r, 18 + c * 2, C_BLOCK);
                cell(2 + r, 19 + c * 2, C_BLOCK);
            }
        }
    }
    put_center("Monochrome Text 2D Engine", 8);

    box(11, 10, 8, 60, 0);
    for (i = 0; i < MENU_ITEMS; i++) {
        sprintf(line, "%d  %s", i + 1, menu_text[i]);
        put(line, 12 + i, 14);
    }
    cell(12 + menu_sel, 12, C_ARROW);
    cell(12 + menu_sel, 13, ' ');
    /* the selected row is shown in brackets, the font has no colors */
    cell(12 + menu_sel, 11, '[');
    cell(12 + menu_sel, 68, ']');

    put_center("This page is an MT2D program: text, sprites and sound come from the engine.", 20);
    put_center(muted ? "Up / Down + Enter, or press 1-5.  Sound is off: press M" : "Up / Down + Enter, or press 1-5", 22);
    chrome("MT2D", "arrows: move   Enter: open   M: mute");
}

/* ---------------------------------------------------------------------------------------------- */
/* text mode effects                                                                              */
/* ---------------------------------------------------------------------------------------------- */

static const unsigned char ramp[] = { ' ', '.', ':', C_SHADE1, C_SHADE2, C_SHADE3, C_BLOCK };
#define RAMP_SIZE 7
#define FX_ROWS 23 /* rows 1..23 */
#define FX_COUNT 4

static int text_fx = 1;
static double fx_t;
static unsigned char heat[FX_ROWS + 1][COLS];
#define STARS 70
static double star_x[STARS], star_y[STARS], star_z[STARS];

static unsigned char shade(double v)
{
    int i = (int)(v * (RAMP_SIZE - 1) + 0.5);
    if (i < 0) i = 0;
    if (i >= RAMP_SIZE) i = RAMP_SIZE - 1;
    return ramp[i];
}

static void star_reset(int i, double z)
{
    star_x[i] = (rand() % 2000 - 1000) / 100.0;
    star_y[i] = (rand() % 2000 - 1000) / 100.0;
    star_z[i] = z;
}

static void fx_plasma(void)
{
    int x, y;
    double v;
    for (y = 0; y < FX_ROWS; y++) {
        for (x = 0; x < COLS; x++) {
            v = sin(x * 0.18 + fx_t) + sin(y * 0.35 - fx_t * 1.3) + sin((x * 0.12 + y * 0.25) + fx_t * 0.7)
                + sin(sqrt((x - 40.0) * (x - 40.0) / 4.0 + (y - 12.0) * (y - 12.0)) * 0.5 - fx_t);
            WINDOW1[1 + y][x] = shade((v + 4.0) / 8.0);
        }
    }
}

static void fx_fire(void)
{
    int x, y, v;
    for (x = 0; x < COLS; x++) heat[FX_ROWS - 1][x] = (unsigned char)(rand() % 4 == 0 ? 0 : 200 + rand() % 56);
    for (y = 0; y < FX_ROWS - 1; y++) {
        for (x = 0; x < COLS; x++) {
            v = heat[y + 1][x] + heat[y + 1][(x + 1) % COLS] + heat[y + 1][(x + COLS - 1) % COLS];
            v = v / 3 - (rand() % 28 > 10 ? 18 : 6);
            if (y + 2 < FX_ROWS) v = (v * 3 + heat[y + 2][x]) / 4;
            heat[y][x] = (unsigned char)(v < 0 ? 0 : v);
        }
    }
    for (y = 0; y < FX_ROWS; y++)
        for (x = 0; x < COLS; x++) WINDOW1[1 + y][x] = shade(heat[y][x] / 220.0);
}

static void fx_stars(void)
{
    int i, x, y;
    fill(1, 0, FX_ROWS, COLS, ' ');
    for (i = 0; i < STARS; i++) {
        star_z[i] -= 0.12;
        if (star_z[i] < 0.3) star_reset(i, 10.0);
        x = 40 + (int)(star_x[i] / star_z[i] * 12.0);
        y = 12 + (int)(star_y[i] / star_z[i] * 5.0);
        if (x >= 0 && x < COLS && y >= 1 && y <= FX_ROWS) WINDOW1[y][x] = star_z[i] < 3.0 ? C_BLOCK : (star_z[i] < 6.0 ? 'o' : '.');
    }
}

static void fx_spiral(void)
{
    int x, y;
    double dx, dy, d, a, v;
    for (y = 0; y < FX_ROWS; y++) {
        for (x = 0; x < COLS; x++) {
            dx = (x - 40.0) / 2.0;
            dy = y - 11.0;
            d = sqrt(dx * dx + dy * dy) + 0.001;
            a = atan2(dy, dx);
            v = sin(8.0 / d * 3.0 - fx_t * 2.0 + a * 3.0);
            WINDOW1[1 + y][x] = shade((v + 1.0) / 2.0 * (d > 1.5 ? 1.0 : d / 1.5));
        }
    }
}

static void draw_text_scene(void)
{
    static const char *names[FX_COUNT] = { "plasma", "fire", "stars", "spiral" };
    char help[COLS + 1];
    int i;

    if (text_fx == 1) fx_plasma();
    else if (text_fx == 2) fx_fire();
    else if (text_fx == 3) fx_stars();
    else fx_spiral();
    fx_t += 0.06;

    help[0] = 0;
    for (i = 0; i < FX_COUNT; i++) {
        char part[24];
        sprintf(part, text_fx == i + 1 ? "[%d %s] " : " %d %s  ", i + 1, names[i]);
        strcat(help, part);
    }
    strcat(help, " ESC: menu");
    chrome("TEXT MODE  every cell is one character of an 80x25 screen", help);
}

/* ---------------------------------------------------------------------------------------------- */
/* graphics                                                                                       */
/* ---------------------------------------------------------------------------------------------- */

static Sprite *spr_far, *spr_near, *spr_ship, *spr_red, *spr_blue, *spr_rock, *spr_coin[4];
static Sprite *spr_sky, *spr_mesas, *spr_dunes, *spr_ground, *spr_fore, *spr_truck[3];
static Sprite *spr_robot[2], *spr_ghost[2], *spr_big_coin[4];
static Sprite *spr_head[3], *spr_mound[2], *spr_dust[4], *spr_grain[2], *spr_seg[2], *spr_drift[4], *spr_gameover;
static int gfx_scene = 1, ship_x = 20, ship_y = 110, gfx_score, gfx_tick;
#define COINS 5
static int coin_x[COINS], coin_y[COINS];

static Sprite *load_native(const char *name)
{
    Sprite *s = Load_Sprite_Image((char *)name, 0, 0);
    if (s) {
        s->ScaleX = s->SizeX; /* 1 image pixel = 1 pixel of the 320x240 screen */
        s->ScaleY = s->SizeY;
    }
    return s;
}

static Sprite *load_scaled(const char *name, int size)
{
    return Load_Sprite_Image((char *)name, size, size);
}

static void load_images(void)
{
    int i;
    char name[24];

    spr_far = load_native("stars_far.png");
    spr_near = load_native("stars_near.png");
    spr_ship = load_native("ship.png");
    spr_red = load_native("planet_red.png");
    spr_blue = load_native("planet_blue.png");
    spr_rock = load_native("rock.png");
    spr_sky = load_native("sky.png");
    spr_mesas = load_native("mesas.png");
    spr_dunes = load_native("dunes.png");
    spr_ground = load_native("ground.png");
    spr_fore = load_native("foreground.png");
    for (i = 0; i < 4; i++) {
        sprintf(name, "coin%d.png", i);
        spr_coin[i] = load_native(name);
        spr_big_coin[i] = load_scaled(name, 48);
    }
    for (i = 0; i < 3; i++) {
        sprintf(name, "truck%d.png", i);
        spr_truck[i] = load_native(name);
    }
    spr_head[0] = load_native("head_up.png");
    spr_head[1] = load_native("head_mid.png");
    spr_head[2] = load_native("head_down.png");
    spr_seg[0] = load_native("seg_big.png");
    spr_seg[1] = load_native("seg_small.png");
    spr_drift[0] = load_native("drift_u1.png");
    spr_drift[1] = load_native("drift_u2.png");
    spr_drift[2] = load_native("drift_d1.png");
    spr_drift[3] = load_native("drift_d2.png");
    spr_gameover = load_native("gameover.png");
    for (i = 0; i < 2; i++) {
        sprintf(name, "mound%d.png", i);
        spr_mound[i] = load_native(name);
        sprintf(name, "grain%d.png", i);
        spr_grain[i] = load_native(name);
    }
    for (i = 0; i < 4; i++) {
        sprintf(name, "dust%d.png", i);
        spr_dust[i] = load_native(name);
    }
    for (i = 0; i < 2; i++) {
        sprintf(name, "robot%d.png", i);
        spr_robot[i] = load_scaled(name, 48);
        sprintf(name, "ghost%d.png", i);
        spr_ghost[i] = load_scaled(name, 48);
    }
    for (i = 0; i < COINS; i++) {
        coin_x[i] = 120 + i * 40;
        coin_y[i] = 30 + (i * 53) % 170;
    }
}

static void draw_sprite(Sprite *s, int x, int y)
{
    if (s) Sprite_Render_on_Window(s, DISPLAY_WINDOW1, x, y);
}

/* draws a layer twice so it scrolls forever */
static void scroll_layer(Sprite *s, int offset, int y)
{
    int x = -(offset % SCREEN_W);
    draw_sprite(s, x, y);
    draw_sprite(s, x + SCREEN_W, y);
}

static int stick_to_pixels(int axis, int speed)
{
    if (axis > -8000 && axis < 8000) return 0;
    return axis * speed / 32767;
}

static void gfx_space(void)
{
    int i;
    char text[40];

    scroll_layer(spr_far, gfx_tick / 3, 0);
    scroll_layer(spr_near, gfx_tick, 0);
    draw_sprite(spr_red, SCREEN_W - (gfx_tick / 2) % (SCREEN_W + 120), 30);
    draw_sprite(spr_blue, SCREEN_W - (gfx_tick / 2 + 150) % (SCREEN_W + 120), 160);
    draw_sprite(spr_rock, SCREEN_W - (gfx_tick * 2) % (SCREEN_W + 60), 100 + (gfx_tick / 40 % 5) * 10);
    for (i = 0; i < COINS; i++) draw_sprite(spr_coin[(gfx_tick / 4 + i) % 4], coin_x[i], coin_y[i]);
    draw_sprite(spr_ship, ship_x, ship_y);

    ship_x += stick_to_pixels(MT2D_Joystick_Get_LeftAxisX(), 2);
    ship_y += stick_to_pixels(MT2D_Joystick_Get_LeftAxisY(), 2);
    if (ship_x < 0) ship_x = 0;
    if (ship_x > SCREEN_W - 32) ship_x = SCREEN_W - 32;
    if (ship_y < 14) ship_y = 14;
    if (ship_y > SCREEN_H - 30) ship_y = SCREEN_H - 30;
    for (i = 0; i < COINS; i++) {
        if (gfx_tick % 2 == 0) coin_x[i] -= 1;
        if (coin_x[i] < -16) {
            coin_x[i] = SCREEN_W + (i * 37) % 60;
            coin_y[i] = 20 + (gfx_tick * 7 + i * 41) % 190;
        }
        if (ship_x < coin_x[i] + 16 && ship_x + 32 > coin_x[i] && ship_y < coin_y[i] + 16 && ship_y + 16 > coin_y[i]) {
            gfx_score++;
            coin_x[i] = SCREEN_W + 20;
            coin_y[i] = 20 + (gfx_tick * 13 + i * 29) % 190;
            sfx("coin.wav");
        }
    }
    sprintf(text, " COINS %03d ", gfx_score);
    put(text, 1, 2);
    put("Sprite_Render_on_Window(ship, DISPLAY_WINDOW1, x, y);", 22, 12);
}

/* ---- desert: drive the truck, do not get caught by the sand worm ----
   Same sprites as examples/desert (there the worm is made with object scripts and the truck drives itself).
   Here the code is plain C: the truck follows the arrow keys (forward, backward, up, down) and the worm chases
   it under the sand (a mound and a dust trail), rears up and leaps at the place where the truck is heading.
   If the head touches the truck it is game over. There is no warning. Positions are in 1/16 pixel. */
#define DY 10 /* the whole scene is 10 pixels down, so the first text row stays visible for the score */
#define TRUCK_W 64
#define TRUCK_H 44
#define TRUCK_MIN_Y 122
#define TRUCK_MAX_Y 176
#define PARTS 40
#define P_DUST 0
#define P_GRAIN 1
#define P_SEG 2
#define W_BURROW 0
#define W_CHASE 1
#define W_RISE 2
#define W_LEAP 3
#define W_CRASH 4
#define W_OVER 5

typedef struct {
    int active, kind, x16, y16, vx16, vy16, age, life;
} Part;

static Part parts[PARTS];
static int d_tx16, d_ty16, d_world16, d_speed16, d_cd, d_over, d_score, d_tick;
static int d_state, d_timer, d_age, d_wx16, d_wy;
static int d_sx, d_sfoot, d_lx, d_lfoot, d_T, d_t, d_hx, d_hy, d_hfoot;

static void part_spawn(int kind, int x, int y, int vx16, int vy16, int life)
{
    int i;
    for (i = 0; i < PARTS; i++) {
        if (!parts[i].active) {
            parts[i].active = 1;
            parts[i].kind = kind;
            parts[i].x16 = x * 16;
            parts[i].y16 = y * 16;
            parts[i].vx16 = vx16;
            parts[i].vy16 = vy16;
            parts[i].age = 0;
            parts[i].life = life;
            return;
        }
    }
}

static void grains(int x, int y, int n)
{
    int i;
    for (i = 0; i < n; i++) part_spawn(P_GRAIN, x + rand() % 24, y, rand() % 80 - 40, -40 - rand() % 40, 34);
}

static void desert_reset(void)
{
    memset(parts, 0, sizeof(parts));
    d_tx16 = 110 * 16;
    d_ty16 = 146 * 16;
    d_world16 = 0;
    d_speed16 = 28;
    d_cd = 0;
    d_over = 0;
    d_score = 0;
    d_tick = 0;
    d_state = W_BURROW;
}

static int clampi(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static int overlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

static void worm_update(int dirx, int diry)
{
    int tx = d_tx16 / 16, ty = d_ty16 / 16, v, px, py;

    switch (d_state) {
    case W_BURROW: /* far behind the truck, on its lane */
        d_wx16 = -50 * 16;
        d_wy = ty + 11;
        d_age = 0;
        d_state = W_CHASE;
        break;
    case W_CHASE: /* under the sand: it comes closer and keeps the lane of the truck */
        v = clampi((tx - 50) * 16 - d_wx16, -800, 1280) / 25;
        d_wx16 += v;
        if (d_wy < ty + 11) d_wy++;
        else if (d_wy > ty + 11) d_wy--;
        d_age++;
        if (d_age % 7 == 0) part_spawn(P_DUST, d_wx16 / 16, d_wy + 8, -16, 0, 24);
        if (d_age > 150 && abs(tx - 50 - d_wx16 / 16) < 45 && rand() % 45 == 0) {
            d_state = W_RISE;
            d_timer = 20;
            sfx("roar.wav");
            grains(d_wx16 / 16, d_wy + 8, 4);
        }
        break;
    case W_RISE: /* the head comes out, then it jumps at where the truck is going */
        d_wx16 -= d_speed16;
        if (--d_timer <= 0) {
            d_T = 46 - 3 * d_score;
            if (d_T < 26) d_T = 26;
            px = clampi(tx + dirx * 2 * d_T, 0, SCREEN_W - TRUCK_W);
            py = clampi(ty + (diry * 3 * d_T) / 2, TRUCK_MIN_Y, TRUCK_MAX_Y);
            d_sx = d_wx16 / 16;
            d_sfoot = d_wy + 32;
            d_lx = px + 12;
            d_lfoot = py + 43;
            d_t = 0;
            d_state = W_LEAP;
            sfx("whoosh.wav");
        }
        break;
    case W_LEAP:
        d_t++;
        d_hx = d_sx + (d_lx - d_sx) * d_t / d_T;
        d_hfoot = d_sfoot + (d_lfoot - d_sfoot) * d_t / d_T;
        d_hy = d_hfoot - 32 - (4 * 64 * d_t * (d_T - d_t)) / (d_T * d_T);
        if (d_t % 3 == 0) part_spawn(P_SEG, d_hx + 3, d_hy + 4, 0, 6, 34);
        if (d_t > d_T * 15 / 100 && d_t < d_T * 95 / 100 && overlap(d_hx + 8, d_hy + 6, 24, 20, tx + 4, ty + 6, TRUCK_W - 8, TRUCK_H - 10)) {
            d_over = 1;
            d_state = W_OVER;
            sfx("explosion.ogg");
            sfx("roar.wav");
            grains(tx + 10, ty + 20, 6);
            break;
        }
        if (d_t >= d_T) {
            d_state = W_CRASH;
            d_timer = 50;
            sfx("thud.wav");
            grains(d_hx, d_hfoot - 8, 5);
            part_spawn(P_DUST, d_hx, d_hfoot - 24, -16, 0, 24);
        }
        break;
    case W_CRASH: /* it missed and sank into the sand */
        if (--d_timer <= 0) {
            d_score++;
            d_state = W_BURROW;
        }
        break;
    default:
        break;
    }
}

static void desert_update(void)
{
    int ax = MT2D_Joystick_Get_LeftAxisX(), ay = MT2D_Joystick_Get_LeftAxisY();
    int dirx = ax < -8000 ? -1 : (ax > 8000 ? 1 : 0), diry = ay < -8000 ? -1 : (ay > 8000 ? 1 : 0), i;

    d_tick++;
    if (!d_over) {
        d_tx16 = clampi(d_tx16 + dirx * 32, 0, (SCREEN_W - TRUCK_W) * 16);
        d_ty16 = clampi(d_ty16 + diry * 24, TRUCK_MIN_Y * 16, TRUCK_MAX_Y * 16);
        d_speed16 = 28 + dirx * 12;
        if (diry && d_cd == 0) {
            sfx("screech.wav");
            d_cd = 60;
        }
        if (d_cd > 0) d_cd--;
        if (d_tick % (diry ? 3 : 7) == 0) part_spawn(P_DUST, d_tx16 / 16 + 2, d_ty16 / 16 + 19, -24, 0, 24);
        worm_update(dirx, diry);
    } else {
        d_speed16 = 0;
    }
    d_world16 += d_speed16;
    for (i = 0; i < PARTS; i++) {
        if (!parts[i].active) continue;
        parts[i].x16 += parts[i].vx16 - d_speed16;
        parts[i].y16 += parts[i].vy16;
        if (parts[i].kind == P_GRAIN) parts[i].vy16 += 3;
        if (++parts[i].age >= parts[i].life) parts[i].active = 0;
    }
}

static void part_draw(int kind)
{
    int i, f;
    for (i = 0; i < PARTS; i++) {
        if (!parts[i].active || parts[i].kind != kind) continue;
        if (kind == P_DUST) {
            f = parts[i].age * 4 / parts[i].life;
            draw_sprite(spr_dust[f > 3 ? 3 : f], parts[i].x16 / 16, parts[i].y16 / 16 + DY);
        } else if (kind == P_GRAIN) {
            draw_sprite(spr_grain[(parts[i].age / 6) % 2], parts[i].x16 / 16, parts[i].y16 / 16 + DY);
        } else {
            draw_sprite(spr_seg[parts[i].age < 26 ? 0 : 1], parts[i].x16 / 16, parts[i].y16 / 16 + DY);
        }
    }
}

static void truck_draw(void)
{
    int tx = d_tx16 / 16, ty = d_ty16 / 16, ax = MT2D_Joystick_Get_LeftAxisY();
    Sprite *s;

    if (d_over) s = spr_drift[2];
    else if (ax < -8000) s = spr_drift[(d_tick / 4) % 2];
    else if (ax > 8000) s = spr_drift[2 + (d_tick / 4) % 2];
    else s = spr_truck[(d_tick / 5) % 3];
    draw_sprite(s, tx, ty + DY);
}

static int worm_draw(void) /* returns the y of the feet (for the depth order), 0 if it is not visible */
{
    if (d_state == W_CHASE) {
        if (d_wx16 > -40 * 16) draw_sprite(spr_mound[(d_age / 6) % 2], d_wx16 / 16, d_wy + DY);
        return d_wy + 32;
    }
    if (d_state == W_RISE) {
        draw_sprite(spr_head[0], d_wx16 / 16, d_wy + DY);
        return d_wy + 32;
    }
    if (d_state == W_LEAP || d_state == W_OVER) {
        draw_sprite(spr_head[d_t * 3 / (d_T + 1) > 2 ? 2 : d_t * 3 / (d_T + 1)], d_hx, d_hy + DY);
        return d_hfoot;
    }
    return 0;
}

static void gfx_desert(void)
{
    int wfoot, tfoot = d_ty16 / 16 + TRUCK_H - 1;
    char text[80];

    desert_update();
    draw_sprite(spr_sky, 0, DY);
    scroll_layer(spr_mesas, d_world16 / 96, 58 + DY);
    scroll_layer(spr_dunes, d_world16 / 48, 88 + DY);
    draw_sprite(spr_ground, 0, 120 + DY);
    part_draw(P_DUST);
    part_draw(P_SEG);
    wfoot = (d_state == W_BURROW || d_state == W_CRASH) ? 0 : (d_state == W_LEAP || d_state == W_OVER ? d_hfoot : d_wy + 32);
    if (wfoot > tfoot || d_state == W_LEAP || d_state == W_OVER) { /* the lower one is closer to us, a head in the air is always in front */
        truck_draw();
        worm_draw();
    } else {
        worm_draw();
        truck_draw();
    }
    scroll_layer(spr_fore, d_world16 / 8, 204 + DY);
    part_draw(P_GRAIN);
    if (d_over) draw_sprite(spr_gameover, (SCREEN_W - 232) / 2, 22); /* in the sky: the truck and the worm stay visible */
    /* the first text row is the only text the scene leaves visible */
    fill(0, 0, 1, COLS, ' ');
    sprintf(text, " %s   dodged: %03d   arrows: drive   ESC: menu ", d_over ? "CAUGHT!" : "DESERT", d_score);
    put(text, 0, 2);
}

static void gfx_zoo(void)
{
    int i, f = (gfx_tick / 14) % 2;
    static const char *label[4] = { "robot", "ghost", "coin", "ship" };

    for (i = 0; i < 3; i++) {
        put(label[i], 16, 8 + i * 22);
    }
    draw_sprite(spr_robot[f], 36, 70 + (int)(6 * sin(gfx_tick * 0.08)));
    draw_sprite(spr_ghost[f], 120, 60 + (int)(10 * sin(gfx_tick * 0.06)));
    draw_sprite(spr_big_coin[(gfx_tick / 6) % 4], 204, 70);
    put("Sprite *s = Load_Sprite_Image(\"robot0.png\", 48, 48);", 3, 14);
    put("Images are drawn on top of the text screen. The text is", 19, 12);
    put("still there: it is what you read behind the sprites.", 20, 12);
}

static void draw_gfx_scene(void)
{
    char help[COLS + 1];
    static const char *names[3] = { "space flight", "desert drive", "sprite zoo" };
    int i;

    if (gfx_scene == 1) gfx_space();
    else if (gfx_scene == 2) gfx_desert();
    else gfx_zoo();
    gfx_tick++;

    help[0] = 0;
    for (i = 0; i < 3; i++) {
        char part[28];
        sprintf(part, gfx_scene == i + 1 ? "[%d %s] " : " %d %s  ", i + 1, names[i]);
        strcat(help, part);
    }
    strcat(help, gfx_scene == 3 ? "ESC: menu" : "arrows: move  ESC: menu");
    if (gfx_scene != 2) chrome("GRAPHICS", help); /* the desert has its own first row */
    /* the title bar is on the text layer: images cover it, so it only shows where they are transparent */
}

/* ---------------------------------------------------------------------------------------------- */
/* sound lab                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

static const char *sample_file[6] = { "blip.wav", "coin.wav", "explosion.ogg", "jump.ogg", "laser.wav", "powerup.wav" };
static const char *sample_name[6] = { "blip", "coin", "explosion", "jump", "laser", "powerup" };
static int sample_flash[6];

/* white keys a s d f g h j k l = C D E F G A B C D, black keys w e t y u = C# D# F# G# A# */
static const char white_key[9] = { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' };
static const char *white_note[9] = { "C", "D", "E", "F", "G", "A", "B", "C", "D" };
static const int white_hz[9] = { 262, 294, 330, 349, 392, 440, 494, 523, 587 };
static const char black_key[5] = { 'w', 'e', 't', 'y', 'u' };
static const int black_hz[5] = { 277, 311, 370, 415, 466 };
static const int black_after[5] = { 0, 1, 3, 4, 5 }; /* index of the white key on their left */
static int white_flash[9], black_flash[5];

static void piano_press(int key)
{
    int i;
    for (i = 0; i < 9; i++) {
        if (key == white_key[i]) {
            play_note(note_wav[i]);
            white_flash[i] = 10;
        }
    }
    for (i = 0; i < 5; i++) {
        if (key == black_key[i]) {
            play_note(note_wav[9 + i]);
            black_flash[i] = 10;
        }
    }
}

static void draw_sound_scene(void)
{
    int i, x, row;
    char line[COLS + 1];

    box(2, 3, 11, 36, 0);
    put(" SAMPLES  MT2D_Play_Audio ", 2, 5);
    for (i = 0; i < 6; i++) {
        sprintf(line, "%d  %s", i + 1, sample_name[i]);
        put(line, 4 + i, 8);
        if (sample_flash[i] > 0) {
            cell(4 + i, 6, C_ARROW);
            cell(4 + i, 7, ' ');
            sample_flash[i]--;
        }
    }
    put("Up to 8 at once, .wav and .ogg", 11, 6);

    box(2, 41, 11, 36, 0);
    put(" PIANO  MT2D_PlayAudioFromMemory ", 2, 43);
    put("Play the keys of the piano below.", 4, 44);
    put("Each note is a tiny WAV that the", 5, 44);
    put("program makes in memory when it", 6, 44);
    put("starts, then plays like any other", 7, 44);
    put("sample (the PC speaker beep and", 8, 44);
    put("audio files are not mixed here).", 9, 44);

    /* piano: 9 white keys of 7 columns */
    x = 8;
    for (i = 0; i < 9; i++) {
        int c = x + i * 7;
        for (row = 0; row < 8; row++) {
            cell(14 + row, c, C_SV);
            if (white_flash[i] > 0) fill(14 + row, c + 1, 1, 6, C_SHADE2);
        }
        cell(22, c, C_SBL);
        fill(22, c + 1, 1, 6, C_SH);
        if (white_flash[i] > 0) white_flash[i]--;
        put(white_note[i], 21, c + 3);
        cell(20, c + 3, white_key[i] - 32);
    }
    for (row = 0; row < 8; row++) cell(14 + row, x + 9 * 7, C_SV);
    cell(22, x + 9 * 7, C_SBR);
    fill(13, x, 1, 9 * 7 + 1, C_SH);
    for (i = 0; i < 5; i++) {
        int c = x + (black_after[i] + 1) * 7 - 2;
        fill(14, c, 5, 5, black_flash[i] > 0 ? C_SHADE2 : C_BLOCK);
        if (black_flash[i] > 0) black_flash[i]--;
        cell(18, c + 2, black_key[i] - 32);
    }
    put("PIANO", 13, 8 + 29);
    chrome("SOUND LAB", "1-6: samples   A S D F G H J K L / W E T Y U: piano   ESC: menu");
}

/* ---------------------------------------------------------------------------------------------- */
/* music                                                                                          */
/* ---------------------------------------------------------------------------------------------- */

static void draw_music_scene(void)
{
    int i, r, h, bar, v, mm, ss;
    char line[COLS + 1];

    if (audio_unlocked && track >= 0 && !music_paused) {
        if (!Mix_PlayingMusic()) play_track(track); /* MT2D loops it a few times, start again when it ends */
        music_frames++;
    }

    box(2, 4, 7, 72, 0);
    put(" PLAYLIST  MT2D_Play_Music ", 2, 6);
    for (i = 0; i < TRACKS; i++) {
        sprintf(line, "%d  %s", i + 1, track_name[i]);
        put(line, 4 + i, 9);
        if (i == track) cell(4 + i, 7, music_paused ? '|' : C_ARROW);
    }

    /* visualizer: 32 levels, newest on the right, 2 columns each */
    box(9, 4, 13, 72, 0);
    for (i = 0; i < LEVELS; i++) {
        v = levels[(level_head + 1 + i) % LEVELS];
        if (music_paused) v = 0;
        h = v * 10 / 24000;
        if (h > 10) h = 10;
        for (bar = 0; bar < 2; bar++) {
            for (r = 0; r < h; r++) cell(20 - r, 7 + i * 2 + bar, r > 7 ? C_SHADE2 : C_BLOCK);
        }
    }
    put("signal", 10, 6);

    mm = music_frames / 3600;
    ss = (music_frames / 60) % 60;
    v = volume * 20 / 128;
    sprintf(line, "%s  %02d:%02d   volume [", track < 0 ? "stopped" : (music_paused ? "paused " : "playing"), mm, ss);
    put(line, 22, 6);
    for (i = 0; i < 20; i++) cell(22, 6 + (int)strlen(line) + i, i < v ? C_BLOCK : '.');
    cell(22, 6 + (int)strlen(line) + 20, ']');
    chrome("MUSIC PLAYER", "1-3: song   Space: pause   + / -: volume   ESC: menu");
}

/* ---------------------------------------------------------------------------------------------- */
/* about                                                                                          */
/* ---------------------------------------------------------------------------------------------- */

static void draw_about_scene(void)
{
    box(1, 2, 23, 76, 1);
    put("A small C framework for text based programs and games.", 3, 5);
    put("The screen is an 80x25 grid of characters, drawn on a real terminal or", 4, 5);
    put("an emulated one (an SDL window, the MS-DOS VGA mode, this web page).", 5, 5);
    put("It can also draw image sprites on top of the text.", 6, 5);

    put("Runs on:  Windows  Linux  macOS  MS-DOS  Android  Web (Emscripten)", 8, 5);
    put("Written in ANSI C (C89), so it also builds with DJGPP and Open Watcom.", 9, 5);

    put("What is in the box", 11, 5);
    put("message boxes, input boxes, menus      keyboard and joystick", 12, 7);
    put("PC speaker, WAV / OGG audio, IMF music  file container (AES)", 13, 7);
    put("ObjectCore: objects, sprites, states    Cscript: small scripts", 14, 7);

    put("Everything on the previous screens comes from these calls:", 16, 5);
    put("insert_string_on_display   WINDOW1[y][x]   Sprite_Render_on_Window", 17, 7);
    put("MT2D_Play_Audio   MT2D_Play_Music   MT2D_PlayAudioFromMemory", 18, 7);

    put("MIT license.  github.com/lucas-zimerman/MT2D", 21, 5);
    chrome("ABOUT", "ESC: menu");
}

/* ---------------------------------------------------------------------------------------------- */
/* input                                                                                          */
/* ---------------------------------------------------------------------------------------------- */

static void select_menu(int i)
{
    static const int target[MENU_ITEMS] = { SCENE_TEXT, SCENE_GFX, SCENE_SOUND, SCENE_MUSIC, SCENE_ABOUT };
    goto_scene(target[i]);
    sfx("blip.wav");
    if (target[i] == SCENE_GFX) {
        gfx_tick = 0;
        if (gfx_scene == 2) desert_reset();
    }
}

static void report_mute(void)
{
    EM_ASM({
        if (window.mt2dMuted) window.mt2dMuted($0);
    }, muted);
}

static void toggle_mute(void)
{
    muted = !muted;
    apply_volume();
    report_mute();
}

static void on_key(int key)
{
    int n;

    if (key >= 'A' && key <= 'Z') key += 32;
    if (key == 'm') {
        toggle_mute();
        return;
    }
    if (key == escape_pressed && scene != SCENE_MENU) {
        goto_scene(SCENE_MENU);
        return;
    }
    if (scene == SCENE_MENU) {
        if (key == K_UP || key == 'w') menu_sel = (menu_sel + MENU_ITEMS - 1) % MENU_ITEMS;
        else if (key == K_DOWN || key == 's') menu_sel = (menu_sel + 1) % MENU_ITEMS;
        else if (key == enter_pressed || key == ' ') select_menu(menu_sel);
        else if (key >= '1' && key <= '5') {
            menu_sel = key - '1';
            select_menu(menu_sel);
        }
    } else if (scene == SCENE_TEXT) {
        if (key >= '1' && key <= '4') text_fx = key - '0';
        else if (key == K_RIGHT || key == ' ') text_fx = text_fx % FX_COUNT + 1;
        else if (key == K_LEFT) text_fx = (text_fx + FX_COUNT - 2) % FX_COUNT + 1;
    } else if (scene == SCENE_GFX) {
        if (gfx_scene == 2 && d_over && key == enter_pressed) {
            desert_reset();
        } else if (key >= '1' && key <= '3') {
            if (key == '2' && gfx_scene != 2) desert_reset();
            gfx_scene = key - '0';
        } else if (key == ' ' && gfx_scene != 2) {
            gfx_scene = gfx_scene % 3 + 1;
            if (gfx_scene == 2) desert_reset();
        }
    } else if (scene == SCENE_SOUND) {
        n = key - '1';
        if (n >= 0 && n < 6) {
            sample_flash[n] = 12;
            sfx(sample_file[n]);
        } else {
            piano_press(key);
        }
    } else if (scene == SCENE_MUSIC) {
        if (key >= '1' && key <= '3') play_track(key - '1');
        else if (key == ' ') {
            if (track < 0) play_track(0);
            else if (music_paused) {
                Mix_ResumeMusic();
                music_paused = 0;
            } else {
                Mix_PauseMusic();
                music_paused = 1;
            }
        } else if (key == '+' || key == '=' || key == K_UP) {
            volume = volume + 16 > 128 ? 128 : volume + 16;
            apply_volume();
        } else if (key == '-' || key == K_DOWN) {
            volume = volume - 16 < 0 ? 0 : volume - 16;
            apply_volume();
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
            if (key == arrow_up_pressed) key = K_UP;
            else if (key == arrow_down_pressed) key = K_DOWN;
            else if (key == arrow_left_pressed) key = K_LEFT;
            else if (key == arrow_right_pressed) key = K_RIGHT;
            else continue;
        }
        if (!audio_unlocked) {
            unlock_audio(); /* the first key or click is what lets the browser play sound */
            if (key == enter_pressed || key == ' ') continue; /* do not open an item with the key that only started the sound */
        }
        on_key(key);
    }
}

/* ---------------------------------------------------------------------------------------------- */
/* main loop                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

static void frame(void)
{
    if (!web_step_due(60)) return; /* see common/web_timing.h */
    MT2D_Joystick_Update(); /* reads the window events and the held keys (the graphics scenes move with them) */
    read_keys();
    MT2D_Clear_Main_Window(); /* also clears the images of the last frame */

    if (scene == SCENE_MENU) draw_menu();
    else if (scene == SCENE_TEXT) draw_text_scene();
    else if (scene == SCENE_GFX) draw_gfx_scene();
    else if (scene == SCENE_SOUND) draw_sound_scene();
    else if (scene == SCENE_MUSIC) draw_music_scene();
    else draw_about_scene();
    MT2D_Draw_Window(DISPLAY_WINDOW1);
    frame_count++;
}

int main(int argc, char **argv)
{
    int i;

    (void)argc;
    (void)argv;
    srand((unsigned)time(NULL));
    /* a fixed 1280x720 canvas: the page scales it with CSS, the browser window size does not matter */
    MT2D_Set_Settings(MT2D_SDL_SET_WINDOW_SIZE, 1280, 720);
    MT2D_Init();
    MT2D_Set_Settings(MT2D_SDL_SET_WINDOWED, 0, 0);
    MT2D_Joystick_Init();
    MT2D_Audio_Init();
    Mix_SetPostMix(post_mix, NULL);
    for (i = 0; i < 9; i++) note_wav[i] = make_note(white_hz[i]);
    for (i = 0; i < 5; i++) note_wav[9 + i] = make_note(black_hz[i]);
    for (i = 0; i < STARS; i++) star_reset(i, 1.0 + (rand() % 1000) / 100.0);
    load_images();
    apply_volume();
    report_mute();
    set_page_title();
    /* 0 = use requestAnimationFrame, 1 = do not return from main (the loop keeps the program alive) */
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}

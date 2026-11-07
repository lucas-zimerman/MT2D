/*
   Desert: a monster truck runs from a sand worm. ESC quits.

   A small "demoscene" that shows what the object scripts (ObjectCore_LoadScript / ObjectCore_Load)
   can do. The scenery is drawn here in plain C, everything that moves is a script (the scripts folder):

     truck    cruises on two lanes, drifts to the other lane when the radar shouts Dodge, smokes
     worm     chases under the sand, rears up and leaps in an arc, lands where the truck was
     radar    an invisible box that follows the truck, GotoStateIfHitObject(global Worm, ...)
     title, alert
              baked pixel font PNGs: the title drops in, the banner blinks when the radar fires
     dust, smoke, grain, segment
              particles made with CreateObject, they remove themselves with Stop

   The game and the scripts talk through globals (ObjectCore_AddGlobalVar / AddGlobalObject):
   Speed (the scripts set it, the scenery scrolls with it), Dodge, Alert, Lane and TruckY.
   The images are made by tools/make_assets.py (cyan = transparent), the music by tools/make_music.py, the sound effects by tools/make_sfx.py.
*/
#include <stdio.h>
#include <stdlib.h>
#include "example.h"
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Object.h> /* keep this one before State.h (they include each other) */
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/ObjectGlobals.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>

#define SCREEN_W 320
#define SCREEN_H 240

static void fail(const char *what, const char *why)
{
    char text[200];
    sprintf(text, "%s: %s", what, why);
    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)text, 10, 2, DISPLAY_WINDOW1);
    insert_string_on_display((char *)"Run the example from its bin folder. Press a key.", 12, 2, DISPLAY_WINDOW1);
    MT2D_Draw_Window(DISPLAY_WINDOW1);
    MT2D_Keyboard_keytouched();
    exit(1);
}

static Sprite *load_layer(const char *name)
{
    Sprite *s = Load_Sprite_Image((char *)name, 0, 0);
    if (!s) fail("Could not load the image", name);
    s->ScaleX = s->SizeX; /* 1 image pixel = 1 screen pixel */
    s->ScaleY = s->SizeY;
    return s;
}

/* Draws a layer twice so it scrolls forever, offset is in pixels. */
static void scroll_layer(Sprite *s, int offset, int y)
{
    int x = -(offset % SCREEN_W);
    Sprite_Render_on_Window(s, DISPLAY_WINDOW1, x, y);
    Sprite_Render_on_Window(s, DISPLAY_WINDOW1, x + SCREEN_W, y);
}

static Object *load_script(const char *file)
{
    char error[200];
    Object *o = ObjectCore_LoadScript(file, error, (int)sizeof(error));
    if (!o) fail(file, error);
    return o;
}

/* a game var the scripts can read and write as "global Name" */
static MT2D_VAR *add_var(const char *name, int value)
{
    MT2D_VAR *v = MT2D_Object_Create_Var_Int(name, value);
    if (!v || !ObjectCore_AddGlobalVar(v)) fail("Could not register the global", name);
    return v;
}

int main(int argc, char **argv)
{
    Sprite *sky, *mesas, *dunes, *ground, *foreground;
    MT2D_VAR *speed;
    ObjectScene *scene;
    Object *worm, *radar, *truck, *title, *alert;
    int scroll = 0, key;

    (void)argc;
    (void)argv;
    MT2D_Init();
    MT2D_Audio_Init(); /* does nothing without SDL2_mixer, the music is started by title.txt, sound effects by the other scripts */

    sky = load_layer("sky.png");
    mesas = load_layer("mesas.png");
    dunes = load_layer("dunes.png");
    ground = load_layer("ground.png");
    foreground = load_layer("foreground.png");

    /* the scripts are compiled when the example starts, ship .mto files made by cscriptc instead
       (ObjectCore_Load) when you do not want the compiler in your game */
    speed = add_var("Speed", 4);
    add_var("Dodge", 0);
    add_var("Lane", 0);
    add_var("Alert", 0);
    add_var("TruckY", 140);

    /* models for CreateObject: they are not in the scene, only their states are used */
    if (!ObjectCore_AddGlobalObject("Dust", load_script("dust.txt"))) fail("global object", "Dust");
    if (!ObjectCore_AddGlobalObject("Smoke", load_script("smoke.txt"))) fail("global object", "Smoke");
    if (!ObjectCore_AddGlobalObject("Grain", load_script("grain.txt"))) fail("global object", "Grain");
    if (!ObjectCore_AddGlobalObject("Segment", load_script("segment.txt"))) fail("global object", "Segment");

    /* the objects of the scene, drawn in this order (what CreateObject makes is drawn last) */
    worm = load_script("worm.txt");
    if (!ObjectCore_AddGlobalObject("Worm", worm)) fail("global object", "Worm");
    radar = load_script("radar.txt"); /* needs Worm to exist */
    truck = load_script("truck.txt");
    title = load_script("title.txt");
    alert = load_script("alert.txt");

    scene = ObjectScene_CreateEmpty();
    ObjectScene_Add(scene, worm);
    ObjectScene_Add(scene, radar);
    ObjectScene_Add(scene, truck);
    ObjectScene_Add(scene, title); /* the texts are baked PNGs shown by scripts too */
    ObjectScene_Add(scene, alert);

    for (;;) {
        MT2D_Clear_Main_Window(); /* also clears the images of the last frame */

        /* back to front, the farther the slower */
        Sprite_Render_on_Window(sky, DISPLAY_WINDOW1, 0, 0);
        scroll_layer(mesas, scroll / 16, 74);
        scroll_layer(dunes, scroll / 5, 100);
        scroll_layer(ground, scroll, 126);
        ObjectScene_Render(scene);
        scroll_layer(foreground, scroll * 2, SCREEN_H - 40);
        MT2D_Draw_Window(DISPLAY_WINDOW1);

        while (MT2D_Keyboard_touched()) {
            key = MT2D_Keyboard_keytouched();
            if (key == escape_pressed) {
                MT2D_Audio_Close();
                return 0;
            }
        }

        ObjectScene_Goto_NextSteps(scene); /* runs every script one step */
        scroll += speed->Data.i;
        MT2D_System_Delay(30);
    }
}

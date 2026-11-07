/*
   Cscript: tiny "scripts" attached to the frames of an object state.

   Every frame of a state has a sprite, a time to wait and (optionally) a Cscript function that runs
   when the frame starts. Each ball below is a little program (Pos_X, Acel_X are variables that every
   object has):

       state GoRight                           state TurnLeft
         frame 0: Move                           frame 0: count_turn()   <- our own C function
         frame 1: Move                           frame 1: Acel_X = -1
         frame 2: if Pos_X >= 58 goto TurnLeft  frame 2: goto GoLeft

   (GoLeft / TurnRight are the same thing mirrored). The rocket waits, sometimes decides to dash
   (random jump) and then teleports back to the start (Pos_X = 2).
   The name of the state each object is running is printed at the right. Press ESC to quit.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "example.h"
#include <MT2D/ObjectCore/Object.h> /* keep this one before State.h (they include each other) */
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/STDFunctions.h>

#define BALLS 4

static int turns = 0;

/* Our own script function: any "void f(Object *)" can be a script. */
static void count_turn(Object *self)
{
    (void)self;
    turns++;
}

/* A var that only holds a name: the name of a state, or of a var of the object running the script. */
static MT2D_VAR *named(const char *name)
{
    return MT2D_Object_Create_Var_Pointer(name);
}

static MT2D_VAR *number(int value)
{
    return MT2D_Object_Create_Var_Int("Number", value);
}

/* A state with: Move, Move, "if Pos_X <cmp> limit goto turn_state". */
static ObjectState *make_walk_state(const char *name, Sprite *a, Sprite *b, int delay, const char *turn_state, int limit, bool going_right)
{
    ObjectState *state = MT2D_OBJECT_CREATE_STATE((char *)name);
    MT2D_ObjectState_ADD(state, a, delay, Cscript_New2(Cscript_Move));
    MT2D_ObjectState_ADD(state, b, delay, Cscript_New2(Cscript_Move));
    MT2D_ObjectState_ADD(state, b, 0,
        Cscript_New3(MT2D_VAR_Create_Matrix3(named(turn_state), named("Pos_X"), number(limit)),
                     going_right ? Cscript_JumpToStateIfVarHigherEq : Cscript_JumpToStateIfVarLowerEq));
    return state;
}

/* A state that runs once: count it, change the direction, go to the next state. */
static ObjectState *make_turn_state(const char *name, Sprite *a, int new_speed, const char *next_state)
{
    ObjectState *state = MT2D_OBJECT_CREATE_STATE((char *)name);
    MT2D_ObjectState_ADD(state, a, 0, Cscript_New2(count_turn));
    MT2D_ObjectState_ADD(state, a, 0, Cscript_New3(MT2D_VAR_Create_Matrix2(named("Acel_X"), number(new_speed)), Cscript_Set_Var));
    MT2D_ObjectState_ADD(state, a, 0, Cscript_New3(MT2D_VAR_Create_Matrix1(named(next_state)), Cscript_Object_SetState));
    return state;
}

/* A ball that walks to the right/left and turns around at the borders. delay = slowness. */
static ObjectState **make_ball_states(Sprite *a, Sprite *b, int delay)
{
    ObjectState **states = (ObjectState **)malloc(4 * sizeof(ObjectState *));
    states[0] = make_walk_state("GoRight", a, b, delay, "TurnLeft", 58, true);
    states[1] = make_turn_state("TurnLeft", a, -1, "GoLeft");
    states[2] = make_walk_state("GoLeft", a, b, delay, "TurnRight", 2, false);
    states[3] = make_turn_state("TurnRight", a, 1, "GoRight");
    return states;
}

/* The rocket waits, sometimes "decides" to dash, then goes back to the start. */
static ObjectState **make_rocket_states(Sprite *idle, Sprite *fly)
{
    ObjectState **states = (ObjectState **)malloc(3 * sizeof(ObjectState *));

    states[0] = MT2D_OBJECT_CREATE_STATE((char *)"Wait");
    MT2D_ObjectState_ADD(states[0], idle, 3, 0);
    /* rand() % 8 <= 1: 2 in 8 chance to jump to the "Dash" state every time this frame runs */
    MT2D_ObjectState_ADD(states[0], idle, 0, Cscript_New3(MT2D_VAR_Create_Matrix3(named("Dash"), number(1), number(8)),
                                                         Cscript_JumpToStateIfRandom));

    /* (a random jump lands on frame 1 of the new state, so frame 0 is only a placeholder) */
    states[1] = MT2D_OBJECT_CREATE_STATE((char *)"Dash");
    MT2D_ObjectState_ADD(states[1], fly, 0, 0);
    MT2D_ObjectState_ADD(states[1], fly, 1, Cscript_New2(Cscript_Move));
    MT2D_ObjectState_ADD(states[1], fly, 0, Cscript_New3(MT2D_VAR_Create_Matrix3(named("Reset"), named("Pos_X"), number(54)),
                                                        Cscript_JumpToStateIfVarHigherEq));

    states[2] = MT2D_OBJECT_CREATE_STATE((char *)"Reset");
    MT2D_ObjectState_ADD(states[2], idle, 0, Cscript_New3(MT2D_VAR_Create_Matrix2(named("Pos_X"), number(2)), Cscript_Set_Var));
    MT2D_ObjectState_ADD(states[2], idle, 0, Cscript_New3(MT2D_VAR_Create_Matrix1(named("Wait")), Cscript_Object_SetState));
    return states;
}

static Sprite *make_sprite(int w, int h, const char *cells)
{
    char *copy = (char *)malloc((size_t)w * h);
    Sprite *s;
    int i;
    for (i = 0; i < w * h; i++)
        copy[i] = cells[i] == ' ' ? 0 : cells[i];
    s = Sprite_Create_Ascii_From_Cells(w, h, copy);
    free(copy);
    return s;
}

int main(int argc, char **argv)
{
    ObjectScene *scene;
    Object *balls[BALLS];
    Object *rocket;
    Sprite *ball_a = make_sprite(3, 1, "(o)");
    Sprite *ball_b = make_sprite(3, 1, "(O)");
    Sprite *rocket_idle = make_sprite(6, 1, "=[__]>");
    Sprite *rocket_fly = make_sprite(8, 1, "=[__]>>>");
    ObjectState **rocket_states;
    int i, key;
    char text[80];

    (void)argc;
    (void)argv;
    MT2D_Init();

    scene = ObjectScene_CreateEmpty();
    for (i = 0; i < BALLS; i++) {
        balls[i] = Object_Create(false, false, 3, 1, 2 + i * 9, 4 + i * 2, make_ball_states(ball_a, ball_b, i + 1), 4);
        ObjectAcelX(balls[i]) = 1; /* starts walking to the right */
        ObjectScene_Add(scene, balls[i]);
    }
    rocket_states = make_rocket_states(rocket_idle, rocket_fly);
    rocket = Object_Create(false, false, 6, 1, 2, 17, rocket_states, 3);
    ObjectAcelX(rocket) = 3;
    ObjectScene_Add(scene, rocket);

    for (;;) {
        MT2D_Clear_Main_Window();
        insert_string_on_display((char *)"MT2D Cscript: scripts running on object states", 1, 16, DISPLAY_WINDOW1);
        ObjectScene_Render(scene);
        for (i = 0; i < BALLS; i++) { /* show which state each object is in */
            sprintf(text, "ball %d: %-7s", i + 1, balls[i]->State[balls[i]->ActualState]->Name);
            insert_string_on_display(text, 4 + i * 2, 63, DISPLAY_WINDOW1);
        }
        sprintf(text, "rocket: %-5s", rocket->State[rocket->ActualState]->Name);
        insert_string_on_display(text, 17, 63, DISPLAY_WINDOW1);
        sprintf(text, "Turns counted by our own function: %d", turns);
        insert_string_on_display(text, 21, 2, DISPLAY_WINDOW1);
        insert_string_on_display((char *)"ESC: quit", 23, 35, DISPLAY_WINDOW1);
        MT2D_Draw_Window(DISPLAY_WINDOW1);

        if (MT2D_Keyboard_touched()) {
            key = MT2D_Keyboard_keytouched();
            if (key == escape_pressed) break;
        }
        ObjectScene_Goto_NextSteps(scene);
        MT2D_System_Delay(60);
    }
    return 0;
}

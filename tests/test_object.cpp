// Tests for MT2D/ObjectCore/Object.cpp (Object and ObjectScene)
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <stdlib.h>
#include <string.h>

static int count_a = 0, count_b = 0;
static void fn_a(Object *) { count_a++; }
static void fn_b(Object *) { count_b++; }

static Sprite *make_sprite(const char *text) {
    Sprite *s = (Sprite *)malloc(sizeof(Sprite));
    s->SizeX = (int)strlen(text);
    s->SizeY = 1;
    s->ScaleX = s->ScaleY = 1;
    s->type = 0;
    s->refCount = 0;
    s->Data = (char **)malloc(sizeof(char *));
    s->Data[0] = (char *)malloc(s->SizeX + 1);
    strcpy(s->Data[0], text);
    return s;
}

static void clear_window() { memset(WINDOW1, 0, sizeof(WINDOW1)); }

static ObjectState **one_state(ObjectState *st) {
    ObjectState **states = (ObjectState **)malloc(sizeof(ObjectState *));
    states[0] = st;
    return states;
}

TEST(create_object_initialises_internal_vars) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Idle");
    MT2D_ObjectState_ADD(st, 0, 4, 0);
    Object *o = Object_Create(true, false, 3, 2, 10, 5, one_state(st), 1);

    CHECK(o->Solid);
    CHECK(!o->RenderOnly);
    CHECK(o->Enabled);
    CHECK(!o->CanDelete);
    CHECK(o->MyScene == 0);
    CHECK_EQ(o->States_Count, 1);
    CHECK_EQ(o->ActualState, 0);
    CHECK_EQ(o->ActualFrame, 0);
    CHECK_EQ(o->ActualFrameWait, 4);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index);

    CHECK_EQ(ObjectPosX(o), 10);
    CHECK_EQ(ObjectPosY(o), 5);
    CHECK_EQ(ObjectSizeX(o), 3);
    CHECK_EQ(ObjectSizeY(o), 2);
    CHECK_EQ(ObjectAcelX(o), 0);
    CHECK_EQ(ObjectAcelY(o), 0);
    CHECK_STR_EQ(o->User_Vars[Object_PosX_Var_Index].Name, "Pos_X");
    CHECK_STR_EQ(o->User_Vars[Object_AcelY_Var_Index].Name, "Acel_Y");

    ObjectPosX(o) = 11;
    CHECK_EQ(*ObjectPosXRef(o), 11);
}

TEST(add_variable_and_variables) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Idle");
    MT2D_ObjectState_ADD(st, 0, 0, 0);
    Object *o = Object_Create(false, false, 1, 1, 0, 0, one_state(st), 1);

    MT2D_VAR *health = MT2D_Object_Create_Var_Int("Health", 20);
    Object_Add_Variable(o, health);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index + 1);
    CHECK_STR_EQ(o->User_Vars[Object_First_User_Var_Index].Name, "Health");
    CHECK_EQ(*(int *)o->User_Vars[Object_First_User_Var_Index].Data, 20);

    MT2D_VAR extra[2];
    extra[0] = *MT2D_Object_Create_Var_Int("Ammo", 3);
    extra[1] = *MT2D_Object_Create_Var_Int("Score", 4);
    Object_Add_Variables(o, extra, 2);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index + 3);
    CHECK_STR_EQ(o->User_Vars[Object_First_User_Var_Index + 1].Name, "Ammo");
    CHECK_STR_EQ(o->User_Vars[Object_First_User_Var_Index + 2].Name, "Score");
    CHECK_EQ(*(int *)o->User_Vars[Object_First_User_Var_Index + 2].Data, 4);
}

TEST(next_step_waits_then_runs_functions) {
    count_a = count_b = 0;
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Loop");
    MT2D_ObjectState_ADD(st, 0, 0, Cscript_New2(fn_a)); // runs immediately
    MT2D_ObjectState_ADD(st, 0, 2, Cscript_New2(fn_b)); // waits 2 steps
    Object *o = Object_Create(false, false, 1, 1, 0, 0, one_state(st), 1);

    Object_Goto_NextStep(o);
    CHECK_EQ(count_a, 1);
    CHECK_EQ(count_b, 0);
    CHECK_EQ(o->ActualFrame, 1);
    CHECK_EQ(o->ActualFrameWait, 2);

    Object_Goto_NextStep(o);
    CHECK_EQ(o->ActualFrameWait, 1);
    Object_Goto_NextStep(o);
    CHECK_EQ(o->ActualFrameWait, 0);
    CHECK_EQ(count_b, 0);

    // frame 1 runs, the state wraps and frame 0 (no wait) runs again in the same step
    Object_Goto_NextStep(o);
    CHECK_EQ(count_b, 1);
    CHECK_EQ(count_a, 2);
    CHECK_EQ(o->ActualFrame, 1);
}

TEST(next_step_handles_missing_function) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Quiet");
    MT2D_ObjectState_ADD(st, 0, 0, 0);
    MT2D_ObjectState_ADD(st, 0, 1, 0);
    Object *o = Object_Create(false, false, 1, 1, 0, 0, one_state(st), 1);
    Object_Goto_NextStep(o); // must not crash
    CHECK_EQ(o->ActualFrame, 1);
}

TEST(next_step_moves_object_with_cscript_move) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Move");
    MT2D_ObjectState_ADD(st, 0, 1, Cscript_New2(Cscript_Move));
    Object *o = Object_Create(false, false, 1, 1, 0, 0, one_state(st), 1);
    ObjectAcelX(o) = 2;
    ObjectAcelY(o) = -1;
    // wait of 1: needs 1 step to count down and one to run
    Object_Goto_NextStep(o);
    Object_Goto_NextStep(o);
    CHECK_EQ(ObjectPosX(o), 2);
    CHECK_EQ(ObjectPosY(o), -1);
}

TEST(render_draws_sprite_at_object_position) {
    clear_window();
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Show");
    MT2D_ObjectState_ADD(st, make_sprite("AB"), 0, 0);
    Object *o = Object_Create(false, false, 2, 1, 4, 3, one_state(st), 1);
    Object_Render(o);
    CHECK_EQ(WINDOW1[3][4], 'A');
    CHECK_EQ(WINDOW1[3][5], 'B');
    CHECK_EQ(WINDOW1[3][6], 0);
}

TEST(render_skips_disabled_object) {
    clear_window();
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Show");
    MT2D_ObjectState_ADD(st, make_sprite("AB"), 0, 0);
    Object *o = Object_Create(false, false, 2, 1, 4, 3, one_state(st), 1);
    o->Enabled = false;
    Object_Render(o);
    CHECK_EQ(WINDOW1[3][4], 0);
}

TEST(scene_add_tracks_objects) {
    ObjectScene *scene = ObjectScene_CreateEmpty();
    CHECK_EQ(scene->Count, 0);
    CHECK(scene->ObjectGroup == 0);

    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"S");
    MT2D_ObjectState_ADD(st, 0, 0, 0);
    ObjectState **states = one_state(st);
    Object *a = Object_Create(false, false, 1, 1, 0, 0, states, 1);
    Object *b = Object_Create(false, false, 1, 1, 0, 0, states, 1);
    ObjectScene_Add(scene, a);
    ObjectScene_Add(scene, b);
    CHECK_EQ(scene->Count, 2);
    CHECK(scene->ObjectGroup[0] == a);
    CHECK(scene->ObjectGroup[1] == b);
    CHECK(a->MyScene == scene);
    CHECK(b->MyScene == scene);
}

TEST(scene_render_draws_all_objects) {
    clear_window();
    ObjectScene *scene = ObjectScene_CreateEmpty();
    for (int i = 0; i < 3; i++) {
        ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"S");
        MT2D_ObjectState_ADD(st, make_sprite("#"), 0, 0);
        ObjectScene_Add(scene, Object_Create(false, false, 1, 1, i * 2, 1, one_state(st), 1));
    }
    ObjectScene_Render(scene);
    CHECK_EQ(WINDOW1[1][0], '#');
    CHECK_EQ(WINDOW1[1][2], '#');
    CHECK_EQ(WINDOW1[1][4], '#');
    CHECK_EQ(WINDOW1[1][1], 0);
}

TEST(scene_next_steps_removes_objects_marked_for_deletion) {
    ObjectScene *scene = ObjectScene_CreateEmpty();
    ObjectState *keep = MT2D_OBJECT_CREATE_STATE((char *)"Keep");
    MT2D_ObjectState_ADD(keep, 0, 5, 0);
    ObjectState *die = MT2D_OBJECT_CREATE_STATE((char *)"Die");
    MT2D_ObjectState_ADD(die, 0, 0, Cscript_New2(Cscript_RemoveObject));
    // A state whose frames all have no wait never stops looping inside
    // Object_Goto_NextStep, so end it with a frame that waits.
    MT2D_ObjectState_ADD(die, 0, 5, 0);

    Object *a = Object_Create(false, false, 1, 1, 0, 0, one_state(keep), 1);
    Object *b = Object_Create(false, false, 1, 1, 0, 0, one_state(die), 1);
    Object *c = Object_Create(false, false, 1, 1, 0, 0, one_state(keep), 1);
    ObjectScene_Add(scene, a);
    ObjectScene_Add(scene, b);
    ObjectScene_Add(scene, c);

    ObjectScene_Goto_NextSteps(scene);
    CHECK_EQ(scene->Count, 2);
    CHECK(scene->ObjectGroup[0] == a);
    CHECK(scene->ObjectGroup[1] == c);
}

TEST(delete_object_accepts_null_and_frees_states_on_request) {
    Object_Delete(0, false);
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"S");
    MT2D_ObjectState_ADD(st, make_sprite("x"), 0, 0);
    Object *o = Object_Create(false, false, 1, 1, 0, 0, one_state(st), 1);
    Object_Delete(o, true);
    CHECK(true); // reaching this line means no crash / double free
}

MT2D_TEST_MAIN()

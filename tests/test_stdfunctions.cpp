// Tests for MT2D/ObjectCore/STDFunctions.cpp (the built in Cscript functions)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <stdlib.h>
#include <string.h>

// ---- helpers -------------------------------------------------------------

// Builds an object with one one-frame state per name. The first frame of every
// state waits `wait` ticks.
static Object *make_object(int x, int y, int sx, int sy, bool solid, int stateCount, const char **names, int wait = 3) {
    ObjectState **states = (ObjectState **)malloc(stateCount * sizeof(ObjectState *));
    for (int i = 0; i < stateCount; i++) {
        states[i] = MT2D_OBJECT_CREATE_STATE((char *)names[i]);
        MT2D_ObjectState_ADD(states[i], 0, wait + i, 0);
    }
    return Object_Create(solid, false, sx, sy, x, y, states, stateCount);
}

static Object *simple_object(int x = 0, int y = 0) {
    static const char *names[] = {"Idle"};
    return make_object(x, y, 1, 1, false, 1, names);
}

static Object *three_state_object() {
    static const char *names[] = {"Idle", "Moving", "Dead"};
    return make_object(0, 0, 1, 1, false, 3, names);
}

// A var whose Name selects a state / an object var and does not hold data.
static MT2D_VAR *named(const char *name) { return MT2D_Object_Create_Var_Pointer(name); }
static MT2D_VAR *integer(int v) { return MT2D_Object_Create_Var_Int("Const", v); }

static MT2D_VAR *add_user_var(Object *o, const char *name, int value) {
    Object_Add_Variable(o, MT2D_Object_Create_Var_Int(name, value));
    return &o->User_Vars[o->User_Vars_Count - 1];
}

// ---- math ----------------------------------------------------------------

TEST(var_add_to_object_var) {
    Object *o = simple_object(10, 0);
    MT2D_VAR *vars[2] = {named("Pos_X"), integer(5)};
    Cscript_VAR_ADD(o, vars);
    CHECK_EQ(ObjectPosX(o), 15);
}

TEST(var_add_object_var_to_plain_var) {
    Object *o = simple_object(7, 0);
    MT2D_VAR *target = integer(1);
    MT2D_VAR *vars[2] = {target, named("Pos_X")};
    Cscript_VAR_ADD(o, vars);
    CHECK_EQ(*(int *)target->Data, 8);
}

TEST(var_add_two_object_vars) {
    Object *o = simple_object(3, 4);
    MT2D_VAR *vars[2] = {named("Pos_X"), named("Pos_Y")};
    Cscript_VAR_ADD(o, vars);
    CHECK_EQ(ObjectPosX(o), 7);
    CHECK_EQ(ObjectPosY(o), 4);
}

TEST(var_add_plain_vars) {
    Object *o = simple_object();
    MT2D_VAR *vars[2] = {integer(2), integer(3)};
    Cscript_VAR_ADD(o, vars);
    CHECK_EQ(*(int *)vars[0]->Data, 5);
}

TEST(var_sub) {
    Object *o = simple_object(10, 10);
    MT2D_VAR *vars[2] = {named("Pos_X"), integer(4)};
    Cscript_VAR_SUB(o, vars);
    CHECK_EQ(ObjectPosX(o), 6);

    MT2D_VAR *plain[2] = {integer(9), integer(2)};
    Cscript_VAR_SUB(o, plain);
    CHECK_EQ(*(int *)plain[0]->Data, 7);

    MT2D_VAR *objs[2] = {named("Pos_X"), named("Pos_Y")};
    Cscript_VAR_SUB(o, objs);
    CHECK_EQ(ObjectPosX(o), -4);
}

TEST(var_inc_and_dec) {
    Object *o = simple_object(5, 0);
    MT2D_VAR *vars[1] = {named("Pos_X")};
    Cscript_VAR_INC(o, vars);
    CHECK_EQ(ObjectPosX(o), 6);
    Cscript_VAR_DEC(o, vars);
    Cscript_VAR_DEC(o, vars);
    CHECK_EQ(ObjectPosX(o), 4);

    MT2D_VAR *plain[1] = {integer(10)};
    Cscript_VAR_INC(o, plain);
    CHECK_EQ(*(int *)plain[0]->Data, 11);
    Cscript_VAR_DEC(o, plain);
    CHECK_EQ(*(int *)plain[0]->Data, 10);
}

// ---- variables -----------------------------------------------------------

TEST(set_var_object_var_from_constant) {
    Object *o = simple_object(1, 1);
    MT2D_VAR *vars[2] = {named("Pos_Y"), integer(42)};
    Cscript_Set_Var(o, vars);
    CHECK_EQ(ObjectPosY(o), 42);
    CHECK_EQ(ObjectPosX(o), 1);
}

TEST(set_var_object_var_from_object_var) {
    Object *o = simple_object(8, 1);
    MT2D_VAR *vars[2] = {named("Pos_Y"), named("Pos_X")};
    Cscript_Set_Var(o, vars);
    CHECK_EQ(ObjectPosY(o), 8);
}

TEST(set_var_plain_var_from_object_var) {
    Object *o = simple_object(33, 0);
    MT2D_VAR *dst = integer(0);
    MT2D_VAR *vars[2] = {dst, named("Pos_X")};
    Cscript_Set_Var(o, vars);
    CHECK_EQ(*(int *)dst->Data, 33);
}

TEST(set_var_plain_from_plain) {
    Object *o = simple_object();
    MT2D_VAR *vars[2] = {integer(0), integer(5)};
    Cscript_Set_Var(o, vars);
    CHECK_EQ(*(int *)vars[0]->Data, 5);
}

TEST(teleport_to_coord) {
    Object *o = simple_object(1, 1);
    MT2D_VAR *vars[2] = {integer(40), integer(12)};
    Cscript_TeleportToCoord(o, vars);
    CHECK_EQ(ObjectPosX(o), 40);
    CHECK_EQ(ObjectPosY(o), 12);
}

TEST(move_applies_acceleration) {
    Object *o = simple_object(10, 10);
    ObjectAcelX(o) = 3;
    ObjectAcelY(o) = -2;
    Cscript_Move(o);
    CHECK_EQ(ObjectPosX(o), 13);
    CHECK_EQ(ObjectPosY(o), 8);
    Cscript_Move(o);
    CHECK_EQ(ObjectPosX(o), 16);
    CHECK_EQ(ObjectPosY(o), 6);
}

TEST(remove_object_marks_for_deletion) {
    Object *o = simple_object();
    CHECK(!o->CanDelete);
    Cscript_RemoveObject(o);
    CHECK(o->CanDelete);
}

// ---- states --------------------------------------------------------------

TEST(set_state_by_name) {
    Object *o = three_state_object();
    MT2D_VAR *vars[1] = {named("Dead")};
    Cscript_Object_SetState(o, vars);
    CHECK_EQ(o->ActualState, 2);
    CHECK_EQ(o->ActualFrame, -1); // incremented by the caller right after
    CHECK_EQ(o->ActualFrameWait, o->State[2]->WaitSprites[0]);
}

TEST(set_state_unknown_name_changes_nothing) {
    Object *o = three_state_object();
    MT2D_VAR *vars[1] = {named("Nope")};
    Cscript_Object_SetState(o, vars);
    CHECK_EQ(o->ActualState, 0);
    CHECK_EQ(o->ActualFrame, 0);
}

TEST(jump_to_state_if_var_lower_eq) {
    Object *o = three_state_object();
    MT2D_VAR *yes[3] = {named("Moving"), integer(3), integer(3)};
    Cscript_JumpToStateIfVarLowerEq(o, yes);
    CHECK_EQ(o->ActualState, 1);

    Object *p = three_state_object();
    MT2D_VAR *no[3] = {named("Moving"), integer(4), integer(3)};
    Cscript_JumpToStateIfVarLowerEq(p, no);
    CHECK_EQ(p->ActualState, 0);
}

TEST(jump_to_state_if_var_higher_eq) {
    Object *o = three_state_object();
    MT2D_VAR *yes[3] = {named("Dead"), integer(5), integer(5)};
    Cscript_JumpToStateIfVarHigherEq(o, yes);
    CHECK_EQ(o->ActualState, 2);

    Object *p = three_state_object();
    MT2D_VAR *no[3] = {named("Dead"), integer(1), integer(5)};
    Cscript_JumpToStateIfVarHigherEq(p, no);
    CHECK_EQ(p->ActualState, 0);
}

TEST(jump_to_state_if_var_eq) {
    Object *o = three_state_object();
    // Vars: [0] state, [1] expected value, [2] value to check
    MT2D_VAR *yes[3] = {named("Dead"), integer(7), integer(7)};
    Cscript_JumpToStateIfVarEq(o, yes);
    CHECK_EQ(o->ActualState, 2);

    Object *p = three_state_object();
    MT2D_VAR *no[3] = {named("Dead"), integer(7), integer(8)};
    Cscript_JumpToStateIfVarEq(p, no);
    CHECK_EQ(p->ActualState, 0);
}

TEST(jump_to_state_compares_object_vars) {
    Object *o = three_state_object();
    MT2D_VAR *vars[3] = {named("Moving"), integer(0), named("Pos_X")};
    Cscript_JumpToStateIfVarEq(o, vars); // Pos_X == 0
    CHECK_EQ(o->ActualState, 1);
}

TEST(jump_to_state_if_random_always_with_max_one) {
    Object *o = three_state_object();
    // rand() % 1 == 0 <= 0, so the jump always happens
    MT2D_VAR *vars[3] = {named("Dead"), integer(0), integer(1)};
    Cscript_JumpToStateIfRandom(o, vars);
    CHECK_EQ(o->ActualState, 2);
    CHECK_EQ(o->ActualFrame, 0);
}

TEST(jump_to_state_if_random_never_with_negative_chance) {
    Object *o = three_state_object();
    MT2D_VAR *vars[3] = {named("Dead"), integer(-1), integer(10)};
    Cscript_JumpToStateIfRandom(o, vars);
    CHECK_EQ(o->ActualState, 0);
}

// ---- frames --------------------------------------------------------------

static Object *frames_object() {
    static const char *names[] = {"Idle"};
    ObjectState **states = (ObjectState **)malloc(sizeof(ObjectState *));
    states[0] = MT2D_OBJECT_CREATE_STATE((char *)names[0]);
    for (int i = 0; i < 4; i++) {
        MT2D_ObjectState_ADD(states[0], 0, 10 + i, 0);
    }
    return Object_Create(false, false, 1, 1, 0, 0, states, 1);
}

TEST(jump_next_frame_if_var_eq) {
    Object *o = frames_object();
    MT2D_VAR *yes[2] = {integer(2), integer(2)};
    Cscript_JumpNextFrameIfVarEq(o, yes);
    CHECK_EQ(o->ActualFrame, 1);
    CHECK_EQ(o->ActualFrameWait, 10);

    Object *p = frames_object();
    MT2D_VAR *no[2] = {integer(2), integer(3)};
    Cscript_JumpNextFrameIfVarEq(p, no);
    CHECK_EQ(p->ActualFrame, 0);
}

TEST(jump_next_frame_if_var_lower_eq) {
    Object *o = frames_object();
    MT2D_VAR *yes[2] = {integer(1), integer(2)};
    Cscript_JumpNextFrameIfVarLowerEq(o, yes);
    CHECK_EQ(o->ActualFrame, 1);

    Object *p = frames_object();
    MT2D_VAR *no[2] = {integer(3), integer(2)};
    Cscript_JumpNextFrameIfVarLowerEq(p, no);
    CHECK_EQ(p->ActualFrame, 0);
}

TEST(jump_next_frame_if_var_higher_eq) {
    Object *o = frames_object();
    MT2D_VAR *yes[2] = {integer(3), integer(2)};
    Cscript_JumpNextFrameIfVarHigherEq(o, yes);
    CHECK_EQ(o->ActualFrame, 1);

    Object *p = frames_object();
    MT2D_VAR *no[2] = {integer(1), integer(2)};
    Cscript_JumpNextFrameIfVarHigherEq(p, no);
    CHECK_EQ(p->ActualFrame, 0);
}

TEST(jump_frames_if_random) {
    Object *o = frames_object();
    // Vars: [0] frames to jump, [1] chance, [2] max
    MT2D_VAR *vars[3] = {integer(2), integer(0), integer(1)};
    Cscript_JumpFramesIfRandom(o, vars);
    CHECK_EQ(o->ActualFrame, 2);

    MT2D_VAR *back[3] = {integer(-1), integer(0), integer(1)};
    Cscript_JumpFramesIfRandom(o, back);
    CHECK_EQ(o->ActualFrame, 1);

    Object *p = frames_object();
    MT2D_VAR *never[3] = {integer(2), integer(-1), integer(10)};
    Cscript_JumpFramesIfRandom(p, never);
    CHECK_EQ(p->ActualFrame, 0);
}

// ---- while ---------------------------------------------------------------

// Frames: 0 = While, 1 = body, 2 = While_End. The While and While_End calls
// share vars[0] which is how the engine finds the matching end.
struct WhileFixture {
    Object *o;
    MT2D_VAR *vars[4];
};

static WhileFixture make_while(char op, int rightValue) {
    WhileFixture f;
    f.vars[0] = integer(0);                       // start index (filled by Cscript_While)
    f.vars[1] = named("Pos_X");                   // left
    f.vars[2] = MT2D_Object_Create_Var_Char("Op", op);
    f.vars[3] = integer(rightValue);              // right

    static const char *names[] = {"Loop"};
    ObjectState **states = (ObjectState **)malloc(sizeof(ObjectState *));
    states[0] = MT2D_OBJECT_CREATE_STATE((char *)names[0]);
    MT2D_ObjectState_ADD(states[0], 0, 0, Cscript_New3(f.vars, Cscript_While));
    MT2D_ObjectState_ADD(states[0], 0, 0, 0);
    MT2D_ObjectState_ADD(states[0], 0, 0, Cscript_New3(f.vars, Cscript_While_End));
    f.o = Object_Create(false, false, 1, 1, 0, 0, states, 1);
    return f;
}

TEST(while_with_true_condition_enters_the_body) {
    WhileFixture f = make_while('<', 3); // Pos_X (0) < 3
    f.o->ActualFrame = 0;
    Cscript_While(f.o, f.vars);
    CHECK_EQ(f.o->ActualFrame, 0); // stays, the body runs next
}

TEST(while_with_false_condition_skips_to_the_end) {
    WhileFixture f = make_while('>', 3); // Pos_X (0) > 3
    f.o->ActualFrame = 0;
    Cscript_While(f.o, f.vars);
    CHECK_EQ(f.o->ActualFrame, 2); // the While_End frame
}

TEST(while_supported_operators) {
    struct Case { char op; int right; bool enters; };
    Case cases[] = {
        {'>', -1, true}, {'>', 0, false},
        {'<', 1, true},  {'<', 0, false},
        {'=', 0, true},  {'=', 1, false},
        {'!', 1, true},  {'!', 0, false},
    };
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        WhileFixture f = make_while(cases[i].op, cases[i].right);
        f.o->ActualFrame = 0;
        Cscript_While(f.o, f.vars);
        if (cases[i].enters != (f.o->ActualFrame == 0)) {
            printf("    operator %d with right=%d\n", (unsigned char)cases[i].op, cases[i].right);
            CHECK(false);
        }
    }
}

// BUG: the operator is stored in a (signed) char, so 242 (>=) and 243 (<=)
// never match the case labels and fall into the default branch.
TEST_XFAIL(while_greater_equal_and_lower_equal_operators) {
    struct Case { char op; int right; bool enters; };
    Case cases[] = {
        {(char)242, 0, true}, {(char)242, 1, false},   // >=
        {(char)243, 0, true}, {(char)243, -1, false},  // <=
    };
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        WhileFixture f = make_while(cases[i].op, cases[i].right);
        f.o->ActualFrame = 0;
        Cscript_While(f.o, f.vars);
        CHECK_EQ(f.o->ActualFrame == 0, cases[i].enters);
    }
}

TEST(while_unknown_operator_enters_the_body) {
    WhileFixture f = make_while('?', 3);
    f.o->ActualFrame = 0;
    Cscript_While(f.o, f.vars);
    CHECK_EQ(f.o->ActualFrame, 0);
}

TEST(while_end_goes_back_to_the_while) {
    WhileFixture f = make_while('<', 3);
    f.o->ActualFrame = 1; // the While is stored at frame 1 for this call
    Cscript_While(f.o, f.vars);
    CHECK_EQ(*(int *)f.vars[0]->Data, 1);
    f.o->ActualFrame = 2;
    Cscript_While_End(f.o, f.vars);
    // The engine increments the frame after every call, so this lands on the While.
    CHECK_EQ(f.o->ActualFrame, 0);
}

// ---- collisions ----------------------------------------------------------

TEST(goto_state_if_hit_object) {
    static const char *names[] = {"Idle", "Hit"};
    Object *caller = make_object(5, 5, 2, 2, false, 2, names);
    Object *target = make_object(6, 6, 2, 2, true, 1, names);
    MT2D_VAR *vars[1] = {named("Hit")};
    Object_GotoState_IfHitObject(caller, target, vars);
    CHECK_EQ(caller->ActualState, 1);
}

TEST(goto_state_if_hit_object_miss) {
    static const char *names[] = {"Idle", "Hit"};
    Object *caller = make_object(0, 0, 2, 2, false, 2, names);
    Object *target = make_object(20, 20, 2, 2, true, 1, names);
    MT2D_VAR *vars[1] = {named("Hit")};
    Object_GotoState_IfHitObject(caller, target, vars);
    CHECK_EQ(caller->ActualState, 0);
}

TEST(goto_state_if_hit_object_ignores_non_solid_targets) {
    static const char *names[] = {"Idle", "Hit"};
    Object *caller = make_object(5, 5, 2, 2, false, 2, names);
    Object *target = make_object(5, 5, 2, 2, false, 1, names);
    MT2D_VAR *vars[1] = {named("Hit")};
    Object_GotoState_IfHitObject(caller, target, vars);
    CHECK_EQ(caller->ActualState, 0);
}

TEST(goto_state_if_hit_object_touching_edges_counts_as_hit) {
    static const char *names[] = {"Idle", "Hit"};
    Object *caller = make_object(0, 0, 2, 2, false, 2, names);
    Object *target = make_object(2, 2, 2, 2, true, 1, names);
    MT2D_VAR *vars[1] = {named("Hit")};
    Object_GotoState_IfHitObject(caller, target, vars);
    CHECK_EQ(caller->ActualState, 1);
}

TEST(goto_state_if_hit_damages_the_other_object) {
    static const char *names[] = {"Idle", "Hit"};
    ObjectScene *scene = ObjectScene_CreateEmpty();
    Object *caller = make_object(5, 5, 2, 2, false, 2, names);
    Object *victim = make_object(6, 6, 2, 2, true, 1, names);
    MT2D_VAR *health = add_user_var(victim, "Health", 10);
    ObjectScene_Add(scene, caller);
    ObjectScene_Add(scene, victim);

    // [0] state to go, [1] damage, [2] name of the health var in the other object
    MT2D_VAR *vars[3] = {named("Hit"), integer(3), named("Health")};
    Object_GotoState_IfHit(caller, vars);
    CHECK_EQ(*(int *)health->Data, 7);
    CHECK_EQ(caller->ActualState, 1);
}

TEST(goto_state_if_hit_without_collision_does_nothing) {
    static const char *names[] = {"Idle", "Hit"};
    ObjectScene *scene = ObjectScene_CreateEmpty();
    Object *caller = make_object(0, 0, 1, 1, false, 2, names);
    Object *victim = make_object(30, 30, 1, 1, true, 1, names);
    MT2D_VAR *health = add_user_var(victim, "Health", 10);
    ObjectScene_Add(scene, caller);
    ObjectScene_Add(scene, victim);
    MT2D_VAR *vars[3] = {named("Hit"), integer(3), named("Health")};
    Object_GotoState_IfHit(caller, vars);
    CHECK_EQ(*(int *)health->Data, 10);
    CHECK_EQ(caller->ActualState, 0);
}

// ---- object creation -----------------------------------------------------

TEST(create_object_spawns_a_copy_of_the_model_in_the_scene) {
    static const char *names[] = {"Idle"};
    ObjectScene *scene = ObjectScene_CreateEmpty();
    Object *caller = make_object(0, 0, 1, 1, false, 1, names);
    Object *model = make_object(0, 0, 4, 3, true, 1, names);
    add_user_var(model, "Health", 9);
    ObjectScene_Add(scene, caller);

    // [0] pos x, [1] pos y, [2] acel x, [3] acel y
    MT2D_VAR *vars[4] = {integer(10), integer(20), integer(1), integer(-1)};
    Cscript_CreateObject(caller, model, vars);

    CHECK_EQ(scene->Count, 2);
    Object *spawned = scene->ObjectGroup[1];
    CHECK(spawned != model);
    CHECK(spawned->MyScene == scene);
    CHECK(spawned->Solid);
    CHECK(spawned->State == model->State);
    CHECK_EQ(ObjectPosX(spawned), 10);
    CHECK_EQ(ObjectPosY(spawned), 20);
    CHECK_EQ(ObjectAcelX(spawned), 1);
    CHECK_EQ(ObjectAcelY(spawned), -1);
    CHECK_EQ(ObjectSizeX(spawned), 4);
    CHECK_EQ(ObjectSizeY(spawned), 3);
    CHECK_EQ(spawned->User_Vars_Count, Object_First_User_Var_Index + 1);
    CHECK_STR_EQ(spawned->User_Vars[Object_First_User_Var_Index].Name, "Health");
    CHECK_EQ(*(int *)spawned->User_Vars[Object_First_User_Var_Index].Data, 9);
    // the var data is cloned, not shared with the model
    CHECK(spawned->User_Vars[Object_First_User_Var_Index].Data != model->User_Vars[Object_First_User_Var_Index].Data);
}

// ---- audio ---------------------------------------------------------------

TEST(audio_functions_accept_a_sound_name) {
    // Without SDL audio these are no-ops, they must still be callable.
    Object *o = simple_object();
    MT2D_VAR *vars[1] = {named("hit.wav")};
    Cscript_PlaySound(o, vars);
    Cscript_PlayMusic(o, vars);
    CHECK(true);
}

MT2D_TEST_MAIN()

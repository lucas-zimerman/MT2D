// Tests for MT2D/ObjectCore/ObjectLoader.cpp (compiled object files, no text compiler involved)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
#include <MT2D/ObjectCore/ObjectGlobals.h>
#include <MT2D/ObjectCore/CscriptFunctions.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// ---- a tiny writer for the file format ------------------------------------

struct Bin {
    std::vector<unsigned char> v;
    Bin &u8(unsigned x) { v.push_back((unsigned char)x); return *this; }
    Bin &u16(unsigned x) { u8(x & 0xFF); u8((x >> 8) & 0xFF); return *this; }
    Bin &i32(long x) { unsigned long u = (unsigned long)x; for (int i = 0; i < 4; i++) u8((u >> (8 * i)) & 0xFF); return *this; }
    Bin &str(const char *s) { u8((unsigned)strlen(s)); for (; *s; s++) u8(*s); return *this; }
    Bin &magic(unsigned flags = 0) { u8('M'); u8('T'); u8('O'); u8('1'); u8(flags); return *this; }
    Object *load() const { return ObjectCore_LoadFromMemory(v.empty() ? 0 : &v[0], (unsigned long)v.size()); }
};

// One state called "Idle" with one frame: no sprite, wait 5, no function. No vars.
static Bin minimal() {
    Bin b;
    b.magic().u16(0);                  // sprites
    b.u16(1).str("Idle").u16(1);       // 1 state, 1 frame
    b.u16(0xFFFF).u16(5).u16(0xFFFF);  // frame
    b.u8(0).u8(0);                     // vars
    return b;
}

TEST(load_minimal_object) {
    Object *o = minimal().load();
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->States_Count, 1);
    CHECK_STR_EQ(o->State[0]->Name, "Idle");
    CHECK_EQ(o->State[0]->Count, 1);
    CHECK_EQ(o->State[0]->WaitSprites[0], 5);
    CHECK(o->State[0]->Functions[0] == 0);
    CHECK(o->State[0]->Sprites[0] == 0);
    CHECK_EQ(o->ActualFrameWait, 5);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index);
    ObjectCore_Unload(o);
}

TEST(load_sets_builtin_and_new_vars) {
    Bin b;
    b.magic().u16(0).u16(1).str("Idle").u16(1).u16(0xFFFF).u16(1).u16(0xFFFF);
    b.u8(1).u8(3);                       // 1 new var, 3 records
    b.u8(0).i32(7);                      // Pos_X = 7
    b.u8(5).i32(-3);                     // Acel_Y = -3
    b.u8(0xFF).str("hp").i32(30);        // int hp = 30
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(ObjectPosX(o), 7);
    CHECK_EQ(ObjectAcelY(o), -3);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index + 1);
    CHECK_STR_EQ(o->User_Vars[Object_First_User_Var_Index].Name, "hp");
    CHECK_EQ(o->User_Vars[Object_First_User_Var_Index].Data.i, 30);
    // the spare slot Object_Add_Variable expects is there
    MT2D_VAR *more = MT2D_Object_Create_Var_Int("more", 1);
    Object_Add_Variable(o, more);
    free(more); // Object_Add_Variable copies the var
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index + 2);
    ObjectCore_Unload(o);
}

// A frame calling fn with the given args. Used inside a state called "Idle".
static Bin with_function(unsigned fn, unsigned argc, unsigned strBytes) {
    Bin b;
    b.magic().u16(0).u16(1).str("Idle").u16(1);
    b.u16(0xFFFF).u16(1).u16(fn);
    b.u8(argc).u16(strBytes);
    return b;
}
static void vars_hp(Bin &b, int hp) { b.u8(1).u8(1).u8(0xFF).str("hp").i32(hp); }

TEST(loaded_function_with_name_and_number_runs) {
    Bin b = with_function(CSCRIPT_FN_VarAdd, 2, 0);
    b.u8(1).str("hp").u8(0).i32(-5);
    vars_hp(b, 30);
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    Cscript *c = o->State[0]->Functions[0];
    CHECK(c != 0);
    CHECK(c->Packed == 1);
    Cscript_Execute(o, c);
    CHECK_EQ(o->User_Vars[Object_First_User_Var_Index].Data.i, 25);
    Cscript_Execute(o, c); // the function can run again
    CHECK_EQ(o->User_Vars[Object_First_User_Var_Index].Data.i, 20);
    ObjectCore_Unload(o);
}

TEST(loaded_function_without_args_runs) {
    Bin b = with_function(CSCRIPT_FN_RemoveObject, 0, 0);
    b.u8(0).u8(0);
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    CHECK(!o->CanDelete);
    Cscript_Execute(o, o->State[0]->Functions[0]);
    CHECK(o->CanDelete);
    ObjectCore_Unload(o);
}

TEST(loaded_function_text_argument) {
    Bin b = with_function(CSCRIPT_FN_PlaySound, 1, 8);
    b.u8(2).str("hit.wav");
    b.u8(0).u8(0);
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    MT2D_VAR *v = o->State[0]->Functions[0]->TempVars[0];
    CHECK_EQ(v->Type, VAR_STRING);
    CHECK_STR_EQ(MT2D_Object_VAR_GetPath(v), "hit.wav");
    ObjectCore_Unload(o);
}

TEST(truncated_files_are_rejected) {
    Bin b = with_function(CSCRIPT_FN_PlaySound, 1, 8);
    b.u8(2).str("hit.wav");
    vars_hp(b, 1);
    Object *whole = b.load();
    CHECK(whole != 0);
    ObjectCore_Unload(whole);
    for (size_t n = 0; n < b.v.size(); n++) {
        CHECK(ObjectCore_LoadFromMemory(n ? &b.v[0] : (const unsigned char *)"", (unsigned long)n) == 0);
    }
}

TEST(invalid_files_are_rejected) {
    { Bin b = minimal(); b.v[0] = 'X'; CHECK(b.load() == 0); }                       // magic
    { Bin b = with_function(CSCRIPT_FN_COUNT, 0, 0); b.u8(0).u8(0); CHECK(b.load() == 0); }  // unknown function
    { Bin b = with_function(CSCRIPT_FN_VarAdd, 1, 0); b.u8(1).str("hp"); vars_hp(b, 1); CHECK(b.load() == 0); } // wrong argc
    { Bin b = with_function(CSCRIPT_FN_PlaySound, 1, 3); b.u8(2).str("hit.wav"); b.u8(0).u8(0); CHECK(b.load() == 0); } // text bigger than stringBytes
    { Bin b = with_function(CSCRIPT_FN_PlaySound, 1, 9); b.u8(2).str("hit.wav"); b.u8(0).u8(0); CHECK(b.load() == 0); } // stringBytes bigger than the text
    { Bin b = with_function(CSCRIPT_FN_VarInc, 1, 0); b.u8(7).i32(1); b.u8(0).u8(0); CHECK(b.load() == 0); } // unknown arg kind
    { Bin b = with_function(CSCRIPT_FN_VarInc, 1, 0); b.u8(1).str("a_name_that_is_too_long"); b.u8(0).u8(0); CHECK(b.load() == 0); } // name > 18
    { Bin b; b.magic().u16(0).u16(0); b.u8(0).u8(0); CHECK(b.load() == 0); }          // no states
    { Bin b; b.magic().u16(0).u16(1).str("Idle").u16(0); b.u8(0).u8(0); CHECK(b.load() == 0); } // state without frames
    { Bin b; b.magic().u16(0).u16(1).str("Idle").u16(1).u16(3).u16(1).u16(0xFFFF); b.u8(0).u8(0); CHECK(b.load() == 0); } // sprite index out of range
    { Bin b = minimal(); b.v[b.v.size() - 2] = 1; CHECK(b.load() == 0); }            // says 1 new var but has no records
    CHECK(ObjectCore_LoadFromMemory(0, 0) == 0);
}

static void write_sprite(const char *name) {
    FILE *f = fopen(name, "wb");
    CHECK(f != 0);
    if (f) { fputs("AB\r\nCD\r\n", f); fclose(f); }
}

TEST(sprites_are_loaded_once_and_shared) {
    write_sprite("shared.txt");
    Bin b;
    b.magic().u16(1).u8(0).str("shared.txt");
    b.u16(1).str("Idle").u16(2);
    b.u16(0).u16(1).u16(0xFFFF);
    b.u16(0).u16(1).u16(0xFFFF);
    b.u8(0).u8(0);
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    CHECK(o->State[0]->Sprites[0] != 0);
    CHECK(o->State[0]->Sprites[0] == o->State[0]->Sprites[1]);
    CHECK_EQ(o->State[0]->Sprites[0]->refCount, 2);
    ObjectCore_Unload(o);
}

TEST(missing_sprite_file_fails) {
    Bin b;
    b.magic().u16(1).u8(0).str("nope_not_here.txt");
    b.u16(1).str("Idle").u16(1).u16(0).u16(1).u16(0xFFFF);
    b.u8(0).u8(0);
    CHECK(b.load() == 0);
}

TEST(load_from_file) {
    Bin b = minimal();
    FILE *f = fopen("minimal.mto", "wb");
    CHECK(f != 0);
    if (!f) return;
    fwrite(&b.v[0], 1, b.v.size(), f);
    fclose(f);
    Object *o = ObjectCore_Load("minimal.mto");
    CHECK(o != 0);
    ObjectCore_Unload(o);
    CHECK(ObjectCore_Load("does_not_exist.mto") == 0);
    CHECK(ObjectCore_Load(0) == 0);
}

TEST(states_can_be_shared_by_new_objects) {
    Object *model = minimal().load();
    CHECK(model != 0);
    if (!model) return;
    Object *other = Object_Create(false, false, 1, 1, 5, 6, model->State, model->States_Count);
    CHECK(other->State == model->State);
    Object_Delete(other, false);
    ObjectCore_Unload(model);
}

// ---- flags, sprite kinds, clone ----------------------------------------------

TEST(flags_set_solid_and_render_only) {
    { Bin b = minimal(); b.v[4] = 1; Object *o = b.load(); CHECK(o != 0); if (o) { CHECK(o->Solid); CHECK(!o->RenderOnly); ObjectCore_Unload(o); } }
    { Bin b = minimal(); b.v[4] = 2; Object *o = b.load(); CHECK(o != 0); if (o) { CHECK(!o->Solid); CHECK(o->RenderOnly); ObjectCore_Unload(o); } }
    { Bin b = minimal(); Object *o = b.load(); CHECK(o != 0); if (o) { CHECK(!o->Solid); CHECK(!o->RenderOnly); ObjectCore_Unload(o); } }
}

TEST(image_sprites_use_the_scale) {
    Bin b;
    b.magic().u16(2);
    b.u8(1).u16(4).u16(5).str("a.png");       // image file
    b.u8(2).u16(8).u16(9).str("b.png");       // image from the container
    b.u16(1).str("Idle").u16(2);
    b.u16(0).u16(1).u16(0xFFFF);
    b.u16(1).u16(1).u16(0xFFFF);
    b.u8(0).u8(0);
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Sprites[0]->ScaleX, 4);
    CHECK_EQ(o->State[0]->Sprites[0]->ScaleY, 5);
    CHECK_EQ(o->State[0]->Sprites[1]->ScaleX, 8);
    CHECK_EQ(o->State[0]->Sprites[1]->ScaleY, 9);
    ObjectCore_Unload(o);
    { Bin bad; bad.magic().u16(1).u8(3).u16(1).u16(1).str("a.png"); bad.u16(1).str("Idle").u16(1).u16(0).u16(1).u16(0xFFFF); bad.u8(0).u8(0); CHECK(bad.load() == 0); } // unknown kind
}

TEST(clone_shares_states_and_copies_vars) {
    Bin b = with_function(CSCRIPT_FN_RemoveObject, 0, 0);
    b.u8(1).u8(3);
    b.u8(2).i32(30);                  // Size_X
    b.u8(4).i32(-2);                  // Acel_X
    b.u8(0xFF).str("hp").i32(7);
    b.v[4] = 1;                       // solid
    Object *model = b.load();
    CHECK(model != 0);
    if (!model) return;
    ObjectPosX(model) = 99;           // the position is not copied
    model->User_Vars[Object_First_User_Var_Index].Data.i = 8;
    Object *c = ObjectCore_Clone(model, 5, 6);
    CHECK(c != 0);
    if (!c) return;
    CHECK(c->State == model->State);
    CHECK_EQ(c->States_Count, model->States_Count);
    CHECK(c->Solid);
    CHECK_EQ(ObjectPosX(c), 5);
    CHECK_EQ(ObjectPosY(c), 6);
    CHECK_EQ(ObjectSizeX(c), 30);
    CHECK_EQ(ObjectAcelX(c), -2);
    CHECK_EQ(c->User_Vars_Count, Object_First_User_Var_Index + 1);
    CHECK_STR_EQ(c->User_Vars[Object_First_User_Var_Index].Name, "hp");
    CHECK_EQ(c->User_Vars[Object_First_User_Var_Index].Data.i, 8);
    c->User_Vars[Object_First_User_Var_Index].Data.i = 1;   // own copy
    CHECK_EQ(model->User_Vars[Object_First_User_Var_Index].Data.i, 8);
    Object_Delete(c, false);
    CHECK(ObjectCore_Clone(0, 0, 0) == 0);
    ObjectCore_Unload(model);
}

// ---- global vars, objects and functions -------------------------------------

static int global_calls = 0;
static void count_call(Object *) { global_calls++; }
static void add_to_first(Object *, MT2D_VAR **v) { global_calls += v[0]->Data.i; }
static Object *seen_target = 0;
static void remember_target(Object *, Object *target, MT2D_VAR **) { seen_target = target; }

TEST(global_var_is_the_registered_var) {
    MT2D_VAR *speed = MT2D_Object_Create_Var_Int("GameSpeed", 3);
    MT2D_VAR *health = MT2D_Object_Create_Var_Int("Health", 6);
    CHECK(ObjectCore_AddGlobalVar(speed));
    CHECK(ObjectCore_AddGlobalVar(health));
    // hp += global GameSpeed ; global Health--
    Bin b;
    b.magic().u16(0).u16(1).str("Idle").u16(2);
    b.u16(0xFFFF).u16(1).u16(CSCRIPT_FN_VarAdd).u8(2).u16(0).u8(1).str("hp").u8(3).str("GameSpeed");
    b.u16(0xFFFF).u16(1).u16(CSCRIPT_FN_VarDec).u8(1).u16(0).u8(3).str("Health");
    vars_hp(b, 10);
    Object *o = b.load();
    CHECK(o != 0);
    if (o) {
        CHECK(o->State[0]->Functions[0]->TempVars[1] == speed);   // not a copy
        Cscript_Execute(o, o->State[0]->Functions[0]);
        CHECK_EQ(o->User_Vars[Object_First_User_Var_Index].Data.i, 13);
        Cscript_Execute(o, o->State[0]->Functions[1]);
        CHECK_EQ(health->Data.i, 5);                                // written through
        speed->Data.i = 10;                                         // read live
        Cscript_Execute(o, o->State[0]->Functions[0]);
        CHECK_EQ(o->User_Vars[Object_First_User_Var_Index].Data.i, 23);
        ObjectCore_Unload(o);
    }
    // once removed, a new load fails and says why
    CHECK(ObjectCore_RemoveGlobalVar("GameSpeed"));
    MT2D_VAR_Free(speed, 1);   // the registry does not own the vars
    Bin again;
    again.magic().u16(0).u16(1).str("Idle").u16(1);
    again.u16(0xFFFF).u16(1).u16(CSCRIPT_FN_VarInc).u8(1).u16(0).u8(3).str("GameSpeed");
    again.u8(0).u8(0);
    CHECK(again.load() == 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "global var is not registered: 'GameSpeed'");
    CHECK(ObjectCore_RemoveGlobalVar("Health"));
    MT2D_VAR_Free(health, 1);
}

TEST(global_function_runs_with_its_arguments) {
    CHECK(ObjectCore_AddGlobalCScript("Counter", count_call));
    CHECK(ObjectCore_AddGlobalCScript("AddToCalls", add_to_first, 1));
    Bin b;
    b.magic().u16(0).u16(1).str("Idle").u16(2);
    b.u16(0xFFFF).u16(1).u16(0xFFFE).str("Counter").u8(0).u16(0);
    b.u16(0xFFFF).u16(1).u16(0xFFFE).str("AddToCalls").u8(1).u16(0).u8(0).i32(40);
    b.u8(0).u8(0);
    Object *o = b.load();
    CHECK(o != 0);
    if (!o) return;
    global_calls = 0;
    Cscript_Execute(o, o->State[0]->Functions[0]);
    CHECK_EQ(global_calls, 1);
    Cscript_Execute(o, o->State[0]->Functions[1]);
    CHECK_EQ(global_calls, 41);
    ObjectCore_Unload(o);

    // wrong number of arguments for AddToCalls (registered with 1)
    Bin wrong;
    wrong.magic().u16(0).u16(1).str("Idle").u16(1);
    wrong.u16(0xFFFF).u16(1).u16(0xFFFE).str("AddToCalls").u8(2).u16(0).u8(0).i32(1).u8(0).i32(2);
    wrong.u8(0).u8(0);
    CHECK(wrong.load() == 0);
    CHECK(strstr(ObjectCore_GetLastError(), "wrong argument count") != 0);

    // not registered
    Bin missing;
    missing.magic().u16(0).u16(1).str("Idle").u16(1);
    missing.u16(0xFFFF).u16(1).u16(0xFFFE).str("Nope").u8(0).u16(0);
    missing.u8(0).u8(0);
    CHECK(missing.load() == 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "global function is not registered: 'Nope'");

    CHECK(ObjectCore_RemoveGlobalCScript("Counter"));
    CHECK(ObjectCore_RemoveGlobalCScript("AddToCalls"));
}

TEST(global_object_is_the_first_argument_of_object_functions) {
    Object *target = minimal().load();
    CHECK(target != 0);
    if (!target) return;
    CHECK(ObjectCore_AddGlobalObject("Doggo", target));
    CHECK(ObjectCore_AddGlobalCScript("Remember", remember_target, 2));
    Bin b;
    b.magic().u16(0).u16(1).str("Idle").u16(2);
    b.u16(0xFFFF).u16(1).u16(0xFFFE).str("Remember").u8(2).u16(0).u8(3).str("Doggo").u8(1).str("hit");
    b.u16(0xFFFF).u16(1).u16(CSCRIPT_FN_GotoStateIfHitObject).u8(2).u16(0).u8(3).str("Doggo").u8(1).str("hit");
    b.u8(0).u8(0);
    Object *o = b.load();
    CHECK(o != 0);
    if (o) {
        seen_target = 0;
        Cscript_Execute(o, o->State[0]->Functions[0]);
        CHECK(seen_target == target);
        CHECK(o->State[0]->Functions[1]->TempObj == target);
        CHECK_EQ(o->State[0]->Functions[1]->Packed, 1);
        ObjectCore_Unload(o);   // must not free the global object
    }
    // the object argument has to be a global
    Bin notGlobal;
    notGlobal.magic().u16(0).u16(1).str("Idle").u16(1);
    notGlobal.u16(0xFFFF).u16(1).u16(CSCRIPT_FN_GotoStateIfHitObject).u8(2).u16(0).u8(1).str("Doggo").u8(1).str("hit");
    notGlobal.u8(0).u8(0);
    CHECK(notGlobal.load() == 0);
    CHECK(ObjectCore_RemoveGlobalObject("Doggo"));
    CHECK(b.load() == 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "global object is not registered: 'Doggo'");
    CHECK(ObjectCore_RemoveGlobalCScript("Remember"));
    ObjectCore_Unload(target);
}

MT2D_TEST_MAIN()

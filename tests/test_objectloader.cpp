// Tests for MT2D/ObjectCore/ObjectLoader.cpp (compiled object files, no text compiler involved)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
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
    Bin &magic() { u8('M'); u8('T'); u8('O'); u8('1'); return *this; }
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
    b.magic().u16(1).str("shared.txt");
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
    b.magic().u16(1).str("nope_not_here.txt");
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

MT2D_TEST_MAIN()

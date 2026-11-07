// Tests for MT2D/ObjectCore/Parser/CscriptCompiler.cpp (text -> compiled object -> running object)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <string>
#include <vector>
#include <stdio.h>

static Object *compile_and_load(const char *text, std::string *errOut = 0) {
    std::vector<unsigned char> bin;
    std::string err;
    bool ok = CscriptCompiler_Compile(text, bin, err);
    if (errOut) *errOut = err;
    if (!ok) return 0;
    return ObjectCore_LoadFromMemory(&bin[0], (unsigned long)bin.size());
}

static std::string error_of(const char *text) {
    std::vector<unsigned char> bin;
    std::string err;
    CHECK(!CscriptCompiler_Compile(text, bin, err));
    CHECK(bin.empty());
    return err;
}

static bool has(const std::string &s, const char *part) { return s.find(part) != std::string::npos; }

static MT2D_VAR *find_var(Object *o, const char *name) {
    for (int i = 0; i < o->User_Vars_Count; i++) if (strcmp(o->User_Vars[i].Name, name) == 0) return &o->User_Vars[i];
    return 0;
}

TEST(moving_dot_runs) {
    Object *o = compile_and_load(
        "object movingdot\n"
        "{\n"
        "    int x = 10;\n"
        "    int y = 20;   // comment\n"
        "    /* another\n comment */\n"
        "    states\n"
        "    {\n"
        "    Spawn:\n"
        "        none 2 VarInc(y)\n"
        "        none 0 JumpToStateIfVarLower(Spawn, y, 24)\n"
        "        Goto Done\n"
        "    Done:\n"
        "        none 1\n"
        "    }\n"
        "}\n");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->States_Count, 2);
    CHECK_STR_EQ(o->State[0]->Name, "Spawn");
    CHECK_STR_EQ(o->State[1]->Name, "Done");
    CHECK_EQ(find_var(o, "x")->Data.i, 10);
    for (int tick = 0; tick < 200 && o->ActualState == 0; tick++) Object_Goto_NextStep(o);
    CHECK_EQ(o->ActualState, 1);
    CHECK_EQ(find_var(o, "y")->Data.i, 24);
    ObjectCore_Unload(o);
}

TEST(builtin_vars_and_function_forms) {
    Object *o = compile_and_load(
        "object a { int Pos_X = 7; int Acel_X = -2; int n = 1;\n"
        " states { Idle:\n"
        "   none 1 move\n"          // no parentheses, not case sensitive
        "   none 1 VarAdd(n, 41);\n"
        "   \"\" 1 PLAYSOUND(\"hit.wav\")\n"
        "   none 0 Stop }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(ObjectPosX(o), 7);
    CHECK_EQ(ObjectAcelX(o), -2);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index + 1); // only n is new
    CHECK_EQ(o->State[0]->Count, 5);
    Cscript_Execute(o, o->State[0]->Functions[0]);
    CHECK_EQ(ObjectPosX(o), 5);
    Cscript_Execute(o, o->State[0]->Functions[1]);
    CHECK_EQ(find_var(o, "n")->Data.i, 42);
    CHECK_STR_EQ(MT2D_Object_VAR_GetPath(o->State[0]->Functions[2]->TempVars[0]), "hit.wav");
    ObjectCore_Unload(o);
}

TEST(operator_arguments) {
    Object *o = compile_and_load(
        "object a { states { S:\n"
        " none 1 JumpFramesIfVar(1, Pos_X, <, 5)\n"
        " none 1 JumpFramesIfVar(1, Pos_X, >=, 5)\n"
        " none 1 JumpFramesIfVar(1, Pos_X, !=, 5)\n"
        " none 1 JumpFramesIfVar(1, Pos_X, ==, 5)\n"
        "}}");
    CHECK(o != 0);
    if (!o) return;
    MT2D_VAR **v = o->State[0]->Functions[1]->TempVars;
    CHECK_EQ(v[2]->Data.i, 242);
    CHECK_EQ(o->State[0]->Functions[0]->TempVars[2]->Data.i, '<');
    CHECK_EQ(o->State[0]->Functions[2]->TempVars[2]->Data.i, '!');
    CHECK_EQ(o->State[0]->Functions[3]->TempVars[2]->Data.i, '=');
    ObjectCore_Unload(o);
}

TEST(sprites_are_shared_between_frames) {
    FILE *f = fopen("dot.txt", "wb");
    CHECK(f != 0);
    if (f) { fputs("AB\r\nCD\r\n", f); fclose(f); }
    Object *o = compile_and_load("object a { states { S: \"dot.txt\" 1  \"dot.txt\" 2 none 3 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(o->State[0]->Sprites[0] != 0);
    CHECK(o->State[0]->Sprites[0] == o->State[0]->Sprites[1]);
    CHECK(o->State[0]->Sprites[2] == 0);
    ObjectCore_Unload(o);
}

TEST(loop_state_goto_and_stop) {
    Object *o = compile_and_load("object a { states { A: none 1 Goto B  B: Stop }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Count, 2);
    Cscript_Execute(o, o->State[0]->Functions[1]);
    CHECK_EQ(o->ActualState, 1);
    Cscript_Execute(o, o->State[1]->Functions[0]);
    CHECK(o->CanDelete);
    ObjectCore_Unload(o);
}

TEST(errors_have_line_numbers_and_reasons) {
    CHECK(has(error_of("object a {\n states {\n S:\n none 1 Nope(1)\n }}"), "line 4: unknown function 'Nope'"));
    CHECK(has(error_of("object a { states { S: none 1 VarInc }}"), "VarInc needs 1 argument(s), got 0"));
    CHECK(has(error_of("object a { states { S: none 1 VarInc(a, b) }}"), "needs 1"));
    CHECK(has(error_of("object a { states { S: none 1 VarInc(ghost) }}"), "'ghost' is not a var or a state"));
    CHECK(has(error_of("object a { states { S: none 1 Goto Nowhere }}"), "'Nowhere' is not a var or a state"));
    CHECK(has(error_of("object a { states { S: none }}"), "expected the wait"));
    CHECK(has(error_of("object a { states { S: none 70000 }}"), "expected the wait"));
    CHECK(has(error_of("object a { states { S: none 1  S: none 1 }}"), "declared twice"));
    CHECK(has(error_of("object a { int x; int x; states { S: none 1 }}"), "declared twice"));
    CHECK(has(error_of("object a { states { S: }}"), "has no frames"));
    CHECK(has(error_of("object a { }"), "no states"));
    CHECK(has(error_of("object a { int a_very_long_variable_name = 1; states { S: none 1 }}"), "longer than 18"));
    CHECK(has(error_of("object a { states { S: \"never closed 1 }}"), "never closed"));
    CHECK(has(error_of("object a { states { S: none 1 } } extra"), "unexpected text"));
    CHECK(has(error_of("object a { states { S: none 1 $ }}"), "unexpected character"));
    CHECK(has(error_of("object a { int x = 99999999999; states { S: none 1 }}"), "too big"));
    CHECK(has(error_of("nothing"), "expected 'object'"));
    CHECK(has(error_of(""), "expected 'object'"));
}

TEST(missing_sprite_file_is_a_load_error) {
    std::string err;
    Object *o = compile_and_load("object a { states { S: \"no_such_sprite.txt\" 1 }}", &err);
    CHECK(o == 0);
    CHECK(err.empty()); // it compiles, the file is only needed when the object is loaded
}

TEST(load_script_from_text_file) {
    FILE *f = fopen("script.txt", "wb");
    CHECK(f != 0);
    if (!f) return;
    fputs("object a { int hp = 3; states { S: none 1 } }", f);
    fclose(f);
    std::string err;
    Object *o = ObjectCore_LoadScript("script.txt", &err);
    CHECK(o != 0);
    if (o) CHECK_EQ(find_var(o, "hp")->Data.i, 3);
    ObjectCore_Unload(o);
    CHECK(ObjectCore_LoadScript("missing.txt", &err) == 0);
    CHECK(has(err, "cannot open"));
}

MT2D_TEST_MAIN()

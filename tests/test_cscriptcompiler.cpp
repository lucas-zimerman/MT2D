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

// ---- while / if / else ------------------------------------------------------

// Runs the object until it reaches the state called "End" (at most 500 ticks). The scripts
// below always finish with `Goto End`, `End: none 1`.
static bool run_to_end(Object *o) {
    for (int tick = 0; tick < 500; tick++) {
        if (o->ActualState == o->States_Count - 1) return true;
        Object_Goto_NextStep(o);
    }
    return false;
}

static int var_of(Object *o, const char *name) { return find_var(o, name)->Data.i; }

TEST(while_loop) {
    Object *o = compile_and_load(
        "object a { int y = 0; int steps = 0; states {\n"
        "Main:\n"
        "  while (y < 5) {\n"
        "    none 2 VarInc(y)\n"
        "    none 1 VarInc(steps)\n"
        "  }\n"
        "  Goto End\n"
        "End: none 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(run_to_end(o));
    CHECK_EQ(var_of(o, "y"), 5);
    CHECK_EQ(var_of(o, "steps"), 5);
    ObjectCore_Unload(o);
}

TEST(while_loop_that_never_runs) {
    Object *o = compile_and_load(
        "object a { int y = 9; states {\n"
        "Main:\n"
        "  while (y < 5) { none 1 VarInc(y) }\n"
        "  Goto End\n"
        "End: none 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(run_to_end(o));
    CHECK_EQ(var_of(o, "y"), 9);
    ObjectCore_Unload(o);
}

TEST(if_else_picks_one_branch) {
    const char *tmpl =
        "object a { int x = %d; int r = 0; states {\n"
        "Main:\n"
        "  if (x < 5) { none 0 VarAdd(r, 1) } else { none 0 VarAdd(r, 10) }\n"
        "  Goto End\n"
        "End: none 1 }}";
    int xs[] = { 4, 5, 6 };
    int expect[] = { 1, 10, 10 };
    for (int i = 0; i < 3; i++) {
        char text[512];
        snprintf(text, sizeof(text), tmpl, xs[i]);
        Object *o = compile_and_load(text);
        CHECK(o != 0);
        if (!o) return;
        CHECK(run_to_end(o));
        CHECK_EQ(var_of(o, "r"), expect[i]);
        ObjectCore_Unload(o);
    }
}

TEST(if_without_else_and_else_if_chain) {
    const char *tmpl =
        "object a { int x = %d; int r = 0; states {\n"
        "Main:\n"
        "  if (x == 1) { none 0 VarAdd(r, 1) }\n"
        "  else if (x == 2) { none 0 VarAdd(r, 2) }\n"
        "  else if (x == 3) { none 0 VarAdd(r, 3) }\n"
        "  else { none 0 VarAdd(r, 100) }\n"
        "  if (x > 2) { none 0 VarAdd(r, 1000) }\n"
        "  Goto End\n"
        "End: none 1 }}";
    int xs[] = { 1, 2, 3, 4 };
    int expect[] = { 1, 2, 1003, 1100 };
    for (int i = 0; i < 4; i++) {
        char text[700];
        snprintf(text, sizeof(text), tmpl, xs[i]);
        Object *o = compile_and_load(text);
        CHECK(o != 0);
        if (!o) return;
        CHECK(run_to_end(o));
        CHECK_EQ(var_of(o, "r"), expect[i]);
        ObjectCore_Unload(o);
    }
}

TEST(nested_while_and_if) {
    // counts the even numbers from 0 to 9
    Object *o = compile_and_load(
        "object a { int i = 0; int even = 0; int odd = 0; states {\n"
        "Main:\n"
        "  while (i < 10) {\n"
        "    if (i == 0 || i == 2 || i == 4 || i == 6 || i == 8) {\n"
        "      none 1 VarInc(even)\n"
        "    } else {\n"
        "      none 1 VarInc(odd)\n"
        "    }\n"
        "    none 0 VarInc(i)\n"
        "  }\n"
        "  Goto End\n"
        "End: none 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(run_to_end(o));
    CHECK_EQ(var_of(o, "i"), 10);
    CHECK_EQ(var_of(o, "even"), 5);
    CHECK_EQ(var_of(o, "odd"), 5);
    ObjectCore_Unload(o);
}

// The condition is compiled to chains of single comparisons. Compare the result with the
// one C++ gives for every combination of a, b and c.
static int cond_result(const char *cond, int a, int b, int c) {
    char text[700];
    snprintf(text, sizeof(text),
        "object t { int a = %d; int b = %d; int c = %d; int r = 0; states {\n"
        "Main:\n"
        "  if (%s) { none 0 VarAdd(r, 1) }\n"
        "  Goto End\n"
        "End: none 1 }}", a, b, c, cond);
    Object *o = compile_and_load(text);
    CHECK(o != 0);
    if (!o) return -1;
    CHECK(run_to_end(o));
    int r = var_of(o, "r");
    ObjectCore_Unload(o);
    return r;
}

TEST(logic_operators_match_c) {
    struct Case { const char *text; bool (*eval)(bool, bool, bool); };
    // each case is written twice: the script text and the same expression in C++
    static const Case cases[] = {
        { "a && b",                 [](bool a, bool b, bool)   { return a && b; } },
        { "a || b",                 [](bool a, bool b, bool)   { return a || b; } },
        { "!a",                     [](bool a, bool, bool)     { return !a; } },
        { "a && b && c",            [](bool a, bool b, bool c) { return a && b && c; } },
        { "a || b || c",            [](bool a, bool b, bool c) { return a || b || c; } },
        { "a || b && c",            [](bool a, bool b, bool c) { return a || (b && c); } },
        { "a && b || c",            [](bool a, bool b, bool c) { return (a && b) || c; } },
        { "!(a && b) || c",         [](bool a, bool b, bool c) { return !(a && b) || c; } },
        { "!a && !b",               [](bool a, bool b, bool)   { return !a && !b; } },
        { "!(a || b)",              [](bool a, bool b, bool)   { return !(a || b); } },
        { "(a || b) && (b || c)",   [](bool a, bool b, bool c) { return (a || b) && (b || c); } },
        { "!(!a || !b)",            [](bool a, bool b, bool)   { return a && b; } },
        { "a && (b || c) && !c",    [](bool a, bool b, bool c) { return a && (b || c) && !c; } },
        { "!(a && (b || !c))",      [](bool a, bool b, bool c) { return !(a && (b || !c)); } },
    };
    for (size_t k = 0; k < sizeof(cases) / sizeof(cases[0]); k++) {
        for (int bits = 0; bits < 8; bits++) {
            int a = bits & 1, b = (bits >> 1) & 1, c = (bits >> 2) & 1;
            int got = cond_result(cases[k].text, a, b, c);
            int want = cases[k].eval(a != 0, b != 0, c != 0) ? 1 : 0;
            if (got != want) printf("    (%s) with a=%d b=%d c=%d gave %d, expected %d\n", cases[k].text, a, b, c, got, want);
            CHECK_EQ(got, want);
        }
    }
}

TEST(comparison_operators_in_conditions) {
    struct Case { const char *cond; int expect; };
    // x is 5
    static const Case cases[] = {
        { "x < 6", 1 },  { "x < 5", 0 },  { "x > 4", 1 },  { "x > 5", 0 },
        { "x == 5", 1 }, { "x = 5", 1 },  { "x != 5", 0 }, { "x != 4", 1 },
        { "x >= 5", 1 }, { "x >= 6", 0 }, { "x <= 5", 1 }, { "x <= 4", 0 },
        { "x", 1 },      { "!x", 0 },     { "0", 0 },      { "7", 1 },
        { "x < y", 1 },  { "y < x", 0 },  { "-3 < x", 1 }, { "(x) && (y)", 1 },
    };
    for (size_t k = 0; k < sizeof(cases) / sizeof(cases[0]); k++) {
        char text[500];
        snprintf(text, sizeof(text),
            "object t { int x = 5; int y = 9; int r = 0; states { Main:\n"
            "  if (%s) { none 0 VarAdd(r, 1) }\n"
            "  Goto End\n"
            "End: none 1 }}", cases[k].cond);
        Object *o = compile_and_load(text);
        CHECK(o != 0);
        if (!o) return;
        CHECK(run_to_end(o));
        if (var_of(o, "r") != cases[k].expect) printf("    (%s) gave %d\n", cases[k].cond, var_of(o, "r"));
        CHECK_EQ(var_of(o, "r"), cases[k].expect);
        ObjectCore_Unload(o);
    }
}

TEST(while_with_logic_operators) {
    Object *o = compile_and_load(
        "object a { int i = 0; int j = 10; int n = 0; states {\n"
        "Main:\n"
        "  while (i < 20 && j > 4) {\n"
        "    none 1 VarInc(i)\n"
        "    none 0 VarDec(j)\n"
        "    none 0 VarInc(n)\n"
        "  }\n"
        "  Goto End\n"
        "End: none 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(run_to_end(o));
    CHECK_EQ(var_of(o, "n"), 6); // j goes 10 -> 4
    CHECK_EQ(var_of(o, "i"), 6);
    ObjectCore_Unload(o);
}

TEST(control_flow_at_the_end_of_a_state_wraps_to_the_first_frame) {
    // the last frame of the state is the jump back of the while, a loop ends past the last frame
    Object *o = compile_and_load(
        "object a { int n = 0; states {\n"
        "A:\n"
        "  none 1\n"
        "  if (n < 3) { none 1 VarInc(n) }\n"
        "}}");
    CHECK(o != 0);
    if (!o) return;
    for (int tick = 0; tick < 40; tick++) Object_Goto_NextStep(o);
    CHECK_EQ(var_of(o, "n"), 3);
    ObjectCore_Unload(o);
}

TEST(control_flow_errors) {
    CHECK(has(error_of("object a { states { S:\n none 1 else { } }}"), "line 2: 'else' without 'if'"));
    CHECK(has(error_of("object a { int x; states { S: while x < 1 { none 1 } }}"), "expected '('"));
    CHECK(has(error_of("object a { int x; states { S: while (x < 1 { none 1 } }}"), "expected ')'"));
    CHECK(has(error_of("object a { int x; states { S: while (x < 1) none 1 }}"), "expected '{'"));
    CHECK(has(error_of("object a { int x; states { S: if (x < 1) { none 1 "), "expected '}'"));
    CHECK(has(error_of("object a { int x; states { S: none 1 }"), "expected 'int', 'states' or '}'"));
    CHECK(has(error_of("object a { int x; states { S: if (x <) { none 1 } }}"), "expected a var or a number"));
    CHECK(has(error_of("object a { int x; states { S: if () { none 1 } }}"), "expected a var or a number"));
    CHECK(has(error_of("object a { states { S: if (ghost < 1) { none 1 } }}"), "'ghost' is not a var or a state"));
    CHECK(has(error_of("object a { int x; states { S: if (x & 1) { none 1 } }}"), "unexpected character '&'"));
    CHECK(has(error_of("object a { int x; states { S: if (x < 1) { T: none 1 } }}"), "label cannot be inside a block"));
    CHECK(has(error_of("object a { int x; states { S: none 1 if (x < 1) { none 1 } else }}"), "expected '{'"));
}

MT2D_TEST_MAIN()

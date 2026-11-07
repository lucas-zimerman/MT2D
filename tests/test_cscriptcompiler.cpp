// Tests for MT2D/ObjectCore/Parser/CscriptCompiler.cpp (text -> compiled object -> running object)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

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
static int var_of(Object *o, const char *name) { return find_var(o, name)->Data.i; }

// Runs the object until it reaches the last state (at most 500 ticks). The control flow
// scripts below finish with `Goto End`, `End: Wait 1`.
static bool run_to_end(Object *o) {
    for (int tick = 0; tick < 500; tick++) {
        if (o->ActualState == o->States_Count - 1) return true;
        Object_Goto_NextStep(o);
    }
    return false;
}

// ---- basics ----------------------------------------------------------------

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
        "        Wait 2\n"
        "        y++\n"
        "        JumpToStateIfVarLower(Spawn, y, 24)\n"
        "        Goto Done\n"
        "    Done:\n"
        "        Wait 1\n"
        "    }\n"
        "}\n");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->States_Count, 2);
    CHECK_STR_EQ(o->State[0]->Name, "Spawn");
    CHECK_STR_EQ(o->State[1]->Name, "Done");
    CHECK_EQ(o->State[0]->Count, 3); // y++ shares the frame of Wait 2
    CHECK_EQ(var_of(o, "x"), 10);
    for (int tick = 0; tick < 200 && o->ActualState == 0; tick++) Object_Goto_NextStep(o);
    CHECK_EQ(o->ActualState, 1);
    CHECK_EQ(var_of(o, "y"), 24);
    ObjectCore_Unload(o);
}

TEST(statement_forms) {
    Object *o = compile_and_load(
        "object t { int a = 1; int b = 10; int c = 0; states { S:\n"
        "  a++\n"
        "  b--;\n"
        "  a += 5\n"
        "  b -= 2\n"
        "  c = 3\n"
        "  c = a\n"
        "  Goto E\n"
        "E: Wait 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Count, 7);
    for (int i = 0; i < 5; i++) Cscript_Execute(o, o->State[0]->Functions[i]);
    CHECK_EQ(var_of(o, "a"), 7);
    CHECK_EQ(var_of(o, "b"), 7);
    CHECK_EQ(var_of(o, "c"), 3);
    Cscript_Execute(o, o->State[0]->Functions[5]);
    CHECK_EQ(var_of(o, "c"), 7); // c = a
    ObjectCore_Unload(o);
}

TEST(builtin_vars_and_function_forms) {
    Object *o = compile_and_load(
        "object a { int Pos_X = 7; int Acel_X = -2; int n = 1;\n"
        " states { Idle:\n"
        "   Wait 1\n"
        "   move\n"                      // no parentheses, not case sensitive
        "   n += 41;\n"
        "   Show \"\" Wait 1\n"          // an empty sprite is no sprite
        "   PLAYSOUND(\"hit.wav\")\n"
        "   Stop }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(ObjectPosX(o), 7);
    CHECK_EQ(ObjectAcelX(o), -2);
    CHECK_EQ(o->User_Vars_Count, Object_First_User_Var_Index + 1); // only n is new
    CHECK_EQ(o->State[0]->Count, 4);
    Cscript_Execute(o, o->State[0]->Functions[0]);
    CHECK_EQ(ObjectPosX(o), 5);
    Cscript_Execute(o, o->State[0]->Functions[1]);
    CHECK_EQ(var_of(o, "n"), 42);
    CHECK_STR_EQ(MT2D_Object_VAR_GetPath(o->State[0]->Functions[2]->TempVars[0]), "hit.wav");
    ObjectCore_Unload(o);
}

TEST(comparison_operators_compile_to_the_opposite_jump) {
    // the jump frame leaves the block when the condition is false: it uses the inverse operator
    Object *o = compile_and_load(
        "object a { states { S:\n"
        " if (Pos_X <  5) { Wait 1 }\n"
        " if (Pos_X >= 5) { Wait 1 }\n"
        " if (Pos_X != 5) { Wait 1 }\n"
        " if (Pos_X == 5) { Wait 1 }\n"
        "}}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Count, 8);
    CHECK_EQ(o->State[0]->Functions[0]->TempVars[2]->Data.i, 242);  // < becomes >=
    CHECK_EQ(o->State[0]->Functions[2]->TempVars[2]->Data.i, '<');  // >= becomes <
    CHECK_EQ(o->State[0]->Functions[4]->TempVars[2]->Data.i, '=');  // != becomes ==
    CHECK_EQ(o->State[0]->Functions[6]->TempVars[2]->Data.i, '!');  // == becomes !=
    ObjectCore_Unload(o);
}

TEST(frame_counting_functions_cannot_be_called_by_hand) {
    CHECK(has(error_of("object a { states { S: JumpFrames(1) }}"), "'JumpFrames' counts frames, use if / while instead"));
    CHECK(has(error_of("object a { states { S: jumpframesifvar(1, Pos_X, <, 5) }}"), "counts frames"));
    CHECK(has(error_of("object a { states { S: JumpFramesIfRandom(1, 190, 200) }}"), "counts frames"));
    CHECK(has(error_of("object a { int x; states { S: JumpNextFrameIfVarEq(x, 1) }}"), "counts frames"));
    // the ones that jump to a state by name are fine
    CHECK(has(error_of("object a { int x; states { S: JumpToStateIfVarLower(S, x, 1) JumpFrames }}"), "counts frames"));
}

TEST(loop_state_goto_and_stop) {
    Object *o = compile_and_load("object a { states { A: Wait 1 Goto B  B: Stop }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Count, 1); // Goto shares the frame of Wait 1
    Cscript_Execute(o, o->State[0]->Functions[0]);
    CHECK_EQ(o->ActualState, 1);
    Cscript_Execute(o, o->State[1]->Functions[0]);
    CHECK(o->CanDelete);
    ObjectCore_Unload(o);
}

// ---- Show / Wait -------------------------------------------------------------

static void write_sprite(const char *name) {
    FILE *f = fopen(name, "wb");
    CHECK(f != 0);
    if (f) { fputs("AB\r\nCD\r\n", f); fclose(f); }
}

TEST(show_belongs_to_the_wait_right_after_it) {
    write_sprite("dot.txt");
    Object *o = compile_and_load(
        "object a { int n = 0; states { S:\n"
        "  Show \"dot.txt\"\n"
        "  Wait 1\n"
        "  Show \"dot.txt\" Wait 2\n"
        "  n++\n"
        "  Wait 3\n"       // the Show above is not linked to this Wait
        "}}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Count, 3);
    CHECK(o->State[0]->Sprites[0] != 0);
    CHECK(o->State[0]->Sprites[0] == o->State[0]->Sprites[1]); // the file is loaded once
    CHECK(o->State[0]->Sprites[2] == 0);
    CHECK_EQ(o->State[0]->WaitSprites[0], 1);
    CHECK_EQ(o->State[0]->WaitSprites[1], 2);
    CHECK_EQ(o->State[0]->WaitSprites[2], 3);
    ObjectCore_Unload(o);
}

TEST(show_none_and_dangling_show) {
    Object *o = compile_and_load("object a { states { S: Show none Wait 4 }}");
    CHECK(o != 0);
    if (o) { CHECK(o->State[0]->Sprites[0] == 0); ObjectCore_Unload(o); }
    CHECK(has(error_of("object a { int x; states { S:\n Show \"dot.txt\"\n x++\n Wait 2 }}"), "line 2: Show must be directly followed by Wait"));
    CHECK(has(error_of("object a { states { S: Show \"dot.txt\" }}"), "Show must be directly followed by Wait"));
    CHECK(has(error_of("object a { states { S: Show x Wait 1 }}"), "or none after Show"));
}

TEST(missing_sprite_file_is_a_load_error) {
    std::string err;
    Object *o = compile_and_load("object a { states { S: Show \"no_such_sprite.txt\" Wait 1 }}", &err);
    CHECK(o == 0);
    CHECK(err.empty()); // it compiles, the file is only needed when the object is loaded
}

// ---- actions share the frame of the Wait before them, unless a jump lands between ------

TEST(action_after_wait_shares_the_frame) {
    Object *o = compile_and_load("object a { int r = 0; states { S: Wait 1  r++  r++ }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK_EQ(o->State[0]->Count, 2);
    CHECK(o->State[0]->Functions[0] != 0);
    CHECK_EQ(o->State[0]->WaitSprites[0], 1);
    CHECK_EQ(o->State[0]->WaitSprites[1], 0);
    ObjectCore_Unload(o);
}

TEST(action_is_not_merged_where_a_jump_lands) {
    // r++ must run after the if, also when the if is skipped
    const char *tmpl =
        "object a { int x = %d; int r = 0; states { Main:\n"
        "  if (x < 5) { Wait 1 }\n"
        "  r++\n"
        "  Goto End\n"
        "End: Wait 1 }}";
    int xs[] = { 4, 6 };
    for (int i = 0; i < 2; i++) {
        char text[300];
        snprintf(text, sizeof(text), tmpl, xs[i]);
        Object *o = compile_and_load(text);
        CHECK(o != 0);
        if (!o) return;
        CHECK_EQ(o->State[0]->Count, 4); // condition, Wait 1, r++, Goto
        CHECK(run_to_end(o));
        CHECK_EQ(var_of(o, "r"), 1);
        ObjectCore_Unload(o);
    }
}

// ---- while / if / else ------------------------------------------------------

TEST(while_loop) {
    Object *o = compile_and_load(
        "object a { int y = 0; int steps = 0; states {\n"
        "Main:\n"
        "  while (y < 5) {\n"
        "    Wait 2\n"
        "    y++\n"
        "    Wait 1\n"
        "    steps++\n"
        "  }\n"
        "  Goto End\n"
        "End: Wait 1 }}");
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
        "  while (y < 5) { Wait 1 y++ }\n"
        "  Goto End\n"
        "End: Wait 1 }}");
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
        "  if (x < 5) { r += 1 } else { r += 10 }\n"
        "  Goto End\n"
        "End: Wait 1 }}";
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
        "  if (x == 1) { r += 1 }\n"
        "  else if (x == 2) { r += 2 }\n"
        "  else if (x == 3) { r += 3 }\n"
        "  else { r += 100 }\n"
        "  if (x > 2) { r += 1000 }\n"
        "  Goto End\n"
        "End: Wait 1 }}";
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
    Object *o = compile_and_load(
        "object a { int i = 0; int even = 0; int odd = 0; states {\n"
        "Main:\n"
        "  while (i < 10) {\n"
        "    if (i == 0 || i == 2 || i == 4 || i == 6 || i == 8) {\n"
        "      Wait 1\n"
        "      even++\n"
        "    } else {\n"
        "      Wait 1\n"
        "      odd++\n"
        "    }\n"
        "    i++\n"
        "  }\n"
        "  Goto End\n"
        "End: Wait 1 }}");
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
        "  if (%s) { r++ }\n"
        "  Goto End\n"
        "End: Wait 1 }}", a, b, c, cond);
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
    // x is 5, y is 9
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
            "  if (%s) { r++ }\n"
            "  Goto End\n"
            "End: Wait 1 }}", cases[k].cond);
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
        "    Wait 1\n"
        "    i++\n"
        "    j--\n"
        "    n++\n"
        "  }\n"
        "  Goto End\n"
        "End: Wait 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(run_to_end(o));
    CHECK_EQ(var_of(o, "n"), 6); // j goes 10 -> 4
    CHECK_EQ(var_of(o, "i"), 6);
    ObjectCore_Unload(o);
}

TEST(control_flow_at_the_end_of_a_state_wraps_to_the_first_frame) {
    // the if is the last statement: when it is skipped the object lands past the last frame
    Object *o = compile_and_load(
        "object a { int n = 0; states {\n"
        "A:\n"
        "  Wait 1\n"
        "  if (n < 3) { Wait 1 n++ }\n"
        "}}");
    CHECK(o != 0);
    if (!o) return;
    for (int tick = 0; tick < 40; tick++) Object_Goto_NextStep(o);
    CHECK_EQ(var_of(o, "n"), 3);
    ObjectCore_Unload(o);
}

// ---- errors ------------------------------------------------------------------

TEST(errors_have_line_numbers_and_reasons) {
    CHECK(has(error_of("object a {\n states {\n S:\n Nope(1)\n }}"), "line 4: unknown function 'Nope'"));
    CHECK(has(error_of("object a { states { S: VarInc }}"), "VarInc needs 1 argument(s), got 0"));
    CHECK(has(error_of("object a { states { S: VarInc(a, b) }}"), "needs 1"));
    CHECK(has(error_of("object a { states { S: VarInc(ghost) }}"), "'ghost' is not a var or a state"));
    CHECK(has(error_of("object a { states { S: ghost++ }}"), "'ghost' is not a var or a state"));
    CHECK(has(error_of("object a { states { S: Goto Nowhere }}"), "'Nowhere' is not a var or a state"));
    CHECK(has(error_of("object a { states { S: Wait }}"), "expected the wait time"));
    CHECK(has(error_of("object a { states { S: Wait 70000 }}"), "expected the wait time"));
    CHECK(has(error_of("object a { int x; states { S: x += }}"), "expected a var or a number"));
    CHECK(has(error_of("object a { states { S: 5 }}"), "expected a statement"));
    CHECK(has(error_of("object a { int x; states { S: x + 1 }}"), "unexpected character '+'"));
    CHECK(has(error_of("object a { states { S: Wait 1  S: Wait 1 }}"), "declared twice"));
    CHECK(has(error_of("object a { int x; int x; states { S: Wait 1 }}"), "declared twice"));
    CHECK(has(error_of("object a { states { S: }}"), "has no frames"));
    CHECK(has(error_of("object a { }"), "no states"));
    CHECK(has(error_of("object a { int a_very_long_variable_name = 1; states { S: Wait 1 }}"), "longer than 18"));
    CHECK(has(error_of("object a { states { S: Show \"never closed Wait 1 }}"), "never closed"));
    CHECK(has(error_of("object a { states { S: Wait 1 } } extra"), "unexpected text"));
    CHECK(has(error_of("object a { states { S: Wait 1 $ }}"), "unexpected character"));
    CHECK(has(error_of("object a { int x = 99999999999; states { S: Wait 1 }}"), "too big"));
    CHECK(has(error_of("nothing"), "expected 'object'"));
    CHECK(has(error_of(""), "expected 'object'"));
}

TEST(control_flow_errors) {
    CHECK(has(error_of("object a { states { S:\n else { } }}"), "line 2: 'else' without 'if'"));
    CHECK(has(error_of("object a { int x; states { S: while x < 1 { Wait 1 } }}"), "expected '('"));
    CHECK(has(error_of("object a { int x; states { S: while (x < 1 { Wait 1 } }}"), "expected ')'"));
    CHECK(has(error_of("object a { int x; states { S: while (x < 1) Wait 1 }}"), "expected '{'"));
    CHECK(has(error_of("object a { int x; states { S: if (x < 1) { Wait 1 "), "expected '}'"));
    CHECK(has(error_of("object a { int x; states { S: Wait 1 }"), "expected 'int', 'solid', 'renderonly', 'states' or '}'"));
    CHECK(has(error_of("object a { int x; states { S: if (x <) { Wait 1 } }}"), "expected a var or a number"));
    CHECK(has(error_of("object a { int x; states { S: if () { Wait 1 } }}"), "expected a var or a number"));
    CHECK(has(error_of("object a { states { S: if (ghost < 1) { Wait 1 } }}"), "'ghost' is not a var or a state"));
    CHECK(has(error_of("object a { int x; states { S: if (x & 1) { Wait 1 } }}"), "unexpected character '&'"));
    CHECK(has(error_of("object a { int x; states { S: if (x < 1) { T: Wait 1 } }}"), "label cannot be inside a block"));
    CHECK(has(error_of("object a { int x; states { S: Wait 1 if (x < 1) { Wait 1 } else }}"), "expected '{'"));
}

// ---- flags, image sprites ---------------------------------------------------

TEST(solid_renderonly_and_image_sprites) {
    Object *o = compile_and_load(
        "object a { solid; renderonly; int Size_X = 32; int Size_Y = 24;\n"
        " states { S:\n"
        "  Show image \"a.png\" 32 24 Wait 1\n"
        "  Show container \"b.png\" 8 9 Wait 2\n"
        "  Show image \"a.png\" 32 24 Wait 3\n"       // same file and scale: loaded once
        "  Show image \"a.png\" 16 24 Wait 4\n"       // other scale: another sprite
        "}}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(o->Solid);
    CHECK(o->RenderOnly);
    CHECK_EQ(ObjectSizeX(o), 32);
    CHECK_EQ(o->State[0]->Sprites[0]->ScaleX, 32);
    CHECK_EQ(o->State[0]->Sprites[1]->ScaleX, 8);
    CHECK_EQ(o->State[0]->Sprites[1]->ScaleY, 9);
    CHECK(o->State[0]->Sprites[0] == o->State[0]->Sprites[2]);
    CHECK(o->State[0]->Sprites[0] != o->State[0]->Sprites[3]);
    ObjectCore_Unload(o);
    Object *plain = compile_and_load("object a { states { S: Wait 1 }}");
    CHECK(plain != 0);
    if (plain) { CHECK(!plain->Solid); CHECK(!plain->RenderOnly); ObjectCore_Unload(plain); }
    CHECK(has(error_of("object a { states { S: Show image \"a.png\" Wait 1 }}"), "expected the sprite width and height"));
    CHECK(has(error_of("object a { states { S: Show image \"a.png\" 0 4 Wait 1 }}"), "expected the sprite width and height"));
    CHECK(has(error_of("object a { states { S: Show image 5 Wait 1 }}"), "expected the \"file\""));
}

// ---- global vars, objects and functions ------------------------------------------

static int fn_calls = 0;
static int fn_sum = 0;
static void fn_count(Object *) { fn_calls++; }
static void fn_add(Object *, MT2D_VAR **v) { fn_sum += v[0]->Data.i + v[1]->Data.i; }

TEST(global_names_are_checked_when_the_object_is_loaded) {
    // compiles without knowing the names
    const char *text =
        "object a { states { S:\n"
        " Wait 1\n"
        " global Health--\n"
        " global Spawn(3, global Speed)\n"
        "}}";
    std::vector<unsigned char> bin;
    std::string err;
    CHECK(CscriptCompiler_Compile(text, bin, err));
    CHECK(err.empty());
    // but loading needs them
    CHECK(ObjectCore_LoadFromMemory(&bin[0], bin.size()) == 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "global var is not registered: 'Health'");
    MT2D_VAR *health = MT2D_Object_Create_Var_Int("Health", 6);
    MT2D_VAR *speed = MT2D_Object_Create_Var_Int("Speed", 4);
    CHECK(ObjectCore_AddGlobalVar(health));
    CHECK(ObjectCore_LoadFromMemory(&bin[0], bin.size()) == 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "global function is not registered: 'Spawn'");
    CHECK(ObjectCore_AddGlobalCScript("Spawn", fn_add, 2));
    CHECK(ObjectCore_LoadFromMemory(&bin[0], bin.size()) == 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "global var is not registered: 'Speed'");
    CHECK(ObjectCore_AddGlobalVar(speed));
    Object *o = ObjectCore_LoadFromMemory(&bin[0], bin.size());
    CHECK(o != 0);
    CHECK_STR_EQ(ObjectCore_GetLastError(), "");
    if (o) {
        CHECK_EQ(o->State[0]->Count, 2);   // Health-- shares the Wait, Spawn is the second frame
        fn_sum = 0;
        Cscript_Execute(o, o->State[0]->Functions[0]);
        CHECK_EQ(health->Data.i, 5);
        Cscript_Execute(o, o->State[0]->Functions[1]);
        CHECK_EQ(fn_sum, 7);
        ObjectCore_Unload(o);
    }
    CHECK(ObjectCore_RemoveGlobalVar("Health"));
    CHECK(ObjectCore_RemoveGlobalVar("Speed"));
    CHECK(ObjectCore_RemoveGlobalCScript("Spawn"));
    MT2D_VAR_Free(health, 1);   // the registry does not own the vars
    MT2D_VAR_Free(speed, 1);
}

TEST(global_vars_in_operands_and_conditions) {
    MT2D_VAR *speed = MT2D_Object_Create_Var_Int("GameSpeed", 3);
    MT2D_VAR *req = MT2D_Object_Create_Var_Char("JmpReq", 1);
    CHECK(ObjectCore_AddGlobalVar(speed));
    CHECK(ObjectCore_AddGlobalVar(req));
    Object *o = compile_and_load(
        "object a { int Pos_X = 100; int r = 0; states {\n"
        "Main:\n"
        "  Pos_X -= global GameSpeed\n"
        "  if (global JmpReq == 1) { r += 10 }\n"
        "  global JmpReq = 0\n"
        "  if (global JmpReq == 1) { r += 100 } else { r += 1000 }\n"
        "  Goto End\n"
        "End: Wait 1 }}");
    CHECK(o != 0);
    if (o) {
        CHECK(run_to_end(o));
        CHECK_EQ(ObjectPosX(o), 97);
        CHECK_EQ(var_of(o, "r"), 1010);
        CHECK_EQ(req->Data.c, 0);
        ObjectCore_Unload(o);
    }
    CHECK(ObjectCore_RemoveGlobalVar("GameSpeed"));
    CHECK(ObjectCore_RemoveGlobalVar("JmpReq"));
    MT2D_VAR_Free(speed, 1);
    MT2D_VAR_Free(req, 1);
}

TEST(global_function_without_arguments_and_errors) {
    CHECK(ObjectCore_AddGlobalCScript("Ping", fn_count));
    Object *o = compile_and_load("object a { states { S: Wait 1  global Ping()  global Ping }}");
    CHECK(o != 0);
    if (o) {
        CHECK_EQ(o->State[0]->Count, 2);
        fn_calls = 0;
        Cscript_Execute(o, o->State[0]->Functions[0]);
        Cscript_Execute(o, o->State[0]->Functions[1]);
        CHECK_EQ(fn_calls, 2);
        ObjectCore_Unload(o);
    }
    CHECK(ObjectCore_RemoveGlobalCScript("Ping"));
    CHECK(has(error_of("object a { states { S: global }}"), "expected a global name"));
    CHECK(has(error_of("object a { states { S: global 5 }}"), "expected a global name"));
    CHECK(has(error_of("object a { int x; states { S: x = global }}"), "expected a global name"));
}

TEST(object_functions_need_a_global_object) {
    Object *target = compile_and_load("object t { solid; states { S: Wait 1 }}");
    CHECK(target != 0);
    if (!target) return;
    CHECK(ObjectCore_AddGlobalObject("Doggo", target));
    Object *o = compile_and_load("object a { states { S: Wait 1  GotoStateIfHitObject(global Doggo, S) }}");
    CHECK(o != 0);
    if (o) {
        CHECK_EQ(o->State[0]->Count, 1);   // the call shares the frame of Wait 1
        CHECK(o->State[0]->Functions[0]->TempObj == target);
        ObjectCore_Unload(o);
    }
    CHECK(has(error_of("object a { states { S: GotoStateIfHitObject(Doggo, S) }}"), "needs a global object as its first argument"));
    CHECK(has(error_of("object a { states { S: GotoStateIfHitObject(3, S) }}"), "needs a global object as its first argument"));
    CHECK(has(error_of("object a { states { S: GotoStateIfHitObject(global Doggo) }}"), "needs 2 argument(s), got 1"));
    CHECK(has(error_of("object a { states { S: CreateObject(global Doggo, 1, 2) }}"), "needs 5 argument(s), got 3"));
    CHECK(ObjectCore_RemoveGlobalObject("Doggo"));
    ObjectCore_Unload(target);
}

// ---- random -------------------------------------------------------------------

TEST(random_condition) {
    srand(12345);
    Object *o = compile_and_load(
        "object a { int n = 0; int hit = 0; int miss = 0; int never = 0; int always = 0;\n"
        "  int notHit = 0; states { Main:\n"
        "  while (n < 2000) {\n"
        "    n++\n"
        "    if (random(25)) { hit++ } else { miss++ }\n"
        "    if (random(0)) { never++ }\n"
        "    if (random(100)) { always++ }\n"
        "    if (!random(25)) { notHit++ }\n"
        "  }\n"
        "  Goto End\n"
        "End: Wait 1 }}");
    CHECK(o != 0);
    if (!o) return;
    CHECK(run_to_end(o));
    CHECK_EQ(var_of(o, "hit") + var_of(o, "miss"), 2000);
    CHECK(var_of(o, "hit") > 350 && var_of(o, "hit") < 650);       // about 25%
    CHECK_EQ(var_of(o, "never"), 0);
    CHECK_EQ(var_of(o, "always"), 2000);
    CHECK(var_of(o, "notHit") > 1350 && var_of(o, "notHit") < 1650); // about 75%
    ObjectCore_Unload(o);
    CHECK(has(error_of("object a { states { S: if (random(101)) { Wait 1 } }}"), "chance in percent"));
    CHECK(has(error_of("object a { int x; states { S: if (random(x)) { Wait 1 } }}"), "chance in percent"));
}

// ---- a Shibe style object ----------------------------------------------------------

TEST(scrolling_tile_with_a_global_speed) {
    MT2D_VAR *speed = MT2D_Object_Create_Var_Int("GameSpeed", 50);
    CHECK(ObjectCore_AddGlobalVar(speed));
    Object *tile = compile_and_load(
        "object tile {\n"
        "    solid;\n"
        "    int Size_X = 102; int Size_Y = 24;\n"
        "    states { loop:\n"
        "        Show container \"Sprites/Game/decoration/Platform/normal.png\" 102 24\n"
        "        Wait 1\n"
        "        Pos_X -= global GameSpeed\n"
        "        if (Pos_X < -100) { Pos_X = 400 }\n"
        "    }\n"
        "}");
    CHECK(tile != 0);
    if (!tile) return;
    Object *a = ObjectCore_Clone(tile, 120, 0);
    Object *b = ObjectCore_Clone(tile, 20, 0);
    CHECK(a != 0 && b != 0);
    if (a && b) {
        CHECK(a->Solid && a->State == tile->State);
        int wraps = 0;
        int last = ObjectPosX(a);
        for (int t = 0; t < 40; t++) {
            Object_Goto_NextStep(a);
            if (ObjectPosX(a) > last) wraps++;
            last = ObjectPosX(a);
        }
        CHECK_EQ(ObjectPosX(b), 20);               // b did not run
        CHECK(wraps >= 1);                        // it left the screen and came back at 400
        int sawFour = 0;
        for (int t = 0; t < 60 && !sawFour; t++) {
            Object_Goto_NextStep(b);
            if (ObjectPosX(b) == 400) sawFour = 1;
        }
        CHECK(sawFour);
        speed->Data.i = 0;                         // the speed is read live
        int frozen = ObjectPosX(b);
        for (int t = 0; t < 10; t++) Object_Goto_NextStep(b);
        CHECK_EQ(ObjectPosX(b), frozen);
    }
    Object_Delete(a, false);
    Object_Delete(b, false);
    ObjectCore_Unload(tile);
    CHECK(ObjectCore_RemoveGlobalVar("GameSpeed"));
    MT2D_VAR_Free(speed, 1);
}

TEST(load_script_from_text_file) {
    FILE *f = fopen("script.txt", "wb");
    CHECK(f != 0);
    if (!f) return;
    fputs("object a { int hp = 3; states { S: Wait 1 } }", f);
    fclose(f);
    std::string err;
    Object *o = ObjectCore_LoadScript("script.txt", &err);
    CHECK(o != 0);
    if (o) CHECK_EQ(var_of(o, "hp"), 3);
    ObjectCore_Unload(o);
    CHECK(ObjectCore_LoadScript("missing.txt", &err) == 0);
    CHECK(has(err, "cannot open"));
}

MT2D_TEST_MAIN()

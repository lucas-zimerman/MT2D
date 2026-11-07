// Tests for MT2D/ObjectCore/Cscript.cpp
#include <mt2d_test.h>
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/Object.h>
#include <stdlib.h>

static int calls_obj = 0;
static int calls_vars = 0;
static int calls_obj_obj_vars = 0;
static Object *last_caller = 0;
static Object *last_model = 0;
static MT2D_VAR **last_vars = 0;

static void reset() {
    calls_obj = calls_vars = calls_obj_obj_vars = 0;
    last_caller = last_model = 0;
    last_vars = 0;
}

static void fn_obj(Object *o) { calls_obj++; last_caller = o; }
static void fn_vars(Object *o, MT2D_VAR **v) { calls_vars++; last_caller = o; last_vars = v; }
static void fn_obj_obj_vars(Object *o, Object *m, MT2D_VAR **v) {
    calls_obj_obj_vars++; last_caller = o; last_model = m; last_vars = v;
}
static void fn_void() {}

TEST(empty_has_no_functions) {
    Cscript *c = Cscript_Empty();
    CHECK(c->_Obj == 0);
    CHECK(c->_Obj_Vars == 0);
    CHECK(c->_Obj_Obj_Vars == 0);
    CHECK(c->TempObj == 0);
    CHECK(c->TempVars == 0);
    free(c);
}

TEST(new1_stores_void_function) {
    Cscript *c = Cscript_New1(fn_void);
    CHECK(c->_void == fn_void);
    CHECK(c->_Obj == 0);
    free(c);
}

TEST(new2_and_execute_obj) {
    reset();
    Cscript *c = Cscript_New2(fn_obj);
    CHECK(c->_Obj == fn_obj);
    Object o;
    Cscript_Execute(&o, c);
    CHECK_EQ(calls_obj, 1);
    CHECK(last_caller == &o);
    free(c);
}

TEST(new3_and_execute_obj_vars) {
    reset();
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("V", 1);
    MT2D_VAR **arr = MT2D_VAR_Create_Matrix1(v);
    Cscript *c = Cscript_New3(arr, fn_vars);
    CHECK(c->TempVars == arr);
    Object o;
    Cscript_Execute(&o, c);
    CHECK_EQ(calls_vars, 1);
    CHECK(last_caller == &o);
    CHECK(last_vars == arr);
    free(arr);
    free(c);
    MT2D_VAR_Free(v, 1);
}

TEST(new4_and_execute_obj_obj_vars) {
    reset();
    Object caller, model;
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("V", 1);
    MT2D_VAR **arr = MT2D_VAR_Create_Matrix1(v);
    Cscript *c = Cscript_New4(&model, arr, fn_obj_obj_vars);
    CHECK(c->TempObj == &model);
    Cscript_Execute(&caller, c);
    CHECK_EQ(calls_obj_obj_vars, 1);
    CHECK(last_caller == &caller);
    CHECK(last_model == &model);
    CHECK(last_vars == arr);
    free(arr);
    free(c);
    MT2D_VAR_Free(v, 1);
}

TEST(execute_ignores_null_arguments) {
    reset();
    Object o;
    Cscript *c = Cscript_New2(fn_obj);
    Cscript_Execute(&o, 0);
    Cscript_Execute(0, c);
    CHECK_EQ(calls_obj, 0);
    free(c);
}

TEST(remove_from_memory_accepts_null_and_valid) {
    Cscript_RemoveFromMemory(0);
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("V", 1);
    // TempVars (the array of pointers) is released, the vars themselves are not.
    Cscript *c = Cscript_New3(MT2D_VAR_Create_Matrix1(v), fn_vars);
    Cscript_RemoveFromMemory(c);
    CHECK_EQ(*(int *)v->Data, 1);
    MT2D_VAR_Free(v, 1);
}

MT2D_TEST_MAIN()

TEST(generic_new_picks_overload_and_counts_vars) {
    reset();
    Object o;
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("a", 1);
    MT2D_VAR *b = MT2D_Object_Create_Var_Int("b", 2);
    MT2D_VAR *c = MT2D_Object_Create_Var_Int("c", 3);

    Cscript *s0 = Cscript_New(fn_void);
    CHECK(s0->_void == fn_void);
    Cscript *s1 = Cscript_New(fn_obj);
    CHECK(s1->_Obj == fn_obj);

    Cscript *s3 = Cscript_New(fn_vars, a, b, c);
    CHECK(s3->_Obj_Vars == fn_vars);
    CHECK(s3->TempVars[0] == a && s3->TempVars[1] == b && s3->TempVars[2] == c);
    Cscript_Execute(&o, s3);
    CHECK_EQ(calls_vars, 1);

    Cscript *s4 = Cscript_New(fn_obj_obj_vars, &o, a, b);
    CHECK(s4->_Obj_Obj_Vars == fn_obj_obj_vars);
    CHECK(s4->TempObj == &o);
    CHECK(s4->TempVars[0] == a && s4->TempVars[1] == b);
    free(s0); free(s1); free(s3->TempVars); free(s3); free(s4->TempVars); free(s4);
}

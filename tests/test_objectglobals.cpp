// Tests for MT2D/ObjectCore/ObjectGlobals.cpp (the registry of global vars, objects and functions)
#include <mt2d_test.h>
#include <MT2D/ObjectCore/ObjectGlobals.h>
#include <string.h>

static void f_obj(Object *) {}
static void f_vars(Object *, MT2D_VAR **) {}
static void f_objobj(Object *, Object *, MT2D_VAR **) {}

TEST(global_vars_add_find_remove) {
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("A", 1);
    MT2D_VAR *b = MT2D_Object_Create_Var_Int("B", 2);
    MT2D_VAR *c = MT2D_Object_Create_Var_Int("C", 3);
    CHECK(ObjectCore_FindGlobalVar("A") == 0);
    CHECK(ObjectCore_AddGlobalVar(a));
    CHECK(ObjectCore_AddGlobalVar(b));
    CHECK(ObjectCore_AddGlobalVar(c));
    CHECK(ObjectCore_FindGlobalVar("A") == a);
    CHECK(ObjectCore_FindGlobalVar("B") == b);
    CHECK(ObjectCore_FindGlobalVar("C") == c);
    CHECK(ObjectCore_RemoveGlobalVar("B"));          // from the middle
    CHECK(ObjectCore_FindGlobalVar("B") == 0);
    CHECK(ObjectCore_FindGlobalVar("A") == a);
    CHECK(ObjectCore_FindGlobalVar("C") == c);
    CHECK(!ObjectCore_RemoveGlobalVar("B"));         // already gone
    CHECK(ObjectCore_RemoveGlobalVar("A"));
    CHECK(ObjectCore_RemoveGlobalVar("C"));
    CHECK(ObjectCore_FindGlobalVar("C") == 0);
    CHECK(ObjectCore_AddGlobalVar(a));               // can be added again after the registry emptied
    CHECK(ObjectCore_RemoveGlobalVar("A"));
    MT2D_VAR_Free(a, 1); MT2D_VAR_Free(b, 1); MT2D_VAR_Free(c, 1);   // the registry does not own the vars
}

TEST(global_vars_reject_bad_input) {
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("Dup", 1);
    MT2D_VAR *same = MT2D_Object_Create_Var_Int("Dup", 2);
    MT2D_VAR *unnamed = MT2D_Object_Create_Var_Int("", 2);
    CHECK(!ObjectCore_AddGlobalVar(0));
    CHECK(!ObjectCore_AddGlobalVar(unnamed));
    CHECK(ObjectCore_AddGlobalVar(a));
    CHECK(!ObjectCore_AddGlobalVar(same));           // duplicate name
    CHECK(ObjectCore_FindGlobalVar("Dup") == a);
    CHECK(!ObjectCore_RemoveGlobalVar(0));
    CHECK(ObjectCore_RemoveGlobalVar("Dup"));
    MT2D_VAR_Free(a, 1); MT2D_VAR_Free(same, 1); MT2D_VAR_Free(unnamed, 1);
}

TEST(global_objects_add_find_remove) {
    Object *o = (Object *)0x1000;                    // never dereferenced by the registry
    Object *p = (Object *)0x2000;
    CHECK(ObjectCore_AddGlobalObject("Doggo", o));
    CHECK(ObjectCore_AddGlobalObject("Boss", p));
    CHECK(!ObjectCore_AddGlobalObject("Doggo", p));
    CHECK(!ObjectCore_AddGlobalObject("Nobody", 0));
    CHECK(!ObjectCore_AddGlobalObject("", o));
    CHECK(!ObjectCore_AddGlobalObject("a_name_longer_than_18_chars", o));
    CHECK(ObjectCore_FindGlobalObject("Doggo") == o);
    CHECK(ObjectCore_FindGlobalObject("Boss") == p);
    CHECK(ObjectCore_RemoveGlobalObject("Doggo"));
    CHECK(ObjectCore_FindGlobalObject("Doggo") == 0);
    CHECK(ObjectCore_FindGlobalObject("Boss") == p);
    CHECK(!ObjectCore_RemoveGlobalObject("Doggo"));
    CHECK(ObjectCore_RemoveGlobalObject("Boss"));
}

TEST(global_functions_keep_their_signature) {
    CHECK(ObjectCore_AddGlobalCScript("One", f_obj));
    CHECK(ObjectCore_AddGlobalCScript("Two", f_vars, 3));
    CHECK(ObjectCore_AddGlobalCScript("Three", f_objobj));
    ObjectCore_CscriptEntry e;
    CHECK(ObjectCore_FindGlobalCScript("One", &e));
    CHECK_EQ(e.Kind, CSCRIPT_KIND_OBJ);
    CHECK_EQ(e.ArgCount, 0);
    CHECK(e.Obj == f_obj);
    CHECK(ObjectCore_FindGlobalCScript("Two", &e));
    CHECK_EQ(e.Kind, CSCRIPT_KIND_VARS);
    CHECK_EQ(e.ArgCount, 3);
    CHECK(e.Vars == f_vars);
    CHECK(ObjectCore_FindGlobalCScript("Three", &e));
    CHECK_EQ(e.Kind, CSCRIPT_KIND_OBJ_OBJ_VARS);
    CHECK_EQ(e.ArgCount, -1);                        // any amount
    CHECK(e.ObjObj == f_objobj);
    CHECK(!ObjectCore_FindGlobalCScript("Four", &e));
    CHECK(ObjectCore_FindGlobalCScript("Two", 0));   // the result is optional
    CHECK(!ObjectCore_AddGlobalCScript("One", f_obj));
    CHECK(!ObjectCore_AddGlobalCScript("Null", (void (*)(Object *))0));
    CHECK(!ObjectCore_AddGlobalCScript("a_name_longer_than_18_chars", f_obj));
    CHECK(ObjectCore_RemoveGlobalCScript("Two"));
    CHECK(!ObjectCore_FindGlobalCScript("Two", 0));
    CHECK(ObjectCore_FindGlobalCScript("One", 0));
    CHECK(!ObjectCore_RemoveGlobalCScript("Two"));
    CHECK(ObjectCore_RemoveGlobalCScript("One"));
    CHECK(ObjectCore_RemoveGlobalCScript("Three"));
}

TEST(vars_objects_and_functions_are_separate_names) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("Same", 1);
    CHECK(ObjectCore_AddGlobalVar(v));
    CHECK(ObjectCore_AddGlobalObject("Same", (Object *)0x1000));
    CHECK(ObjectCore_AddGlobalCScript("Same", f_obj));
    CHECK(ObjectCore_RemoveGlobalObject("Same"));
    CHECK(ObjectCore_FindGlobalVar("Same") == v);
    CHECK(ObjectCore_FindGlobalCScript("Same", 0));
    CHECK(ObjectCore_RemoveGlobalVar("Same"));
    CHECK(ObjectCore_RemoveGlobalCScript("Same"));
    MT2D_VAR_Free(v, 1);
}

TEST(last_error_text) {
    ObjectCore_SetLastError("something is missing:", "Thing");
    CHECK_STR_EQ(ObjectCore_GetLastError(), "something is missing: 'Thing'");
    ObjectCore_ClearLastError();
    CHECK_STR_EQ(ObjectCore_GetLastError(), "");
    ObjectCore_SetLastError("very long text that does not fit the small buffer of the error message at all, really not", "an equally long name that has to be cut by the buffer too");
    CHECK(strlen(ObjectCore_GetLastError()) < 96);
}

MT2D_TEST_MAIN()

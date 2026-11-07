// Tests for MT2D/ObjectCore/Vars.cpp
#include <mt2d_test.h>
#include <MT2D/ObjectCore/Vars.h>
#include <stdlib.h>
#include <string.h>

TEST(create_int_var) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("Health", 42);
    CHECK(v != 0);
    CHECK_EQ(v->Type, VAR_INT);
    CHECK_STR_EQ(v->Name, "Health");
    CHECK_EQ(*(int *)MT2D_Object_Get_Var_Data(v), 42);
    CHECK_EQ(MT2D_Var_Last_Error, Var_No_Error);
    MT2D_VAR_Free(v, 1);
}

TEST(create_var_copies_name_and_data) {
    char name[] = "Original";
    MT2D_VAR *v = MT2D_Object_Create_Var_Int(name, 7);
    strcpy(name, "Changed!");
    CHECK_STR_EQ(v->Name, "Original");
    MT2D_VAR_Free(v, 1);
}

TEST(create_char_var) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Char("Letter", 'x');
    CHECK_EQ(v->Type, VAR_CHAR);
    CHECK_EQ(*(char *)v->Data, 'x');
    MT2D_VAR_Free(v, 1);
}

TEST(create_pointer_var_has_no_data) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Pointer("Pos_X");
    CHECK_EQ(v->Type, VAR_POINTER);
    CHECK(v->Data == 0);
    CHECK_STR_EQ(v->Name, "Pos_X");
    MT2D_VAR_Free(v, 1);
}

TEST(create_generic_float_var) {
    float f = 1.5f;
    MT2D_VAR *v = MT2D_Object_Create_Var("Speed", VAR_FLOAT, &f);
    CHECK_EQ(v->Type, VAR_FLOAT);
    CHECK(*(float *)v->Data == 1.5f);
    // the var owns a copy, not the caller's pointer
    CHECK(v->Data != (void *)&f);
    MT2D_VAR_Free(v, 1);
}

TEST(create_var_with_null_data_reports_error) {
    MT2D_VAR *v = MT2D_Object_Create_Var("Broken", VAR_INT, NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Null_Data);
    free(v);
    MT2D_Var_Last_Error = Var_No_Error;
}

TEST(get_data_of_null_var) {
    CHECK(MT2D_Object_Get_Var_Data(NULL) == NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Not_Found);
    MT2D_Var_Last_Error = Var_No_Error;
}

TEST(set_var_generic_same_type_replaces_data) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("A", 1);
    int *fresh = (int *)malloc(sizeof(int));
    *fresh = 99;
    MT2D_Object_Set_Var(v, (void *)fresh, VAR_INT);
    CHECK_EQ(MT2D_Var_Last_Error, Var_No_Error);
    CHECK(v->Data == fresh);
    CHECK_EQ(*(int *)v->Data, 99);
    MT2D_VAR_Free(v, 1);
}

TEST(set_var_generic_wrong_type_is_rejected) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("A", 1);
    void *before = v->Data;
    float *f = (float *)malloc(sizeof(float));
    *f = 2.0f;
    MT2D_Object_Set_Var(v, (void *)f, VAR_FLOAT);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Wrong_Type);
    CHECK(v->Data == before);
    CHECK_EQ(*(int *)v->Data, 1);
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_VAR_Free(v, 1);
}

TEST(set_var_generic_null_var) {
    int *i = (int *)malloc(sizeof(int));
    MT2D_Object_Set_Var(NULL, (void *)i, VAR_INT);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Not_Found);
    MT2D_Var_Last_Error = Var_No_Error;
    free(i);
}

TEST(get_var_pointer_by_name) {
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("A", 1);
    MT2D_VAR *b = MT2D_Object_Create_Var_Int("B", 2);
    MT2D_VAR array[2] = {*a, *b};
    MT2D_VAR *found = MT2D_Object_Get_Var_Pointer(array, (char *)"B");
    CHECK(found == &array[1]);
    CHECK_EQ(*(int *)found->Data, 2);
    // names are case sensitive, a missing name is not tested because the lookup
    // does not stop at the end of the array.
    CHECK(MT2D_Object_Get_Var_Pointer(array, (char *)"A") == &array[0]);
    MT2D_VAR_Free(a, 1);
    MT2D_VAR_Free(b, 1);
}

TEST(get_var_pointer_null_name) {
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("A", 1);
    CHECK(MT2D_Object_Get_Var_Pointer(a, NULL) == NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Null_Name);
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_VAR_Free(a, 1);
}

TEST(var_get_int_converts_types) {
    MT2D_VAR *i = MT2D_Object_Create_Var_Int("I", -5);
    MT2D_VAR *c = MT2D_Object_Create_Var_Char("C", 10);
    float f = 7.9f;
    MT2D_VAR *fl = MT2D_Object_Create_Var("F", VAR_FLOAT, &f);
    CHECK_EQ(MT2D_Object_VAR_GetInt(i), -5);
    CHECK_EQ(MT2D_Object_VAR_GetInt(c), 10);
    CHECK_EQ(MT2D_Object_VAR_GetInt(fl), 7); // truncated
    MT2D_VAR_Free(i, 1);
    MT2D_VAR_Free(c, 1);
    MT2D_VAR_Free(fl, 1);
}

TEST(var_get_int_unsupported_type) {
    MT2D_VAR *p = MT2D_Object_Create_Var_Pointer("P");
    CHECK_EQ(MT2D_Object_VAR_GetInt(p), 0);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Type_Not_Supported);
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_VAR_Free(p, 1);
}

TEST(var_get_bool) {
    MT2D_VAR *pos = MT2D_Object_Create_Var_Int("P", 3);
    MT2D_VAR *zero = MT2D_Object_Create_Var_Int("Z", 0);
    MT2D_VAR *neg = MT2D_Object_Create_Var_Int("N", -1);
    MT2D_VAR *c = MT2D_Object_Create_Var_Char("C", 1);
    CHECK(MT2D_Object_VAR_GetBool(pos));
    CHECK(!MT2D_Object_VAR_GetBool(zero));
    CHECK(!MT2D_Object_VAR_GetBool(neg)); // only values > 0 are true
    CHECK(MT2D_Object_VAR_GetBool(c));
    MT2D_VAR_Free(pos, 1);
    MT2D_VAR_Free(zero, 1);
    MT2D_VAR_Free(neg, 1);
    MT2D_VAR_Free(c, 1);
}

TEST(var_set_var_keeps_destination_type) {
    MT2D_VAR *dst = MT2D_Object_Create_Var_Int("D", 0);
    MT2D_VAR *c = MT2D_Object_Create_Var_Char("C", 9);
    float f = 3.7f;
    MT2D_VAR *fl = MT2D_Object_Create_Var("F", VAR_FLOAT, &f);
    MT2D_Object_VAR_SetVar(dst, c);
    CHECK_EQ(*(int *)dst->Data, 9);
    MT2D_Object_VAR_SetVar(dst, fl);
    CHECK_EQ(*(int *)dst->Data, 3);
    CHECK_EQ(dst->Type, VAR_INT);
    MT2D_VAR *src = MT2D_Object_Create_Var_Int("S", 100);
    MT2D_Object_VAR_SetVar(dst, src);
    CHECK_EQ(*(int *)dst->Data, 100);
    MT2D_VAR_Free(dst, 1);
    MT2D_VAR_Free(c, 1);
    MT2D_VAR_Free(fl, 1);
    MT2D_VAR_Free(src, 1);
}

TEST(var_set_var_on_pointer_type_is_unsupported) {
    MT2D_VAR *p = MT2D_Object_Create_Var_Pointer("P");
    MT2D_VAR *i = MT2D_Object_Create_Var_Int("I", 1);
    MT2D_Object_VAR_SetVar(p, i);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Type_Not_Supported);
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_VAR_Free(p, 1);
    MT2D_VAR_Free(i, 1);
}

TEST(add_and_sub_int) {
    MT2D_VAR *store = MT2D_Object_Create_Var_Int("S", 10);
    MT2D_VAR *five = MT2D_Object_Create_Var_Int("F", 5);
    MT2D_VAR *c = MT2D_Object_Create_Var_Char("C", 2);
    MT2D_Object_ADD(store, five);
    CHECK_EQ(*(int *)store->Data, 15);
    MT2D_Object_SUB(store, five);
    CHECK_EQ(*(int *)store->Data, 10);
    MT2D_Object_ADD(store, c);
    CHECK_EQ(*(int *)store->Data, 12);
    MT2D_Object_SUB(store, c);
    CHECK_EQ(*(int *)store->Data, 10);
    MT2D_VAR_Free(store, 1);
    MT2D_VAR_Free(five, 1);
    MT2D_VAR_Free(c, 1);
}

TEST(add_and_sub_char) {
    MT2D_VAR *store = MT2D_Object_Create_Var_Char("S", 10);
    MT2D_VAR *three = MT2D_Object_Create_Var_Int("T", 3);
    MT2D_Object_ADD(store, three);
    CHECK_EQ(*(char *)store->Data, 13);
    MT2D_Object_SUB(store, three);
    CHECK_EQ(*(char *)store->Data, 10);
    MT2D_VAR_Free(store, 1);
    MT2D_VAR_Free(three, 1);
}

TEST(clone_int_and_char) {
    MT2D_VAR *i = MT2D_Object_Create_Var_Int("I", 8);
    MT2D_VAR *ci = MT2D_VAR_CLONE(i);
    CHECK(ci != i);
    CHECK(ci->Data != i->Data);
    CHECK(ci->Name != i->Name);
    CHECK_STR_EQ(ci->Name, "I");
    CHECK_EQ(*(int *)ci->Data, 8);
    *(int *)i->Data = 1;
    CHECK_EQ(*(int *)ci->Data, 8);

    MT2D_VAR *c = MT2D_Object_Create_Var_Char("C", 'q');
    MT2D_VAR *cc = MT2D_VAR_CLONE(c);
    CHECK_EQ(cc->Type, VAR_CHAR);
    CHECK_EQ(*(char *)cc->Data, 'q');
    MT2D_VAR_Free(i, 1);
    MT2D_VAR_Free(ci, 1);
    MT2D_VAR_Free(c, 1);
    MT2D_VAR_Free(cc, 1);
}

TEST(clone_unsupported_type_returns_null) {
    MT2D_VAR *p = MT2D_Object_Create_Var_Pointer("P");
    CHECK(MT2D_VAR_CLONE(p) == 0);
    MT2D_VAR_Free(p, 1);
}

TEST(create_matrices) {
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("A", 1);
    MT2D_VAR *b = MT2D_Object_Create_Var_Int("B", 2);
    MT2D_VAR *c = MT2D_Object_Create_Var_Int("C", 3);
    MT2D_VAR *d = MT2D_Object_Create_Var_Int("D", 4);
    MT2D_VAR *e = MT2D_Object_Create_Var_Int("E", 5);

    MT2D_VAR **m1 = MT2D_VAR_Create_Matrix1(a);
    CHECK(m1[0] == a);
    MT2D_VAR **m2 = MT2D_VAR_Create_Matrix2(a, b);
    CHECK(m2[0] == a && m2[1] == b);
    MT2D_VAR **m3 = MT2D_VAR_Create_Matrix3(a, b, c);
    CHECK(m3[0] == a && m3[1] == b && m3[2] == c);
    MT2D_VAR **m4 = MT2D_VAR_Create_Matrix4(a, b, c, d);
    CHECK(m4[0] == a && m4[1] == b && m4[2] == c && m4[3] == d);
    MT2D_VAR **m5 = MT2D_VAR_Create_MatrixN(5, a, b, c, d, e);
    CHECK(m5[0] == a && m5[1] == b && m5[2] == c && m5[3] == d && m5[4] == e);

    free(m1); free(m2); free(m3); free(m4); free(m5);
    MT2D_VAR_Free(a, 1); MT2D_VAR_Free(b, 1); MT2D_VAR_Free(c, 1);
    MT2D_VAR_Free(d, 1); MT2D_VAR_Free(e, 1);
}

MT2D_TEST_MAIN()

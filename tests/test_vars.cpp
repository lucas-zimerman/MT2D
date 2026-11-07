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
    CHECK_EQ(v->Data.c, 'x');
    MT2D_VAR_Free(v, 1);
}

TEST(create_pointer_var_has_no_data) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Pointer("Pos_X");
    CHECK_EQ(v->Type, VAR_POINTER);
    CHECK(v->Data.p == 0);
    CHECK_STR_EQ(v->Name, "Pos_X");
    MT2D_VAR_Free(v, 1);
}

TEST(create_generic_float_var) {
    float f = 1.5f;
    MT2D_VAR *v = MT2D_Object_Create_Var("Speed", VAR_FLOAT, &f);
    CHECK_EQ(v->Type, VAR_FLOAT);
    CHECK(v->Data.f == 1.5f);
    // the value is stored inline: changing the source does not affect the var
    f = 9.0f;
    CHECK(v->Data.f == 1.5f);
    MT2D_VAR_Free(v, 1);
}

TEST(create_var_with_null_data_reports_error) {
    MT2D_VAR *v = MT2D_Object_Create_Var("Broken", VAR_INT, NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Null_Data);
    CHECK(v == NULL);
    MT2D_Var_Last_Error = Var_No_Error;
}

TEST(get_data_of_null_var) {
    CHECK(MT2D_Object_Get_Var_Data(NULL) == NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Not_Found);
    MT2D_Var_Last_Error = Var_No_Error;
}

TEST(set_var_generic_same_type_copies_value) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("A", 1);
    int fresh = 99;
    MT2D_Object_Set_Var(v, (void *)&fresh, VAR_INT);
    CHECK_EQ(MT2D_Var_Last_Error, Var_No_Error);
    CHECK_EQ(v->Data.i, 99);
    fresh = 5;
    CHECK_EQ(v->Data.i, 99); // copied, not aliased
    MT2D_VAR_Free(v, 1);
}

TEST(set_var_overloads_store_the_value) {
    MT2D_VAR *i = MT2D_Object_Create_Var_Int("I", 0);
    MT2D_VAR *f = MT2D_Object_Create_Var_Float("F", 0.0f);
    MT2D_Object_Set_Var(i, 12);
    MT2D_Object_Set_Var(f, 2.5f);
    CHECK_EQ(i->Data.i, 12);
    CHECK(f->Data.f == 2.5f);
    MT2D_Object_Set_Var(i, true);
    CHECK_EQ(i->Data.i, 1);
    MT2D_Object_Set_Var(i, 2.5f); // wrong type for an int var
    CHECK_EQ(MT2D_Var_Last_Error, Var_Wrong_Type);
    CHECK_EQ(i->Data.i, 1);
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_VAR_Free(i, 1);
    MT2D_VAR_Free(f, 1);
}

TEST(set_var_generic_pointer_stores_the_pointer) {
    MT2D_VAR *p = MT2D_Object_Create_Var_Pointer("P");
    int target = 3;
    MT2D_Object_Set_Var(p, (void *)&target, VAR_POINTER);
    CHECK(p->Data.p == &target);
    CHECK(MT2D_Object_Get_Var_Data(p) == &target);
    MT2D_VAR_Free(p, 0);
}

TEST(var_value_and_name_are_inline) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("A", 4);
    CHECK((void *)MT2D_Object_Get_Var_Data(v) == (void *)&v->Data);
    CHECK((char *)v->Name >= (char *)v && (char *)v->Name < (char *)v + sizeof(MT2D_VAR));
    // no pointer inside: value + type + name (+ alignment padding)
    CHECK(sizeof(MT2D_VAR) <= sizeof(MT2D_VAR_DATA) + 1 + MT2D_VAR_NAME_MAX + sizeof(void *));
    if (sizeof(void *) == 4) CHECK_EQ(sizeof(MT2D_VAR), 24); // MS-DOS / 32 bit layout
    MT2D_VAR_Free(v, 1);
}

TEST(name_at_the_limit_fits_and_longer_is_rejected) {
    char fits[MT2D_VAR_NAME_MAX];
    char tooLong[MT2D_VAR_NAME_MAX + 1];
    memset(fits, 'a', MT2D_VAR_NAME_MAX - 1); fits[MT2D_VAR_NAME_MAX - 1] = 0;
    memset(tooLong, 'a', MT2D_VAR_NAME_MAX); tooLong[MT2D_VAR_NAME_MAX] = 0;
    MT2D_VAR *v = MT2D_Object_Create_Var_Int(fits, 1);
    CHECK(v != 0);
    CHECK_STR_EQ(v->Name, fits);
    CHECK_EQ(MT2D_Var_Last_Error, Var_No_Error);
    MT2D_VAR_Free(v, 1);

    CHECK(MT2D_Object_Create_Var_Int(tooLong, 1) == 0);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Name_Too_Long);
    CHECK(MT2D_Object_Create_Var_Pointer(tooLong) == 0);
    CHECK(MT2D_Object_Create_Var_String(tooLong, "x") == 0);
    CHECK_EQ(MT2D_Object_Get_Last_Var_Error(), Var_Name_Too_Long);
    // a good create clears the error
    v = MT2D_Object_Create_Var_Int("ok", 1);
    CHECK_EQ(MT2D_Var_Last_Error, Var_No_Error);
    MT2D_VAR_Free(v, 1);
}

TEST(null_name_is_rejected) {
    CHECK(MT2D_Object_Create_Var_Int(NULL, 1) == 0);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Null_Name);
    MT2D_Var_Last_Error = Var_No_Error;
}

TEST(string_var_keeps_long_text) {
    const char *path = "assets/Sounds/a_really_long_sound_file_name.ogg";
    MT2D_VAR *v = MT2D_Object_Create_Var_String("Boom", path);
    CHECK(v != 0);
    CHECK_EQ(v->Type, VAR_STRING);
    CHECK_STR_EQ(v->Name, "Boom");
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(v), path);
    CHECK(MT2D_Object_Get_Var_String(v) != path); // copied
    CHECK(MT2D_Object_Get_Var_Data(v) == (void *)MT2D_Object_Get_Var_String(v));
    CHECK_STR_EQ(MT2D_Object_VAR_GetPath(v), path);
    MT2D_VAR_Free(v, 1);
}

TEST(get_path_falls_back_to_the_name) {
    MT2D_VAR *p = MT2D_Object_Create_Var_Pointer("hit.wav");
    CHECK_STR_EQ(MT2D_Object_VAR_GetPath(p), "hit.wav");
    MT2D_VAR_Free(p, 1);
}

TEST(string_var_set_replaces_the_text) {
    MT2D_VAR *v = MT2D_Object_Create_Var_String("S", "first text");
    MT2D_Object_Set_Var_String(v, "second");
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(v), "second");
    // the new text can come from the old one
    MT2D_Object_Set_Var_String(v, MT2D_Object_Get_Var_String(v) + 3);
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(v), "ond");
    MT2D_Object_Set_Var(v, (void *)"via generic", VAR_STRING);
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(v), "via generic");
    CHECK_EQ(MT2D_Var_Last_Error, Var_No_Error);
    MT2D_VAR_Free(v, 1);
}

TEST(string_var_errors) {
    MT2D_VAR *i = MT2D_Object_Create_Var_Int("I", 1);
    MT2D_VAR *s = MT2D_Object_Create_Var_String("S", "t");
    CHECK(MT2D_Object_Get_Var_String(i) == NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Wrong_Type);
    MT2D_Object_Set_Var_String(i, "x");
    CHECK_EQ(MT2D_Var_Last_Error, Var_Wrong_Type);
    CHECK(MT2D_Object_Create_Var_String("N", NULL) == NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Null_Data);
    MT2D_Object_Set_Var_String(s, NULL);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Null_Data);
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(s), "t");
    // not a number
    CHECK_EQ(MT2D_Object_VAR_GetInt(s), 0);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Type_Not_Supported);
    CHECK(!MT2D_Object_VAR_GetBool(s));
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_Object_VAR_SetVar(i, s); // no conversion, nothing changes
    CHECK_EQ(i->Data.i, 1);
    MT2D_VAR_Free(i, 1);
    MT2D_VAR_Free(s, 1);
}

TEST(string_var_clone_is_independent) {
    MT2D_VAR *s = MT2D_Object_Create_Var_String("S", "long text for the clone");
    MT2D_VAR *c = MT2D_VAR_CLONE(s);
    CHECK(c != 0 && c->Type == VAR_STRING);
    CHECK_STR_EQ(c->Name, "S");
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(c), "long text for the clone");
    CHECK(c->Data.p != s->Data.p);
    MT2D_Object_Set_Var_String(s, "changed");
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(c), "long text for the clone");
    MT2D_VAR_Free(s, 1);
    MT2D_VAR_Free(c, 1);
}

TEST(generic_create_string_and_release_data) {
    MT2D_VAR *s = MT2D_Object_Create_Var("G", VAR_STRING, (void *)"abc");
    CHECK_STR_EQ(MT2D_Object_Get_Var_String(s), "abc");
    MT2D_VAR copy = *s; // by value, like Object_Add_Variable does
    free(s);            // the shell only, the copy owns the text now
    CHECK_STR_EQ(copy.Name, "G");
    CHECK_STR_EQ((char *)copy.Data.p, "abc");
    MT2D_VAR_ReleaseData(&copy);
    CHECK(copy.Data.p == 0);
    MT2D_VAR_ReleaseData(&copy); // safe to repeat
}

TEST(set_var_generic_wrong_type_is_rejected) {
    MT2D_VAR *v = MT2D_Object_Create_Var_Int("A", 1);
    float f = 2.0f;
    MT2D_Object_Set_Var(v, (void *)&f, VAR_FLOAT);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Wrong_Type);
    CHECK_EQ(v->Data.i, 1);
    MT2D_Var_Last_Error = Var_No_Error;
    MT2D_VAR_Free(v, 1);
}

TEST(set_var_generic_null_var) {
    int i = 0;
    MT2D_Object_Set_Var(NULL, (void *)&i, VAR_INT);
    CHECK_EQ(MT2D_Var_Last_Error, Var_Not_Found);
    MT2D_Var_Last_Error = Var_No_Error;
}

TEST(get_var_pointer_by_name) {
    MT2D_VAR *a = MT2D_Object_Create_Var_Int("A", 1);
    MT2D_VAR *b = MT2D_Object_Create_Var_Int("B", 2);
    MT2D_VAR array[2] = {*a, *b};
    MT2D_VAR *found = MT2D_Object_Get_Var_Pointer(array, (char *)"B");
    CHECK(found == &array[1]);
    CHECK_EQ(found->Data.i, 2);
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
    CHECK_EQ(dst->Data.i, 9);
    MT2D_Object_VAR_SetVar(dst, fl);
    CHECK_EQ(dst->Data.i, 3);
    CHECK_EQ(dst->Type, VAR_INT);
    MT2D_VAR *src = MT2D_Object_Create_Var_Int("S", 100);
    MT2D_Object_VAR_SetVar(dst, src);
    CHECK_EQ(dst->Data.i, 100);
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
    CHECK_EQ(store->Data.i, 15);
    MT2D_Object_SUB(store, five);
    CHECK_EQ(store->Data.i, 10);
    MT2D_Object_ADD(store, c);
    CHECK_EQ(store->Data.i, 12);
    MT2D_Object_SUB(store, c);
    CHECK_EQ(store->Data.i, 10);
    MT2D_VAR_Free(store, 1);
    MT2D_VAR_Free(five, 1);
    MT2D_VAR_Free(c, 1);
}

TEST(add_and_sub_char) {
    MT2D_VAR *store = MT2D_Object_Create_Var_Char("S", 10);
    MT2D_VAR *three = MT2D_Object_Create_Var_Int("T", 3);
    MT2D_Object_ADD(store, three);
    CHECK_EQ(store->Data.c, 13);
    MT2D_Object_SUB(store, three);
    CHECK_EQ(store->Data.c, 10);
    MT2D_VAR_Free(store, 1);
    MT2D_VAR_Free(three, 1);
}

TEST(int_store_fast_paths_match_the_generic_ones) {
    MT2D_VAR *store = MT2D_Object_Create_Var_Int("S", 100);
    MT2D_VAR *neg = MT2D_Object_Create_Var_Char("N", -3);
    MT2D_VAR *big = MT2D_Object_Create_Var_Int("B", -1000);
    float f = 2.5f;
    MT2D_VAR *fl = MT2D_Object_Create_Var("F", VAR_FLOAT, &f);
    MT2D_Object_ADD(store, neg);  CHECK_EQ(store->Data.i, 97);   // int += negative char
    MT2D_Object_SUB(store, neg);  CHECK_EQ(store->Data.i, 100);
    MT2D_Object_ADD(store, big);  CHECK_EQ(store->Data.i, -900); // int += negative int
    MT2D_Object_SUB(store, big);  CHECK_EQ(store->Data.i, 100);
    MT2D_Object_ADD(store, fl);   CHECK_EQ(store->Data.i, 102);  // float source: generic path, truncated
    MT2D_Object_SUB(store, fl);   CHECK_EQ(store->Data.i, 100);
    CHECK_EQ(MT2D_Object_VAR_GetInt(store), 100);
    MT2D_VAR_Free(store, 1); MT2D_VAR_Free(neg, 1); MT2D_VAR_Free(big, 1); MT2D_VAR_Free(fl, 1);
}

TEST(clone_int_and_char) {
    MT2D_VAR *i = MT2D_Object_Create_Var_Int("I", 8);
    MT2D_VAR *ci = MT2D_VAR_CLONE(i);
    CHECK(ci != i);
    CHECK(ci->Name != i->Name);
    CHECK_STR_EQ(ci->Name, "I");
    CHECK_EQ(ci->Data.i, 8);
    i->Data.i = 1;
    CHECK_EQ(ci->Data.i, 8);

    MT2D_VAR *c = MT2D_Object_Create_Var_Char("C", 'q');
    MT2D_VAR *cc = MT2D_VAR_CLONE(c);
    CHECK_EQ(cc->Type, VAR_CHAR);
    CHECK_EQ(cc->Data.c, 'q');
    MT2D_VAR_Free(i, 1);
    MT2D_VAR_Free(ci, 1);
    MT2D_VAR_Free(c, 1);
    MT2D_VAR_Free(cc, 1);
}

TEST(clone_float_and_float_arithmetic) {
    MT2D_VAR *f = MT2D_Object_Create_Var_Float("F", 1.5f);
    MT2D_VAR *two = MT2D_Object_Create_Var_Int("T", 2);
    MT2D_VAR *c = MT2D_VAR_CLONE(f);
    CHECK(c != 0 && c->Type == VAR_FLOAT && c->Data.f == 1.5f);
    MT2D_Object_ADD(f, two);
    CHECK(f->Data.f == 3.5f);
    MT2D_Object_SUB(f, two);
    CHECK(f->Data.f == 1.5f);
    CHECK(c->Data.f == 1.5f);
    MT2D_VAR_Free(f, 1); MT2D_VAR_Free(two, 1); MT2D_VAR_Free(c, 1);
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

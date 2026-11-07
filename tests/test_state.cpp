// Tests for MT2D/ObjectCore/State.cpp
#include <mt2d_test.h>
#include <MT2D/ObjectCore/Object.h> // must come before State.h (circular includes)
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <stdlib.h>

static Sprite *new_sprite() { return Sprite_Create_Ascii_From_Cells(1, 1, "x"); }

static void dummy_fn(Object *) {}

TEST(create_state_is_empty) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Idle");
    CHECK(st != 0);
    CHECK_STR_EQ(st->Name, "Idle");
    CHECK_EQ(st->Count, 0);
    CHECK(st->Sprites == 0);
    CHECK(st->Functions == 0);
    CHECK(st->WaitSprites == 0);
    free(st);
}

TEST(add_state_first_and_next_frames) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Walk");
    Sprite *s1 = new_sprite();
    Sprite *s2 = new_sprite();
    Cscript *f = Cscript_New2(dummy_fn);

    MT2D_OBJECT_ADD_STATE(st, s1, 3, f);
    CHECK_EQ(st->Count, 1);
    MT2D_OBJECT_ADD_STATE(st, s2, 5, 0);
    CHECK_EQ(st->Count, 2);

    CHECK(st->Sprites[0] == s1);
    CHECK(st->Sprites[1] == s2);
    CHECK_EQ(st->WaitSprites[0], 3);
    CHECK_EQ(st->WaitSprites[1], 5);
    CHECK(st->Functions[0] == f);
    CHECK(st->Functions[1] == 0);
    CHECK_EQ(s1->refCount, 1);
    CHECK_EQ(s2->refCount, 1);
}

TEST(add_state_ignores_null_state) {
    MT2D_OBJECT_ADD_STATE(0, 0, 1, 0); // must not crash
    CHECK(true);
}

TEST(state_add_appends_in_order) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Run");
    Sprite *s[3] = {new_sprite(), new_sprite(), new_sprite()};
    for (int i = 0; i < 3; i++) {
        MT2D_ObjectState_ADD(st, s[i], i + 1, 0);
    }
    CHECK_EQ(st->Count, 3);
    for (int i = 0; i < 3; i++) {
        CHECK(st->Sprites[i] == s[i]);
        CHECK_EQ(st->WaitSprites[i], i + 1);
        CHECK_EQ(s[i]->refCount, 1);
    }
}

TEST(state_add_accepts_null_sprite) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)"Hidden");
    MT2D_ObjectState_ADD(st, 0, 2, 0);
    CHECK_EQ(st->Count, 1);
    CHECK(st->Sprites[0] == 0);
}

TEST(shared_sprite_refcount_grows_and_survives_state_delete) {
    Sprite *shared = new_sprite();
    ObjectState *a = MT2D_OBJECT_CREATE_STATE((char *)"A");
    ObjectState *b = MT2D_OBJECT_CREATE_STATE((char *)"B");
    MT2D_ObjectState_ADD(a, shared, 1, 0);
    MT2D_ObjectState_ADD(b, shared, 1, 0);
    CHECK_EQ(shared->refCount, 2);
    // Deleting a state releases one reference, the sprite is still alive for the other state.
    MT2D_ObjectState_Delete(a);
    CHECK_EQ(shared->refCount, 1);
    CHECK_EQ(Sprite_Get_Char(shared, 0, 0), 'x');
    MT2D_ObjectState_Delete(b);
}

MT2D_TEST_MAIN()

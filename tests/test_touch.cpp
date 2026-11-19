// Tests for MT2D/Touch/MT2D_Touch.c (touch controls made of objects, they write into the joystick state)
#include <mt2d_test.h>
#include <MT2D/MT2D.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/Touch/MT2D_Touch.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Vars.h>
#include <stdlib.h>
#include <string.h>

extern "C" MT2D_Joystick *GlobalJoystickHandler;

static MT2D_Joystick joy;

// what MT2D_Joystick_Update does at the start of a frame
static void frame() {
    for (int i = 0; i < 4; i++) joy.Button_Pressed[i] = false;
    joy.Touched = false;
    joy.Left_Modified = false;
    MT2D_Touch_Update();
}

static void reset() {
    memset(&joy, 0, sizeof(joy));
    GlobalJoystickHandler = &joy;
    MT2D_Touch_Init();
    memset(WINDOW1, ' ', sizeof(WINDOW1));
}

// the center of a cell, in 0..MT2D_TOUCH_RANGE
static int cx(int x) { return (int)(((long)(2 * x + 1) * MT2D_TOUCH_RANGE) / (2 * MAX_HOR)); }
static int cy(int y) { return (int)(((long)(2 * y + 1) * MT2D_TOUCH_RANGE) / (2 * MAX_VER)); }

static Sprite *box(int w, int h, char c) {
    char *cells = (char *)malloc((size_t)(w * h));
    memset(cells, c, (size_t)(w * h));
    Sprite *s = Sprite_Create_Ascii_From_Cells(w, h, cells);
    free(cells);
    return s;
}

static ObjectState *state(const char *name, Sprite *s, int frames = 1, int wait = 1) {
    ObjectState *st = MT2D_OBJECT_CREATE_STATE((char *)name);
    for (int i = 0; i < frames; i++) MT2D_ObjectState_ADD(st, s, wait, 0);
    return st;
}

// "up" (state 0) and "down" (state 1), size 0 = the sprite size
static Object *control(int x, int y, int w, int h, ObjectState *extra = 0) {
    ObjectState **states = (ObjectState **)malloc(3 * sizeof(ObjectState *));
    states[0] = state("up", box(w, h, 'u'));
    states[1] = state("down", box(w, h, 'd'));
    states[2] = extra;
    return Object_Create(false, false, 0, 0, x, y, states, extra ? 3 : 2);
}

static void var_str(Object *o, const char *name, const char *text) {
    Object_Add_Variable(o, MT2D_Object_Create_Var_String(name, text));
}

static void var_int(Object *o, const char *name, int v) {
    Object_Add_Variable(o, MT2D_Object_Create_Var_Int(name, v));
}

TEST(button_defaults_to_joystick_button_1) {
    reset();
    Object *b = control(70, 20, 4, 3);
    CHECK_EQ(MT2D_Touch_Bind(b), 0);
    CHECK_EQ(b->ActualState, 0);

    MT2D_Touch_Finger_Down(1, cx(71), cy(21));
    frame();
    CHECK(joy.Button_Pressed[0]);
    CHECK(joy.Touched);
    CHECK(MT2D_Touch_Pressed(b));
    CHECK_EQ(b->ActualState, 1);

    frame(); // still held
    CHECK(joy.Button_Pressed[0]);

    MT2D_Touch_Finger_Up(1, cx(71), cy(21));
    frame(); // the release is seen on the next update
    CHECK(joy.Button_Pressed[0]);
    frame();
    CHECK(!joy.Button_Pressed[0]);
    CHECK(!MT2D_Touch_Pressed(b));
    CHECK_EQ(b->ActualState, 0);
}

TEST(button_id_and_state_names) {
    reset();
    Object *b = control(0, 0, 2, 2);
    var_int(b, "Btn_Id", 3);
    var_str(b, "Select", "up");    // reversed on purpose
    var_str(b, "Deselect", "down");
    CHECK(MT2D_Touch_Bind(b) >= 0);
    CHECK_EQ(b->ActualState, 1);
    MT2D_Touch_Finger_Down(7, cx(1), cy(1));
    frame();
    CHECK(!joy.Button_Pressed[0]);
    CHECK(joy.Button_Pressed[2]);
    CHECK_EQ(b->ActualState, 0);
}

TEST(quick_tap_between_updates_is_seen_once) {
    reset();
    Object *b = control(10, 10, 2, 2);
    MT2D_Touch_Bind(b);
    MT2D_Touch_Finger_Down(1, cx(10), cy(10));
    MT2D_Touch_Finger_Up(1, cx(10), cy(10));
    frame();
    CHECK(joy.Button_Pressed[0]);
    frame();
    CHECK(!joy.Button_Pressed[0]);
}

TEST(finger_outside_and_sliding_between_buttons) {
    reset();
    Object *a = control(10, 10, 2, 2);
    Object *b = control(12, 10, 2, 2);
    var_int(b, "Btn_Id", 2);
    MT2D_Touch_Bind(a);
    MT2D_Touch_Bind(b);

    MT2D_Touch_Finger_Down(1, cx(30), cy(10));
    frame();
    CHECK(!joy.Touched);

    MT2D_Touch_Finger_Move(1, cx(11), cy(11));
    frame();
    CHECK(joy.Button_Pressed[0]);
    CHECK(!joy.Button_Pressed[1]);

    MT2D_Touch_Finger_Move(1, cx(12), cy(11));
    frame();
    CHECK(!joy.Button_Pressed[0]);
    CHECK(joy.Button_Pressed[1]);
    CHECK_EQ(a->ActualState, 0);
    CHECK_EQ(b->ActualState, 1);

    MT2D_Touch_Finger_Down(2, cx(10), cy(10)); // multitouch
    frame();
    CHECK(joy.Button_Pressed[0]);
    CHECK(joy.Button_Pressed[1]);
}

TEST(hidden_until_the_first_touch) {
    reset();
    Object *b = control(0, 0, 3, 1);
    MT2D_Touch_Bind(b);
    CHECK(!MT2D_Touch_Visible());
    MT2D_Touch_Render();
    CHECK_EQ(WINDOW1[0][0], ' ');

    MT2D_Touch_Finger_Down(1, cx(40), cy(20));
    CHECK(MT2D_Touch_Visible());
    MT2D_Touch_Render();
    CHECK_EQ(WINDOW1[0][0], 'u');
    CHECK_EQ(WINDOW1[0][2], 'u');
}

TEST(show_and_hide) {
    reset();
    Object *b = control(0, 0, 2, 2);
    MT2D_Touch_Bind(b);
    MT2D_Touch_Show(MT2D_TOUCH_SHOW);
    CHECK(MT2D_Touch_Visible());

    MT2D_Touch_Finger_Down(1, cx(0), cy(0));
    frame();
    CHECK(MT2D_Touch_Pressed(b));

    MT2D_Touch_Show(MT2D_TOUCH_HIDE); // releases and forgets the fingers
    CHECK(!MT2D_Touch_Visible());
    CHECK(!MT2D_Touch_Pressed(b));
    CHECK_EQ(b->ActualState, 0);
    MT2D_Touch_Finger_Down(2, cx(0), cy(0));
    frame();
    CHECK(!joy.Touched);

    MT2D_Touch_Show(MT2D_TOUCH_SHOW);
    frame();
    CHECK(!joy.Touched);
}

TEST(fixed_axis_is_the_offset_from_the_center) {
    reset();
    Object *pad = control(0, 15, 10, 10);
    var_str(pad, "Btn_Type", "Axis");
    CHECK(MT2D_Touch_Bind(pad) >= 0);

    MT2D_Touch_Finger_Down(1, cx(9), cy(19));
    frame();
    CHECK(joy.Left_Modified);
    CHECK(joy.Touched);
    CHECK(joy.Left_X_axis > 28000);
    CHECK(joy.Left_Y_axis < 0 && joy.Left_Y_axis > -4000); // half a cell above the center of 10 rows
    CHECK_EQ(pad->ActualState, 1);
    CHECK_EQ(ObjectPosX(pad), 0); // fixed: does not move

    MT2D_Touch_Finger_Move(1, cx(4), cy(15));
    frame();
    CHECK(joy.Left_X_axis < 0 && joy.Left_X_axis > -5000);
    CHECK(joy.Left_Y_axis < -28000);

    MT2D_Touch_Finger_Up(1, cx(4), cy(15));
    frame();
    frame();
    CHECK_EQ(joy.Left_X_axis, 0);
    CHECK_EQ(joy.Left_Y_axis, 0);
    CHECK_EQ(pad->ActualState, 0);
}

TEST(axis_keeps_its_finger_outside_of_the_area) {
    reset();
    Object *pad = control(0, 15, 10, 10);
    var_str(pad, "Btn_Type", "axis"); // not case sensitive
    Object *btn = control(40, 15, 10, 10);
    MT2D_Touch_Bind(pad);
    MT2D_Touch_Bind(btn);

    MT2D_Touch_Finger_Down(1, cx(5), cy(20));
    frame();
    MT2D_Touch_Finger_Move(1, cx(45), cy(20)); // over the button: the stick keeps it
    frame();
    CHECK(!joy.Button_Pressed[0]);
    CHECK_EQ(joy.Left_X_axis, 32767);
}

TEST(right_stick) {
    reset();
    Object *pad = control(70, 15, 10, 10);
    var_str(pad, "Btn_Type", "Axis");
    var_str(pad, "Btn_Id", "Right");
    CHECK(MT2D_Touch_Bind(pad) >= 0);
    MT2D_Touch_Finger_Down(1, cx(70), cy(20));
    frame();
    CHECK(joy.Right_Modified);
    CHECK(joy.Right_X_axis < -28000);
    CHECK_EQ(joy.Left_X_axis, 0);

    MT2D_Touch_Finger_Move(1, cx(75), cy(24)); // bottom center
    frame();
    CHECK(MT2D_Joystick_Get_RightAxisY() > 28000); // used to return the X axis
    CHECK(MT2D_Joystick_Get_RightAxisX() > -1000 && MT2D_Joystick_Get_RightAxisX() < 4000);
}

TEST(count_bound_controls) {
    reset();
    CHECK_EQ(MT2D_Touch_Count(), 0);
    Object *a = control(0, 0, 1, 1);
    MT2D_Touch_Bind(a);
    MT2D_Touch_Bind(control(5, 0, 1, 1));
    CHECK_EQ(MT2D_Touch_Count(), 2);
    MT2D_Touch_Unbind(a);
    CHECK_EQ(MT2D_Touch_Count(), 1);
    MT2D_Touch_Close();
    CHECK_EQ(MT2D_Touch_Count(), 0);
}

TEST(release_keeps_a_value_written_by_a_key) {
    reset();
    Object *pad = control(0, 15, 10, 10);
    var_str(pad, "Btn_Type", "Axis");
    MT2D_Touch_Bind(pad);
    MT2D_Touch_Finger_Down(1, cx(9), cy(20));
    frame();
    MT2D_Touch_Finger_Up(1, cx(9), cy(20));
    frame();
    joy.Left_X_axis = -32768; // an arrow key during the next joystick update
    frame();
    CHECK_EQ(joy.Left_X_axis, -32768);
}

TEST(reactive_thumb_moves_inside_the_area) {
    reset();
    // the area is a 10x1 line (a slider), the thumb 2x1
    ObjectState *area = state("area", box(10, 1, '-'), 1, 0);
    ObjectState **states = (ObjectState **)malloc(3 * sizeof(ObjectState *));
    states[0] = state("up", box(2, 1, 'o'));
    states[1] = state("down", box(2, 1, 'O'));
    states[2] = area;
    Object *s = Object_Create(false, false, 0, 0, 10, 20, states, 3);
    var_str(s, "Btn_Type", "Axis");
    var_str(s, "Ax_Style", "Reactive");
    var_str(s, "Ax_Area", "area");
    CHECK(MT2D_Touch_Bind(s) >= 0);
    CHECK_EQ(ObjectPosX(s), 14); // centered
    CHECK_EQ(ObjectPosY(s), 20);

    MT2D_Touch_Finger_Down(1, cx(19), cy(20));
    frame();
    CHECK_EQ(ObjectPosX(s), 18);
    CHECK_EQ(joy.Left_X_axis, 32767);
    CHECK_EQ(joy.Left_Y_axis, 0); // same height as the thumb: Y does not move

    MT2D_Touch_Finger_Move(1, cx(0), cy(0)); // far outside: clamped
    frame();
    CHECK_EQ(ObjectPosX(s), 10);
    CHECK_EQ(ObjectPosY(s), 20);
    CHECK_EQ(joy.Left_X_axis, -32767);

    MT2D_Touch_Show(MT2D_TOUCH_SHOW);
    MT2D_Touch_Render();
    CHECK_EQ(WINDOW1[20][10], 'O');
    CHECK_EQ(WINDOW1[20][12], '-');

    MT2D_Touch_Finger_Up(1, cx(0), cy(0));
    frame();
    frame();
    CHECK_EQ(ObjectPosX(s), 14);
    CHECK_EQ(joy.Left_X_axis, 0);

    MT2D_Touch_Unbind(s);
    CHECK_EQ(ObjectPosX(s), 10); // back where it was bound
}

TEST(int_vars_like_object_files) {
    reset();
    ObjectState **states = (ObjectState **)malloc(3 * sizeof(ObjectState *));
    states[0] = state("up", box(2, 2, 'o'));
    states[1] = state("down", box(2, 2, 'O'));
    states[2] = state("area", box(6, 6, '.'), 1, 0);
    Object *s = Object_Create(false, false, 0, 0, 0, 0, states, 3);
    var_int(s, "Btn_Type", 1);
    var_int(s, "Btn_Id", 2);
    var_int(s, "Ax_Style", 1);
    var_int(s, "Ax_Area", 2);
    CHECK(MT2D_Touch_Bind(s) >= 0);
    CHECK_EQ(ObjectPosX(s), 2);
    CHECK_EQ(ObjectPosY(s), 2);
    MT2D_Touch_Finger_Down(1, cx(5), cy(5));
    frame();
    CHECK_EQ(joy.Right_X_axis, 32767);
    CHECK_EQ(joy.Right_Y_axis, 32767);
}

TEST(bind_rejects_invalid_controls) {
    reset();
    Object *b = control(0, 0, 1, 1);
    var_int(b, "Btn_Id", 5);
    CHECK_EQ(MT2D_Touch_Bind(b), -1);

    b = control(0, 0, 1, 1);
    var_str(b, "Btn_Type", "Wheel");
    CHECK_EQ(MT2D_Touch_Bind(b), -1);

    b = control(0, 0, 1, 1);
    var_str(b, "Select", "missing");
    CHECK_EQ(MT2D_Touch_Bind(b), -1);

    b = control(0, 0, 1, 1);
    var_str(b, "Btn_Type", "Axis");
    var_str(b, "Ax_Style", "Reactive"); // no Ax_Area
    CHECK_EQ(MT2D_Touch_Bind(b), -1);

    // a state where every frame waits 0 would hang Object_Goto_NextStep
    ObjectState **states = (ObjectState **)malloc(sizeof(ObjectState *));
    states[0] = state("busy", box(1, 1, 'x'), 2, 0);
    CHECK_EQ(MT2D_Touch_Bind(Object_Create(false, false, 0, 0, 0, 0, states, 1)), -1);

    CHECK_EQ(MT2D_Touch_Bind(0), -1);
}

TEST(bind_twice_uses_one_slot_and_slots_run_out) {
    reset();
    Object *b = control(0, 0, 1, 1);
    CHECK_EQ(MT2D_Touch_Bind(b), 0);
    CHECK_EQ(MT2D_Touch_Bind(b), 0);
    for (int i = 1; i < MT2D_TOUCH_MAX_CONTROLS; i++) CHECK_EQ(MT2D_Touch_Bind(control(0, 0, 1, 1)), i);
    CHECK_EQ(MT2D_Touch_Bind(control(0, 0, 1, 1)), -1);
    MT2D_Touch_Unbind(b);
    CHECK_EQ(MT2D_Touch_Bind(control(0, 0, 1, 1)), 0);
}

TEST(render_steps_the_states) {
    reset();
    ObjectState **states = (ObjectState **)malloc(2 * sizeof(ObjectState *));
    states[0] = state("up", box(1, 1, 'u'));
    ObjectState *down = MT2D_OBJECT_CREATE_STATE((char *)"down");
    MT2D_ObjectState_ADD(down, box(1, 1, '1'), 1, 0);
    MT2D_ObjectState_ADD(down, box(1, 1, '2'), 1, 0);
    states[1] = down;
    Object *b = Object_Create(false, false, 0, 0, 5, 5, states, 2);
    MT2D_Touch_Bind(b);
    MT2D_Touch_Finger_Down(1, cx(5), cy(5));
    frame();
    MT2D_Touch_Render();
    CHECK_EQ(WINDOW1[5][5], '1'); // the first frame of the new state is shown
    MT2D_Touch_Render();
    MT2D_Touch_Render();
    CHECK_EQ(WINDOW1[5][5], '2');
    MT2D_Touch_Render();
    MT2D_Touch_Render();
    CHECK_EQ(WINDOW1[5][5], '1'); // the state loops
}

TEST(no_joystick_does_not_crash) {
    reset();
    GlobalJoystickHandler = 0;
    Object *b = control(0, 0, 2, 2);
    Object *pad = control(10, 10, 4, 4);
    var_str(pad, "Btn_Type", "Axis");
    MT2D_Touch_Bind(b);
    MT2D_Touch_Bind(pad);
    MT2D_Touch_Finger_Down(1, cx(0), cy(0));
    MT2D_Touch_Finger_Down(2, cx(11), cy(11));
    MT2D_Touch_Update();
    CHECK(MT2D_Touch_Pressed(b));
    CHECK(MT2D_Touch_Pressed(pad));
}

MT2D_TEST_MAIN()

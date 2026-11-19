/*
        MT2D_Touch.c: touch screen controls made of ObjectCore objects, see MT2D_Touch.h.

        The backend only records the fingers (MT2D_Touch_Finger_*), everything else happens in
        MT2D_Touch_Update, called by the game after MT2D_Joystick_Update: hit tests, states, thumb position
        and the joystick values. No heap is used, the controls and the fingers live in fixed tables.
*/
#define MT2D_TOUCH_IMPLEMENTATION
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D.h>
#include <MT2D/Touch/MT2D_Touch.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Vars.h>
#include <stddef.h>
#include <string.h>

extern MT2D_Joystick *GlobalJoystickHandler;

#define TOUCH_BUTTON 0
#define TOUCH_AXIS 1

#define TOUCH_FIXED 0
#define TOUCH_REACTIVE 1

#define TOUCH_AXIS_MAX 32767L

typedef struct {
        long Id;
        int X, Y;     /* 0..MT2D_TOUCH_RANGE */
        char Used;
        char Up;      /* released, removed at the end of the next update (a quick tap is still seen once) */
        char Taken;   /* held by a stick during this update */
} MT2D_Touch_Finger;

typedef struct {
        Object *Obj;
        char Type;
        char Id;        /* button 1..4, stick 1 (left) or 2 (right) */
        char Style;
        char Pressed;
        char HasFinger; /* sticks keep the finger that grabbed them */
        long Finger;
        int Select, Deselect;
        int UnitX, UnitY; /* MAX_HOR / MAX_VER for ASCII sprites (cells), 320 / 240 for image sprites */
        /* Reactive: the area, in object units, fixed at bind time */
        Sprite *Area;
        int AreaX, AreaY, AreaW, AreaH;
        int ThumbW, ThumbH;
        /* last values written into the joystick, so a release only clears what the touch wrote */
        int LastX, LastY;
} MT2D_Touch_Control;

static MT2D_Touch_Control Controls[MT2D_TOUCH_MAX_CONTROLS];
static MT2D_Touch_Finger Fingers[MT2D_TOUCH_MAX_FINGERS];
static int ShowMode = MT2D_TOUCH_AUTO;
static bool SeenTouch = false;

/* ---- helpers ---- */

static int Touch_Lower(int c) {
        return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

static bool Touch_Same_Text(const char *a, const char *b) {
        while (*a && *b) {
                if (Touch_Lower((unsigned char)*a) != Touch_Lower((unsigned char)*b)) {
                        return false;
                }
                a++;
                b++;
        }
        return *a == *b;
}

static MT2D_VAR *Touch_Find_Var(Object *obj, const char *name) {
        int i;
        for (i = 0; i < obj->User_Vars_Count; i++) {
                if (strcmp(obj->User_Vars[i].Name, name) == 0) {
                        return &obj->User_Vars[i];
                }
        }
        return NULL;
}

static bool Touch_Var_Is_Number(MT2D_VAR *v) {
        return v->Type == VAR_INT || v->Type == VAR_CHAR || v->Type == VAR_FLOAT;
}

/* A var that is a string from "names" (its index + first) or a number. Missing: fallback. Invalid: -1. */
static int Touch_Read_Choice(Object *obj, const char *var, const char *const *names, int count, int first, int fallback) {
        MT2D_VAR *v = Touch_Find_Var(obj, var);
        int i;
        if (!v) {
                return fallback;
        }
        if (Touch_Var_Is_Number(v)) {
                return MT2D_Object_VAR_GetInt(v);
        }
        if (v->Type == VAR_STRING && v->Data.p) {
                for (i = 0; i < count; i++) {
                        if (Touch_Same_Text((const char *)v->Data.p, names[i])) {
                                return i + first;
                        }
                }
        }
        return -1;
}

/* A state given by name (string var) or index (number var). Missing: fallback. Invalid: -1. */
static int Touch_Read_State(Object *obj, const char *var, int fallback) {
        MT2D_VAR *v = Touch_Find_Var(obj, var);
        int i;
        if (!v) {
                return fallback;
        }
        if (Touch_Var_Is_Number(v)) {
                i = MT2D_Object_VAR_GetInt(v);
                return (i >= 0 && i < obj->States_Count) ? i : -1;
        }
        if (v->Type == VAR_STRING && v->Data.p) {
                for (i = 0; i < obj->States_Count; i++) {
                        if (obj->State[i]->Name && strcmp(obj->State[i]->Name, (const char *)v->Data.p) == 0) {
                                return i;
                        }
                }
        }
        return -1;
}

/* Object_Goto_NextStep loops forever on a state where every frame waits 0 */
static bool Touch_State_Can_Loop(ObjectState *st) {
        int i;
        for (i = 0; i < st->Count; i++) {
                if (st->WaitSprites[i] >= 1) {
                        return true;
                }
        }
        return false;
}

static Sprite *Touch_First_Sprite(ObjectState *st) {
        int i;
        for (i = 0; i < st->Count; i++) {
                if (st->Sprites[i]) {
                        return st->Sprites[i];
                }
        }
        return NULL;
}

static void Touch_Sprite_Size(Sprite *s, int *w, int *h) {
        if (!s) {
                *w = 0;
                *h = 0;
        }
        else if (s->type == 0) {
                *w = s->SizeX;
                *h = s->SizeY;
        }
        else {
                *w = s->ScaleX;
                *h = s->ScaleY;
        }
}

static void Touch_Set_Units(MT2D_Touch_Control *c, Sprite *s) {
        if (s && s->type != 0) {
                c->UnitX = 320;
                c->UnitY = 240;
        }
        else {
                c->UnitX = MAX_HOR;
                c->UnitY = MAX_VER;
        }
}

/* object units -> 0..MT2D_TOUCH_RANGE */
static long Touch_To_Range(int v, int unit) {
        return ((long)v * MT2D_TOUCH_RANGE) / unit;
}

/* 0..MT2D_TOUCH_RANGE -> object units, rounded */
static int Touch_From_Range(long v, int unit) {
        long r = v * unit;
        return (int)((r + (r >= 0 ? MT2D_TOUCH_RANGE / 2 : -(MT2D_TOUCH_RANGE / 2))) / MT2D_TOUCH_RANGE);
}

/* The object's size, or the size of its Deselect sprite when Size_X / Size_Y is 0 */
static void Touch_Object_Size(MT2D_Touch_Control *c, int *w, int *h) {
        int sw, sh;
        *w = ObjectSizeX(c->Obj);
        *h = ObjectSizeY(c->Obj);
        if (*w <= 0 || *h <= 0) {
                Touch_Sprite_Size(Touch_First_Sprite(c->Obj->State[c->Deselect]), &sw, &sh);
                if (*w <= 0) *w = sw;
                if (*h <= 0) *h = sh;
        }
}

static void Touch_Set_State(Object *o, int st) {
        if (st < 0 || st >= o->States_Count || o->ActualState == st) {
                return;
        }
        /* not Cscript_Object_SetState: it leaves ActualFrame at -1 for the frame function that called it */
        o->ActualState = st;
        o->ActualFrame = 0;
        o->ActualFrameWait = o->State[st]->WaitSprites[0];
}

static void Touch_Center_Thumb(MT2D_Touch_Control *c) {
        ObjectPosX(c->Obj) = c->AreaX + (c->AreaW - c->ThumbW) / 2;
        ObjectPosY(c->Obj) = c->AreaY + (c->AreaH - c->ThumbH) / 2;
}

static void Touch_Write_Axis(MT2D_Touch_Control *c, int x, int y) {
        MT2D_Joystick *j = GlobalJoystickHandler;
        c->LastX = x;
        c->LastY = y;
        if (!j) {
                return;
        }
        if (c->Id == 2) {
                j->Right_X_axis = x;
                j->Right_Y_axis = y;
                j->Right_Modified = true;
        }
        else {
                j->Left_X_axis = x;
                j->Left_Y_axis = y;
                j->Left_Modified = true;
        }
        j->Touched = true;
}

/* back to 0, unless something else (a key, a controller) wrote the stick after the touch did */
static void Touch_Clear_Axis(MT2D_Touch_Control *c) {
        MT2D_Joystick *j = GlobalJoystickHandler;
        if (j) {
                if (c->Id == 2) {
                        if (j->Right_X_axis == c->LastX && j->Right_Y_axis == c->LastY) {
                                j->Right_X_axis = 0;
                                j->Right_Y_axis = 0;
                        }
                }
                else if (j->Left_X_axis == c->LastX && j->Left_Y_axis == c->LastY) {
                        j->Left_X_axis = 0;
                        j->Left_Y_axis = 0;
                }
        }
        c->LastX = 0;
        c->LastY = 0;
}

static void Touch_Release(MT2D_Touch_Control *c) {
        if (c->Pressed) {
                c->Pressed = 0;
                Touch_Set_State(c->Obj, c->Deselect);
                if (c->Type == TOUCH_AXIS) {
                        Touch_Clear_Axis(c);
                        if (c->Style == TOUCH_REACTIVE) {
                                Touch_Center_Thumb(c);
                        }
                }
        }
        c->HasFinger = 0;
}

static MT2D_Touch_Finger *Touch_Get_Finger(long id) {
        int i;
        for (i = 0; i < MT2D_TOUCH_MAX_FINGERS; i++) {
                if (Fingers[i].Used && Fingers[i].Id == id) {
                        return &Fingers[i];
                }
        }
        return NULL;
}

static bool Touch_Inside(const MT2D_Touch_Finger *f, long x, long y, long w, long h) {
        return f->X >= x && f->X < x + w && f->Y >= y && f->Y < y + h;
}

/* the touch area in 0..MT2D_TOUCH_RANGE */
static void Touch_Area(MT2D_Touch_Control *c, long *x, long *y, long *w, long *h) {
        int ow, oh;
        if (c->Type == TOUCH_AXIS && c->Style == TOUCH_REACTIVE) {
                *x = Touch_To_Range(c->AreaX, c->UnitX);
                *y = Touch_To_Range(c->AreaY, c->UnitY);
                *w = Touch_To_Range(c->AreaW, c->UnitX);
                *h = Touch_To_Range(c->AreaH, c->UnitY);
        }
        else {
                Touch_Object_Size(c, &ow, &oh);
                *x = Touch_To_Range(ObjectPosX(c->Obj), c->UnitX);
                *y = Touch_To_Range(ObjectPosY(c->Obj), c->UnitY);
                *w = Touch_To_Range(ow, c->UnitX);
                *h = Touch_To_Range(oh, c->UnitY);
        }
}

/* -32767..32767 from the offset of "pos" to the center of [start, start + size) */
static int Touch_Axis_Value(long pos, long start, long size) {
        long half = size / 2;
        long v;
        if (half <= 0) {
                return 0;
        }
        v = ((pos - (start + half)) * TOUCH_AXIS_MAX) / half;
        if (v > TOUCH_AXIS_MAX) v = TOUCH_AXIS_MAX;
        if (v < -TOUCH_AXIS_MAX) v = -TOUCH_AXIS_MAX;
        return (int)v;
}

/* Reactive: the thumb follows the finger, clamped inside the area, and gives the axis value */
static int Touch_Thumb_Axis(long finger, long area, long areaSize, long thumbSize, int unit, int *pos) {
        long travel = areaSize - thumbSize; /* where the left / top of the thumb can go */
        long left;
        if (travel <= 0) {
                *pos = Touch_From_Range(area + travel / 2, unit);
                return 0;
        }
        left = finger - thumbSize / 2;
        if (left < area) left = area;
        if (left > area + travel) left = area + travel;
        *pos = Touch_From_Range(left, unit);
        return Touch_Axis_Value(left, area, travel);
}

static void Touch_Update_Axis(MT2D_Touch_Control *c, MT2D_Touch_Finger *f) {
        long ax, ay, aw, ah;
        int x, y, px, py;
        Touch_Area(c, &ax, &ay, &aw, &ah);
        if (c->Style == TOUCH_REACTIVE) {
                x = Touch_Thumb_Axis(f->X, ax, aw, Touch_To_Range(c->ThumbW, c->UnitX), c->UnitX, &px);
                y = Touch_Thumb_Axis(f->Y, ay, ah, Touch_To_Range(c->ThumbH, c->UnitY), c->UnitY, &py);
                ObjectPosX(c->Obj) = px;
                ObjectPosY(c->Obj) = py;
        }
        else {
                x = Touch_Axis_Value(f->X, ax, aw);
                y = Touch_Axis_Value(f->Y, ay, ah);
        }
        Touch_Write_Axis(c, x, y);
}

/* ---- public API ---- */

void MT2D_Touch_Init(void) {
        memset(Controls, 0, sizeof(Controls));
        memset(Fingers, 0, sizeof(Fingers));
        ShowMode = MT2D_TOUCH_AUTO;
        SeenTouch = false;
}

void MT2D_Touch_Close(void) {
        int i;
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                if (Controls[i].Obj) {
                        MT2D_Touch_Unbind(Controls[i].Obj);
                }
        }
        MT2D_Touch_Init();
}

int MT2D_Touch_Bind(Object *obj) {
        static const char *const types[] = { "Button", "Axis" };
        static const char *const sticks[] = { "Left", "Right" };
        static const char *const styles[] = { "Fixed", "Reactive" };
        MT2D_Touch_Control c;
        int slot = -1;
        int i, area;

        if (!obj || obj->States_Count <= 0) {
                return -1;
        }
        MT2D_Touch_Unbind(obj); /* binding again reads the vars again */
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS && slot < 0; i++) {
                if (!Controls[i].Obj) {
                        slot = i;
                }
        }
        if (slot < 0) {
                return -1;
        }

        memset(&c, 0, sizeof(c));
        c.Obj = obj;
        i = Touch_Read_Choice(obj, "Btn_Type", types, 2, 0, TOUCH_BUTTON);
        if (i != TOUCH_BUTTON && i != TOUCH_AXIS) {
                return -1;
        }
        c.Type = (char)i;
        if (c.Type == TOUCH_BUTTON) {
                i = Touch_Read_Choice(obj, "Btn_Id", NULL, 0, 1, 1);
                if (i < 1 || i > 4) {
                        return -1;
                }
                c.Style = TOUCH_FIXED;
        }
        else {
                i = Touch_Read_Choice(obj, "Btn_Id", sticks, 2, 1, 1);
                if (i < 1 || i > 2) {
                        return -1;
                }
                c.Style = (char)Touch_Read_Choice(obj, "Ax_Style", styles, 2, 0, TOUCH_FIXED);
                if (c.Style != TOUCH_FIXED && c.Style != TOUCH_REACTIVE) {
                        return -1;
                }
        }
        c.Id = (char)i;

        c.Deselect = Touch_Read_State(obj, "Deselect", 0);
        c.Select = Touch_Read_State(obj, "Select", obj->States_Count > 1 ? 1 : 0);
        if (c.Deselect < 0 || c.Select < 0 ||
                !Touch_State_Can_Loop(obj->State[c.Deselect]) || !Touch_State_Can_Loop(obj->State[c.Select])) {
                return -1;
        }
        Touch_Set_Units(&c, Touch_First_Sprite(obj->State[c.Deselect]));

        if (c.Style == TOUCH_REACTIVE) {
                area = Touch_Read_State(obj, "Ax_Area", -1);
                if (area < 0) {
                        return -1;
                }
                c.Area = Touch_First_Sprite(obj->State[area]);
                if (!c.Area) {
                        return -1;
                }
                Touch_Set_Units(&c, c.Area);
                Touch_Sprite_Size(c.Area, &c.AreaW, &c.AreaH);
                c.AreaX = ObjectPosX(obj);
                c.AreaY = ObjectPosY(obj);
                Touch_Object_Size(&c, &c.ThumbW, &c.ThumbH);
                Touch_Center_Thumb(&c);
        }

        Controls[slot] = c;
        Touch_Set_State(obj, c.Deselect);
        return slot;
}

void MT2D_Touch_Unbind(Object *obj) {
        int i;
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                if (obj && Controls[i].Obj == obj) {
                        Touch_Release(&Controls[i]);
                        if (Controls[i].Style == TOUCH_REACTIVE) {
                                ObjectPosX(obj) = Controls[i].AreaX; /* where it was when it was bound */
                                ObjectPosY(obj) = Controls[i].AreaY;
                        }
                        memset(&Controls[i], 0, sizeof(Controls[i]));
                }
        }
}

void MT2D_Touch_Show(int mode) {
        int i;
        ShowMode = mode;
        if (mode == MT2D_TOUCH_HIDE) {
                for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                        if (Controls[i].Obj) {
                                Touch_Release(&Controls[i]);
                        }
                }
                memset(Fingers, 0, sizeof(Fingers));
        }
}

bool MT2D_Touch_Visible(void) {
        return ShowMode == MT2D_TOUCH_SHOW || (ShowMode == MT2D_TOUCH_AUTO && SeenTouch);
}

bool MT2D_Touch_Pressed(Object *obj) {
        int i;
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                if (obj && Controls[i].Obj == obj) {
                        return Controls[i].Pressed != 0;
                }
        }
        return false;
}

int MT2D_Touch_Count(void) {
        int i, n = 0;
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                if (Controls[i].Obj) {
                        n++;
                }
        }
        return n;
}

void MT2D_Touch_Update(void) {
        int i, k;
        long ax, ay, aw, ah;
        MT2D_Touch_Control *c;
        MT2D_Touch_Finger *f;
        bool now;

        if (!MT2D_Touch_Visible()) {
                return;
        }
        for (k = 0; k < MT2D_TOUCH_MAX_FINGERS; k++) {
                Fingers[k].Taken = 0;
        }

        /* sticks first: a stick keeps its finger, even outside of its area, until it is lifted */
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                c = &Controls[i];
                if (!c->Obj || c->Type != TOUCH_AXIS) {
                        continue;
                }
                if (!c->Obj->Enabled) {
                        Touch_Release(c);
                        continue;
                }
                f = c->HasFinger ? Touch_Get_Finger(c->Finger) : NULL;
                if (!f) {
                        c->HasFinger = 0;
                        Touch_Area(c, &ax, &ay, &aw, &ah);
                        for (k = 0; k < MT2D_TOUCH_MAX_FINGERS && !f; k++) {
                                if (Fingers[k].Used && !Fingers[k].Taken && Touch_Inside(&Fingers[k], ax, ay, aw, ah)) {
                                        f = &Fingers[k];
                                }
                        }
                }
                if (!f) {
                        Touch_Release(c);
                        continue;
                }
                f->Taken = 1;
                c->HasFinger = 1;
                c->Finger = f->Id;
                if (!c->Pressed) {
                        c->Pressed = 1;
                        Touch_Set_State(c->Obj, c->Select);
                }
                Touch_Update_Axis(c, f);
        }

        /* buttons: pressed while any finger that no stick holds is on them (fingers can slide between buttons) */
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                c = &Controls[i];
                if (!c->Obj || c->Type != TOUCH_BUTTON) {
                        continue;
                }
                now = false;
                if (c->Obj->Enabled) {
                        Touch_Area(c, &ax, &ay, &aw, &ah);
                        for (k = 0; k < MT2D_TOUCH_MAX_FINGERS && !now; k++) {
                                now = Fingers[k].Used && !Fingers[k].Taken && Touch_Inside(&Fingers[k], ax, ay, aw, ah);
                        }
                }
                if (now) {
                        if (!c->Pressed) {
                                c->Pressed = 1;
                                Touch_Set_State(c->Obj, c->Select);
                        }
                        if (GlobalJoystickHandler) {
                                GlobalJoystickHandler->Button_Pressed[c->Id - 1] = true;
                                GlobalJoystickHandler->Touched = true;
                        }
                }
                else {
                        Touch_Release(c);
                }
        }

        /* lifted fingers were seen once, drop them */
        for (k = 0; k < MT2D_TOUCH_MAX_FINGERS; k++) {
                if (Fingers[k].Used && Fingers[k].Up) {
                        Fingers[k].Used = 0;
                }
        }
}

void MT2D_Touch_Render(void) {
        int i;
        MT2D_Touch_Control *c;
        if (!MT2D_Touch_Visible()) {
                return;
        }
        for (i = 0; i < MT2D_TOUCH_MAX_CONTROLS; i++) {
                c = &Controls[i];
                if (!c->Obj || !c->Obj->Enabled) {
                        continue;
                }
                if (c->Area) {
                        Sprite_Render_on_Window(c->Area, DISPLAY_WINDOW1, c->AreaX, c->AreaY);
                }
                Object_Render(c->Obj);
                Object_Goto_NextStep(c->Obj); /* after rendering: the first frame of a new state is shown too */
        }
}

/* ---- input from the backend ---- */

void MT2D_Touch_Finger_Down(long id, int x, int y) {
        MT2D_Touch_Finger *f;
        int k;
        if (ShowMode == MT2D_TOUCH_HIDE) {
                return;
        }
        SeenTouch = true;
        f = Touch_Get_Finger(id);
        for (k = 0; k < MT2D_TOUCH_MAX_FINGERS && !f; k++) {
                if (!Fingers[k].Used) {
                        f = &Fingers[k];
                }
        }
        if (!f) {
                return; /* more fingers than MT2D_TOUCH_MAX_FINGERS */
        }
        f->Used = 1;
        f->Up = 0;
        f->Taken = 0;
        f->Id = id;
        f->X = x;
        f->Y = y;
}

void MT2D_Touch_Finger_Move(long id, int x, int y) {
        MT2D_Touch_Finger *f = Touch_Get_Finger(id);
        if (f && !f->Up) {
                f->X = x;
                f->Y = y;
        }
}

void MT2D_Touch_Finger_Up(long id, int x, int y) {
        MT2D_Touch_Finger *f = Touch_Get_Finger(id);
        if (f) {
                f->X = x;
                f->Y = y;
                f->Up = 1;
        }
}

#include <MT2D/ObjectCore/STDFunctions.h> /* declares the functions with C linkage */
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <string.h>
#include <stdlib.h>

/* Var and state names are equal: the first char is compared before strcmp, so most names are rejected
   without a call (cheap on 16 bit DOS). a and b are evaluated twice. */
#define MT2D_NAME_EQ(a, b) ((a)[0] == (b)[0] && strcmp((a), (b)) == 0)

int Cscript_VAR_Get_Integer(Object *object, MT2D_VAR *Var);

/**
Return the index of a Var inside an Object where is
the first similar Var with the same requested name
by the Pointer.
**/
int Cscript_Get_Object_VarIndex(MT2D_VAR *Var) {
        int out = -1;
        Object *O = (Object*)Var->Data.p;
        int i = 0;
        for (; i < O->User_Vars_Count; i++) {
                if (MT2D_NAME_EQ(O->User_Vars[i].Name, Var->Name)) {
                        out = i;
                        i = O->User_Vars_Count;
                }
        }
        return out;
}

/**
>> USE Cscript_VAR_Get_Integer
Get the value from the internal struct of an object.
use the function Cscript_VAR_Get_Integer in case you are dealing
with other types of data other than object struct data.
**/
int Cscript_Get_Object_Internal_Data(MT2D_VAR *Var) {
        Object *O = (Object*)Var->Data.p;
        int Return = 0;
        if (O) {
                int i = 0;
                for (; i < O->User_Vars_Count; i++) {
                        if (MT2D_NAME_EQ(O->User_Vars[i].Name, Var->Name)) {
                                Return = MT2D_Object_VAR_GetInt(&O->User_Vars[i]);
                                i = O->User_Vars_Count;
                        }
                }
        }
        return Return;
}

/**
>> USE Cscript_VAR_Set_Integer
Get the value from the internal struct of an object.
use the function Cscript_VAR_Get_Integer in case you are dealing
with other types of data other than object struct data.

!! WARNING, NewVar must not be a POINTER!!!
**/
void Cscript_Set_Object_Internal_Data(Object *caller, MT2D_VAR *Var, MT2D_VAR *NewVar) {
        Object *O = (Object*)Var->Data.p;
        if (O) {
                /* check vars */
                int i = 0;
                for (; i < O->User_Vars_Count; i++) {
                        if (MT2D_NAME_EQ(O->User_Vars[i].Name, Var->Name)) {
                                MT2D_Object_VAR_SetVar(&O->User_Vars[i], NewVar);
                                i = O->User_Vars_Count;
                        }
                }
		}
}

/**
gets an integer representation of the data pointed in MT2D_VAR.
It checks what type of var is before taking directly the data.
**/
int Cscript_VAR_Get_Integer(Object *object, MT2D_VAR *Var) {
        int Return = 0;
        if (Var->Type == VAR_POINTER) {
                Var->Data.p = object;
                Return = Cscript_Get_Object_Internal_Data(Var);
                Var->Data.p = 0;
        }
        else {
                Return = MT2D_Object_VAR_GetInt(Var);
        }
        return Return;
}

/**
set an integer representation of the data pointed in MT2D_VAR.
It checks what type of var is before taking directly the data.
**/
void Cscript_VAR_Set_Integer(Object *object, MT2D_VAR *Var, MT2D_VAR *NewData) {
        if (Var->Type == VAR_POINTER) {
                Var->Data.p = object;
                if (NewData->Type == VAR_POINTER) {
                        MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_Get_Object_Internal_Data(NewData));
                        Cscript_Set_Object_Internal_Data(object, Var, Tmp);
                        MT2D_VAR_Free(Tmp,1);
                }
                else {
                        Cscript_Set_Object_Internal_Data(object, Var, NewData);
                }
        }
        else if (NewData->Type == VAR_POINTER) {
                MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_Get_Object_Internal_Data(NewData));
                MT2D_Object_VAR_SetVar(Var, NewData);
                MT2D_VAR_Free(Tmp,1);

        }
        else {
                MT2D_Object_VAR_SetVar(Var, NewData);
        }
}

/**
VARS[0] = State Name
**/
void Cscript_Object_SetState(Object *Object, MT2D_VAR **StateName) {
        int i = 0;
        while (i < Object->States_Count) {
                if (MT2D_NAME_EQ(Object->State[i]->Name, StateName[0]->Name)) {
                        Object->ActualState = i;
                        Object->ActualFrame = -1; /* after this call the function is going to be increased by one so we avoid the code to miss the first state. */
                        Object->ActualFrameWait = Object->State[i]->WaitSprites[0];
                        i = Object->States_Count;
                }
                i++;
        }
}

/**
VARS[0] = Pointer
VARS[1] = Data to transfer
**/
void Cscript_Set_Var(Object *Object, MT2D_VAR **VARS) {
        MT2D_VAR *tmp;
        if (VARS[0]->Type == VAR_POINTER) {
                if (VARS[0]->Data.p == 0) {
                        VARS[0]->Data.p = Object;
                        if (VARS[1]->Type == VAR_POINTER) {
                                VARS[1]->Data.p = Object;
                                Cscript_VAR_Set_Integer(Object, VARS[0], VARS[1]);
                                VARS[1]->Data.p = 0;
                        }
                        else {
                                Cscript_VAR_Set_Integer(Object, VARS[0], VARS[1]);
                        }
                        VARS[0]->Data.p = 0;
                }
                else {
                        Cscript_Set_Object_Internal_Data(Object, VARS[0], VARS[1]);
                }
        }
        else if (VARS[1]->Type == VAR_POINTER) {
                tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(Object, VARS[1]));
                MT2D_Object_VAR_SetVar(VARS[0], tmp);
                MT2D_VAR_Free(tmp,1);
        }
        else {
                MT2D_Object_VAR_SetVar(VARS[0], VARS[1]);
        }
}

/**
object  = caller
VARS[0] = Name of the state to jump
VARS[1] = ACTUAL VALUE TO SELECT (Rand()%VARS[2]  <= VARS[1])
VARS[2] = MAX vALUe to JUMP, <= 0 never jumps
Main Object
**/
void Cscript_JumpToStateIfRandom(Object *object, MT2D_VAR **JUMPTO_CHANCE_MAX) {
        int i = 0;
        int MAX = Cscript_VAR_Get_Integer(object, JUMPTO_CHANCE_MAX[2]);
        int SEL_RANGE = Cscript_VAR_Get_Integer(object, JUMPTO_CHANCE_MAX[1]);
        if (MAX > 0 && rand() % MAX <= SEL_RANGE) {
                while (i < object->States_Count) {
                        if (MT2D_NAME_EQ(object->State[i]->Name, JUMPTO_CHANCE_MAX[0]->Name)) {
                                object->ActualState = i;
                                object->ActualFrame = 0;
                                object->ActualFrameWait = object->State[i]->WaitSprites[0];
                                i = object->States_Count;
                        }
                        i++;
                }
        }
}

/**
object  = caller
VARS[0] = Total of Frames to jump, if 1 it'll jump to the next frame, 2 will skip 1 frame, -1 will go back one frame,...
VARS[1] = ACTUAL VALUE TO SELECT (Rand()%VARS[2]  <= VARS[1])
VARS[2] = MAX vALUe to JUMP, <= 0 never jumps
Main Object
**/
void Cscript_JumpFramesIfRandom(Object* caller, MT2D_VAR** vars) {
        int jump = Cscript_VAR_Get_Integer(caller, vars[0]);
        int max = Cscript_VAR_Get_Integer(caller, vars[2]);
        int selectRange = Cscript_VAR_Get_Integer(caller, vars[1]);
        if (max > 0 && rand() % max <= selectRange) {
                caller->ActualFrame += jump;
/*              caller->ActualFrameWait = caller->State[caller->ActualState]->WaitSprites[caller->ActualFrame -1]; */
        }
}

/*
Vars:
[0] = the name of the state to be jumped
[1] <= [2], jump if true
*/
void Cscript_JumpToStateIfVarLowerEq(Object *object, MT2D_VAR **Vars) {
        int i = 0;
        int Left = Cscript_VAR_Get_Integer(object, Vars[1]);
        int Right = Cscript_VAR_Get_Integer(object, Vars[2]);
        if (Left <= Right) {
                        while (i < object->States_Count) {
                        if (MT2D_NAME_EQ(object->State[i]->Name, Vars[0]->Name)) {
                                object->ActualState = i;
                                object->ActualFrame = -1; /* after this call the function is going to be increased by one so we avoid the code to miss the first state. */
                                object->ActualFrameWait = object->State[i]->WaitSprites[0];
                                i = object->States_Count;
                        }
                        i++;
                }
        }
}

/*
Vars:
[0] = the name of the state to be jumped
[1] >= [2], jump if true
*/
void Cscript_JumpToStateIfVarHigherEq(Object *object, MT2D_VAR **Vars) {
        int i = 0;
        int Left = Cscript_VAR_Get_Integer(object, Vars[1]);
        int Right = Cscript_VAR_Get_Integer(object, Vars[2]);
        if (Left >= Right) {
                while (i < object->States_Count) {
                        if (MT2D_NAME_EQ(object->State[i]->Name, Vars[0]->Name)) {
                                object->ActualState = i;
                                object->ActualFrame = -1; /* after this call the function is going to be increased by one so we avoid the code to miss the first state. */
                                object->ActualFrameWait = object->State[i]->WaitSprites[0];
                                i = object->States_Count;
                        }
                        i++;
                }
        }
}

/*
Vars:
[0] = the name of the state to be jumped
[1] = the max value to jump
[2] = the var where the check number is stored
*/
void Cscript_JumpToStateIfVarEq(Object *object, MT2D_VAR **Vars) {
        int i = 0;
        int MAX = Cscript_VAR_Get_Integer(object, Vars[1]);
        int Now = Cscript_VAR_Get_Integer(object, Vars[2]);
        if (object->ActualState == 2) {
                i = 0;
        }

        if (Now == MAX) {
                if (MT2D_NAME_EQ(Vars[0]->Name, "Moving")) {
                        i = 0;
                }
                while (i < object->States_Count) {
                        if (MT2D_NAME_EQ(object->State[i]->Name, Vars[0]->Name)) {
                                object->ActualState = i;
                                object->ActualFrame = -1; /* after this call the function is going to be increased by one so we avoid the code to miss the first state. */
                                object->ActualFrameWait = object->State[i]->WaitSprites[0];
                                i = object->States_Count;
                        }
                        i++;
                }
        }
}

/*
NOTE: Do not use this function in the last function of a state.
Vars:
[1] = the max value to jump
[2] = the var where the check number is stored
*/
void Cscript_JumpNextFrameIfVarEq(Object *object, MT2D_VAR **Vars) {
        int MAX = Cscript_VAR_Get_Integer(object, Vars[0]);
        int Now = Cscript_VAR_Get_Integer(object, Vars[1]);
        if (Now == MAX) {
                object->ActualFrame++;
                object->ActualFrameWait = object->State[object->ActualState]->WaitSprites[object->ActualFrame-1];

        }
}

/*
NOTE: Do not use this function in the last function of a state.
Vars:
[0] = Left
[1] = Right
*/
void Cscript_JumpNextFrameIfVarLowerEq(Object *object, MT2D_VAR **Vars) {
        int Left = Cscript_VAR_Get_Integer(object, Vars[0]);
        int Right = Cscript_VAR_Get_Integer(object, Vars[1]);
        if (Left <= Right) {
                object->ActualFrame++;
                object->ActualFrameWait = object->State[object->ActualState]->WaitSprites[object->ActualFrame - 1];
        }
}

/*
NOTE: Do not use this function in the last function of a state.
Vars:
[0] = Left
[1] = Right
*/
void Cscript_JumpNextFrameIfVarHigherEq(Object *object, MT2D_VAR **Vars) {
        int Left = Cscript_VAR_Get_Integer(object, Vars[0]);
        int Right = Cscript_VAR_Get_Integer(object, Vars[1]);
        if (Left >= Right) {
                object->ActualFrame++;
                object->ActualFrameWait = object->State[object->ActualState]->WaitSprites[object->ActualFrame - 1];
        }
}

/* Operator codes shared with Cscript_While: '<' '>' '=' '!' (not equal), 242 (>=) and 243 (<=). */
static bool Cscript_Private_Compare(int Left, unsigned char Operation, int Right) {
        switch (Operation) {
        case '>':  return Left > Right;
        case '<':  return Left < Right;
        case '=':  return Left == Right;
        case '!':  return Left != Right;
        case 242:  return Left >= Right; /* >= */
        case 243:  return Left <= Right; /* <= */
        default:   return false;
        }
}

/*
Moves the frame pointer by "jump" frames, forward or backward. The engine goes to the next
frame right after the call, so the object lands on frame (ActualFrame + jump + 1):
jump 0 = next frame (no change), jump 1 = skip one frame, jump -2 = go back to the previous frame.
Vars:
[0] = jump
*/
void Cscript_JumpFrames(Object *object, MT2D_VAR **Vars) {
        object->ActualFrame += Cscript_VAR_Get_Integer(object, Vars[0]);
}

/*
Same as Cscript_JumpFrames, but only when the comparison is true.
Vars:
[0] = jump
[1] = Left
[2] = operator, an unsigned char (see Cscript_While; 242 is >=, 243 is <=)
[3] = Right
*/
void Cscript_JumpFramesIfVar(Object *object, MT2D_VAR **Vars) {
        int Left = Cscript_VAR_Get_Integer(object, Vars[1]);
        unsigned char Operation = (unsigned char)Cscript_VAR_Get_Integer(object, Vars[2]);
        int Right = Cscript_VAR_Get_Integer(object, Vars[3]);
        if (Cscript_Private_Compare(Left, Operation, Right)) {
                object->ActualFrame += Cscript_VAR_Get_Integer(object, Vars[0]);
        }
}

/* Jumps to the state called like Name, does nothing if there is none. */
static void Cscript_Private_GotoStateByName(Object *object, const char *Name) {
        int i;
        for (i = 0; i < object->States_Count; i++) {
                if (MT2D_NAME_EQ(object->State[i]->Name, Name)) {
                        object->ActualState = i;
                        object->ActualFrame = -1; /* after this call the function is going to be increased by one so we avoid the code to miss the first frame. */
                        object->ActualFrameWait = object->State[i]->WaitSprites[0];
                        return;
                }
        }
}

/*
Vars:
[0] = the name of the state to be jumped
[1] < [2], jump if true
*/
void Cscript_JumpToStateIfVarLower(Object *object, MT2D_VAR **Vars) {
        if (Cscript_VAR_Get_Integer(object, Vars[1]) < Cscript_VAR_Get_Integer(object, Vars[2])) {
                Cscript_Private_GotoStateByName(object, Vars[0]->Name);
        }
}

/*
Vars:
[0] = the name of the state to be jumped
[1] > [2], jump if true
*/
void Cscript_JumpToStateIfVarHigher(Object *object, MT2D_VAR **Vars) {
        if (Cscript_VAR_Get_Integer(object, Vars[1]) > Cscript_VAR_Get_Integer(object, Vars[2])) {
                Cscript_Private_GotoStateByName(object, Vars[0]->Name);
        }
}

/*
Vars:
[0] = the name of the state to be jumped
[1] != [2], jump if true
*/
void Cscript_JumpToStateIfVarNotEq(Object *object, MT2D_VAR **Vars) {
        if (Cscript_VAR_Get_Integer(object, Vars[1]) != Cscript_VAR_Get_Integer(object, Vars[2])) {
                Cscript_Private_GotoStateByName(object, Vars[0]->Name);
        }
}

/*
NOTE: Do not use this function in the last function of a state.
Vars:
[0] = Left
[1] = Right
Skips the next frame if Left < Right.
*/
void Cscript_JumpNextFrameIfVarLower(Object *object, MT2D_VAR **Vars) {
        if (Cscript_VAR_Get_Integer(object, Vars[0]) < Cscript_VAR_Get_Integer(object, Vars[1])) {
                object->ActualFrame++;
                object->ActualFrameWait = object->State[object->ActualState]->WaitSprites[object->ActualFrame - 1];
        }
}

/*
NOTE: Do not use this function in the last function of a state.
Skips the next frame if Left > Right.
*/
void Cscript_JumpNextFrameIfVarHigher(Object *object, MT2D_VAR **Vars) {
        if (Cscript_VAR_Get_Integer(object, Vars[0]) > Cscript_VAR_Get_Integer(object, Vars[1])) {
                object->ActualFrame++;
                object->ActualFrameWait = object->State[object->ActualState]->WaitSprites[object->ActualFrame - 1];
        }
}

/*
NOTE: Do not use this function in the last function of a state.
Skips the next frame if Left != Right.
*/
void Cscript_JumpNextFrameIfVarNotEq(Object *object, MT2D_VAR **Vars) {
        if (Cscript_VAR_Get_Integer(object, Vars[0]) != Cscript_VAR_Get_Integer(object, Vars[1])) {
                object->ActualFrame++;
                object->ActualFrameWait = object->State[object->ActualState]->WaitSprites[object->ActualFrame - 1];
        }
}

/**
It'll only mark that this object wants to be deleted, so you can manually do that or let
the Scenecontrol to do that automatically.
**/
void Cscript_RemoveObject(Object *Caller) {
        Caller->CanDelete = true;
}

/**
DESCRIPTION: Add a new MT2D_VAR under the Caller object.
Var:
[0] = The var to be added
**/
void Cscript_AddVar_ToObject(Object *Caller, MT2D_VAR **Var){
        int i =0;
        while(i < Caller->User_Vars_Count){
                if(MT2D_NAME_EQ(Var[i]->Name, Caller->User_Vars[i].Name)){
                        i = Caller->User_Vars_Count;
                }
                i++;            
        }               
        if(i ==  Caller->User_Vars_Count){
                /* Skip duplicated var */
                MT2D_VAR* clone = MT2D_VAR_CLONE(Var[0]);
                Object_Add_Variable(Caller, clone);
                free(clone); /* only the shell, Name and Data now belong to Caller */
        }
}

/*TODO void Cscript_RemoveVar_FromObject(OBject *Caller, MT2D_VAR **Var);*/

/* Var 0 Start index of while, the value and data type does not matter. */
/* Var 1 Left Compare */
/* Var 2 Signal (char), should not be pointer */
/* Var 3 Right */
void Cscript_While(Object* caller, MT2D_VAR** vars) {
        int skipTo = Cscript_VAR_Get_Integer(caller, vars[0]);
        int Left = Cscript_VAR_Get_Integer(caller, vars[1]);
        int Right = Cscript_VAR_Get_Integer(caller, vars[3]);
        unsigned char operation = (unsigned char)Cscript_VAR_Get_Integer(caller, vars[2]);
        bool skip = false;
        switch (operation) {
        case '>':
                skip = Left > Right;
                break;
        case '<':
                skip = Left < Right;
                break;
        case '=':
                skip = Left == Right;
                break;
        case '!':
                skip = Left != Right;
                break;
        case 242: /* >= */
                skip = Left >= Right;
                break;
        case 243: /* <= */
                skip = Left <= Right;
                break;
        default:
                skip = true;
        }
        if (skipTo == 0) {
                MT2D_VAR* tmp = MT2D_Object_Create_Var_Int("", caller->ActualFrame);
                Cscript_VAR_Set_Integer(caller, vars[0], tmp);
                MT2D_VAR_Free(tmp, 1);
        }
        if (!skip) {
                int i = caller->ActualFrame + 1;
                while (!skip)
                {
                        if (caller->State[caller->ActualState]->Functions[i] != 0) {
                                if (caller->State[caller->ActualState]->Functions[i]->TempVars != NULL) {
                                        if (caller->State[caller->ActualState]->Functions[i]->TempVars[0] == vars[0]) {
                                                skip = true;
                                        }
                                }
                        }
                        i++;
                }

                caller->ActualFrame = i - 1;
/*              caller->ActualFrameWait = caller->State[caller->ActualState]->WaitSprites[i]; */
        }
}

/* VAR 0: Reset state to this place */
void Cscript_While_End(Object* caller, MT2D_VAR** vars) {
        int i = Cscript_VAR_Get_Integer(caller, vars[0]) - 1;
        caller->ActualFrame = i;
/*      caller->ActualFrameWait = caller->State[caller->ActualState]->WaitSprites[i]; */
}

void Cscript_TeleportToCoord(Object *object, MT2D_VAR **X_Y) {
        int x = Cscript_VAR_Get_Integer(object, X_Y[0]);
        int y = Cscript_VAR_Get_Integer(object, X_Y[1]);
        object->User_Vars[Object_PosX_Var_Index].Data.i = x;
        object->User_Vars[Object_PosY_Var_Index].Data.i = y;
}

void Cscript_Move(Object *object) {
        object->User_Vars[Object_PosX_Var_Index].Data.i += object->User_Vars[Object_AcelX_Var_Index].Data.i;
        object->User_Vars[Object_PosY_Var_Index].Data.i += object->User_Vars[Object_AcelY_Var_Index].Data.i;
}

/**
Vars[0] = Pos X
Vars[1] = Pos Y
Vars[2] = Acel X
Vars[3] = Acel Y
**/
void Cscript_CreateObject(Object *Caller, Object *NewModel, MT2D_VAR **Vars) {
        Object *NewObject;
        int i;
        int X[2];
        int Y[2];
        X[0] = Cscript_VAR_Get_Integer(Caller,Vars[0]);
        X[1] = Cscript_VAR_Get_Integer(Caller,Vars[2]);
        Y[0] = Cscript_VAR_Get_Integer(Caller,Vars[1]);
        Y[1] = Cscript_VAR_Get_Integer(Caller,Vars[3]);
        NewObject = Object_Create(
                NewModel->Solid, 
                NewModel->RenderOnly, 
                NewModel->User_Vars[Object_SizeX_Var_Index].Data.i,
                NewModel->User_Vars[Object_SizeY_Var_Index].Data.i,
                X[0], 
                Y[0], 
                NewModel->State, 
                NewModel->States_Count
        );
        NewObject->User_Vars[Object_AcelX_Var_Index].Data.i = X[1];
        NewObject->User_Vars[Object_AcelY_Var_Index].Data.i = Y[1];
        if (NewModel->User_Vars_Count > 0)
        {
 /* TODO: OBSOLETE? */
 /*               MT2D_VAR *NewVars = (MT2D_VAR*)malloc(NewModel->User_Vars_Count * sizeof(MT2D_VAR)); */
                for (i = Object_First_User_Var_Index; i < NewModel->User_Vars_Count; i++)
                {
                        MT2D_VAR* clone = MT2D_VAR_CLONE(&NewModel->User_Vars[i]);
                        Object_Add_Variable(NewObject, clone);
                        free(clone); /* only the shell, Name and Data now belong to NewObject */
                }
        }
        ObjectScene_Add(Caller->MyScene, NewObject);
}

bool Object_Hit_Object(Object* Caller, Object* Target)
{
        if (Caller->User_Vars[Object_PosX_Var_Index].Data.i <= Target->User_Vars[Object_PosX_Var_Index].Data.i + Target->User_Vars[Object_SizeX_Var_Index].Data.i)
        {
                if (Caller->User_Vars[Object_PosX_Var_Index].Data.i + Caller->User_Vars[Object_SizeX_Var_Index].Data.i >= Target->User_Vars[Object_PosX_Var_Index].Data.i)
                {
                        if (Caller->User_Vars[Object_PosY_Var_Index].Data.i <= Target->User_Vars[Object_PosY_Var_Index].Data.i + Target->User_Vars[Object_SizeY_Var_Index].Data.i)
                        {
                                if (Caller->User_Vars[Object_PosY_Var_Index].Data.i + Caller->User_Vars[Object_SizeY_Var_Index].Data.i >= Target->User_Vars[Object_PosY_Var_Index].Data.i)
                                {
                                        return true;
                                }
                        }
                }
        }
        return false;
}

/**
This function will check if a hit happened in every object inside the objectscene
pointed by this object.
VARS[0] = State to jump if object hit A solid object
VARS[1] = Amount of damage in case (SHOULD NOT BE A POINTER TYPE)
VARS[2] = The index of the var that's used to represent the health on the other object
**/
void Object_GotoState_IfHit(Object *Caller, MT2D_VAR **Vars) {
        int i = 0;
        int ObjVarInded = 0;
        while (i < Caller->MyScene->Count)
        {
                if (Caller->MyScene->ObjectGroup[i]->Solid && Object_Hit_Object(Caller, Caller->MyScene->ObjectGroup[i]))
                {
                        /* Object collided. */
                        Vars[2]->Data.p = Caller->MyScene->ObjectGroup[i];
                        ObjVarInded = Cscript_Get_Object_VarIndex(Vars[2]);
                        Vars[2]->Data.p = 0;
                        MT2D_Object_SUB(&Caller->MyScene->ObjectGroup[i]->User_Vars[ObjVarInded], Vars[1]);
                        i = Caller->MyScene->Count;
                        Cscript_Object_SetState(Caller, Vars);
                }
                i++;
        }
}

/**
This function will check if a hit happened on a given object inside the objectscene
pointed by this object.
VARS[0] = the state to jump
**/
void Object_GotoState_IfHitObject(Object* Caller, Object* Target, MT2D_VAR** Vars)
{
        if (Target->Solid && Object_Hit_Object(Caller, Target))
        {
                Cscript_Object_SetState(Caller, Vars);
        }
}

/**
VARS[0] the number to be increased
VARS[1] the amount to increase
**/
void Cscript_VAR_ADD(Object *object, MT2D_VAR **VARS) {
        if (VARS[0]->Type == VAR_POINTER) {
                MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(object, VARS[0]));
                if (VARS[1]->Type == VAR_POINTER) {
                        MT2D_VAR *Tmp2 = MT2D_Object_Create_Var_Int((char*)"Tmp2", Cscript_VAR_Get_Integer(object, VARS[1]));
                        MT2D_Object_ADD(Tmp, Tmp2);
                        Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
                        MT2D_VAR_Free(Tmp2,1);
                }
                else {
                        MT2D_Object_ADD(Tmp, VARS[1]);
                        Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
                }
                MT2D_VAR_Free(Tmp,1);
        }
        else {
                if (VARS[1]->Type == VAR_POINTER) {
                        MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(object, VARS[1]));
                        MT2D_Object_ADD(VARS[0], Tmp);
                        MT2D_VAR_Free(Tmp,1);
                }
                else {
                        MT2D_Object_ADD(VARS[0], VARS[1]);
                }
        }
}

/**
VARS[0] the number to be decreased
VARS[1] the amount to decrease
**/
void Cscript_VAR_SUB(Object *object, MT2D_VAR **VARS) {
        if (VARS[0]->Type == VAR_POINTER) {
                MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(object, VARS[0]));
                if (VARS[1]->Type == VAR_POINTER) {
                        MT2D_VAR *Tmp2 = MT2D_Object_Create_Var_Int((char*)"Tmp2", Cscript_VAR_Get_Integer(object, VARS[1]));
                        MT2D_Object_SUB(Tmp, Tmp2);
                        Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
                        MT2D_VAR_Free(Tmp2,1);
                }
                else {
                        MT2D_Object_SUB(Tmp, VARS[1]);
                        Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
                }
                MT2D_VAR_Free(Tmp,1);
        }
        else {
                if (VARS[1]->Type == VAR_POINTER) {
                        MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(object, VARS[1]));
                        MT2D_Object_SUB(VARS[0], Tmp);
                        MT2D_VAR_Free(Tmp,1);
                }
                else {
                        MT2D_Object_SUB(VARS[0], VARS[1]);
                }
        }
}

/**
        object = caller
        VAR[0] = pointer. (always)
**/
void Cscript_VAR_INC(Object *object, MT2D_VAR **VAR) {
 MT2D_VAR *One = MT2D_Object_Create_Var_Char((char*)"One", 1);
        if (VAR[0]->Type == VAR_POINTER) {
                MT2D_VAR * tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(object, VAR[0]));
                MT2D_Object_ADD(tmp, One);
                Cscript_VAR_Set_Integer(object, VAR[0], tmp);
                MT2D_VAR_Free(tmp,1);
        }
        else {
                MT2D_Object_ADD(VAR[0], One);
        }
        MT2D_VAR_Free(One,1);
}

/**
object = caller
VAR[0] = pointer. (always)
**/
void Cscript_VAR_DEC(Object *object, MT2D_VAR **VAR) {
        MT2D_VAR *One = MT2D_Object_Create_Var_Char((char*)"One", 1);
        if (VAR[0]->Type == VAR_POINTER) {
                MT2D_VAR * tmp = MT2D_Object_Create_Var_Int((char*)"Tmp", Cscript_VAR_Get_Integer(object, VAR[0]));
                MT2D_Object_SUB(tmp, One);
                Cscript_VAR_Set_Integer(object, VAR[0], tmp);
                MT2D_VAR_Free(tmp, 1);
        }
        else {
                MT2D_Object_SUB(VAR[0], One);
        }
        MT2D_VAR_Free(One,1);
}

/* The operator of a Calc var: '+', '-', '*', '/' or '%' from a VAR_STRING var, 0 for anything else. */
static char Private_Calc_Op(MT2D_VAR *Var) {
        const char *s;
        if (!Var || Var->Type != VAR_STRING || !Var->Data.p) return 0;
        s = (const char*)Var->Data.p;
        if (s[0] == 0 || s[1] != 0 || !strchr("+-*/%", s[0])) return 0;
        return s[0];
}

/* C89 lets a compiler round a negative / down; when the preprocessor rounds toward zero the compiler does too
   (true on x86: idiv), so the fix-up is not built. */
#if (-7 / 2) == -3 && (-7 % 2) == -1
#define MT2D_CALC_DIV_TRUNCATES 1
#else
#define MT2D_CALC_DIV_TRUNCATES 0
#endif

/* a / b (Mod = 0) or a % b (Mod = 1) rounded toward zero on every compiler. b must not be 0.
   b = -1 does not divide: INT_MIN / -1 does not fit an int and makes x86 raise the divide error
   ("Divide overflow" on DOS), so the result is -a (INT_MIN stays INT_MIN) and 0. */
static int Private_Calc_Div(int a, int b, int Mod) {
        int q, r;
        if (b == -1) return Mod ? 0 : (int)(0u - (unsigned)a);
        q = a / b;
        r = a % b;
#if !MT2D_CALC_DIV_TRUNCATES
        if (r != 0 && ((r < 0) != (a < 0))) { /* the compiler rounded down */
                q += 1;
                r -= b;
        }
#endif
        return Mod ? r : q;
}

/* The var a Calc arg stands for: the var of the caller with that name for a VAR_POINTER (0 when it has none),
   the var itself for anything else (a number or a global var). */
static MT2D_VAR *Private_Calc_Var(Object *object, MT2D_VAR *Var) {
        int i;
        if (Var->Type != VAR_POINTER) return Var;
        for (i = 0; i < object->User_Vars_Count; i++) {
                if (MT2D_NAME_EQ(object->User_Vars[i].Name, Var->Name)) return &object->User_Vars[i];
        }
        return 0;
}

/* The value of an operand, 0 for a var the caller does not have. */
static int Private_Calc_Int(Object *object, MT2D_VAR *Var) {
        MT2D_VAR *v = Private_Calc_Var(object, Var);
        return v ? MT2D_Object_VAR_GetInt(v) : 0;
}

static float Private_Calc_Float(Object *object, MT2D_VAR *Var) {
        MT2D_VAR *v = Private_Calc_Var(object, Var);
        if (!v) return 0.0f;
        if (v->Type == VAR_FLOAT) return v->Data.f;
        return (float)MT2D_Object_VAR_GetInt(v);
}

/* VARS[2..count] in int, 0 when the list is wrong or divides by 0 */
static int Private_Calc_Ints(Object *object, MT2D_VAR **VARS, int count, int *Out) {
        int i, value, term, total = 0, neg = 0;
        char op;
        term = Private_Calc_Int(object, VARS[2]);
        for (i = 3; i + 1 <= count; i += 2) {
                op = Private_Calc_Op(VARS[i]);
                if (!op || VARS[i + 1]->Type == VAR_STRING) return 0;
                value = Private_Calc_Int(object, VARS[i + 1]);
                if (op == '*') term *= value;
                else if (op == '/' || op == '%') {
                        if (value == 0) return 0;
                        term = Private_Calc_Div(term, value, op == '%');
                }
                else { /* + or -: the term is done, add it with its sign (no imul) */
                        if (neg) total -= term; else total += term;
                        neg = op == '-';
                        term = value;
                }
        }
        *Out = neg ? total - term : total + term;
        return 1;
}

/* the same in float, % is refused */
static int Private_Calc_Floats(Object *object, MT2D_VAR **VARS, int count, float *Out) {
        int i;
        float value, term, total = 0.0f, sign = 1.0f;
        char op;
        term = Private_Calc_Float(object, VARS[2]);
        for (i = 3; i + 1 <= count; i += 2) {
                op = Private_Calc_Op(VARS[i]);
                if (!op || op == '%' || VARS[i + 1]->Type == VAR_STRING) return 0;
                value = Private_Calc_Float(object, VARS[i + 1]);
                if (op == '*') term *= value;
                else if (op == '/') {
                        if (value == 0.0f) return 0;
                        term /= value;
                }
                else {
                        total += sign * term;
                        sign = op == '-' ? -1.0f : 1.0f;
                        term = value;
                }
        }
        *Out = total + sign * term;
        return 1;
}

/**
Store = math, without parentheses: * / % before + -, then left to right.
VARS[0] = how many vars come after it (the store and the math): 2, 4, 6 ...
VARS[1] = where the result is stored
VARS[2] = the first operand, then pairs of operator ("+" "-" "*" "/" "%" as VAR_STRING) and operand.
        Cscript_New3V(Cscript_Calc, 7, count6, x, a, plus, b, times, two)   ->   x = a + b * 2
The type of the store picks the math: a VAR_FLOAT store is computed in float (no %), any other in int.
Nothing is stored when the list is wrong (bad count, an operator that is not one, a store the caller does not have)
or on a / or % by 0.
**/
void Cscript_Calc(Object *object, MT2D_VAR **VARS) {
        MT2D_VAR Result;
        MT2D_VAR *store;
        int count, total;
        float totalF;
        if (!VARS || !VARS[0] || VARS[0]->Type == VAR_STRING) return;
        count = Cscript_VAR_Get_Integer(object, VARS[0]);
        if (count < 2 || (count & 1)) return;
        if (VARS[1]->Type == VAR_STRING || VARS[2]->Type == VAR_STRING) return;
        store = Private_Calc_Var(object, VARS[1]);
        if (!store) return;
        if (store->Type == VAR_FLOAT) {
                if (!Private_Calc_Floats(object, VARS, count, &totalF)) return;
                MT2D_VAR_Init_Float(&Result, "Result", totalF);
        }
        else {
                if (!Private_Calc_Ints(object, VARS, count, &total)) return;
                MT2D_VAR_Init_Int(&Result, "Result", total);
        }
        MT2D_Object_VAR_SetVar(store, &Result);
}

/* The file is the text of a VAR_STRING var, or the Name of any other var. */
void Cscript_PlaySound(Object *object, MT2D_VAR **SoundName) {
        MT2D_Play_Audio((char*)MT2D_Object_VAR_GetPath(SoundName[0]));
}

void Cscript_PlayMusic(Object *object, MT2D_VAR **MusicName) {
        MT2D_Play_Music((char*)MT2D_Object_VAR_GetPath(MusicName[0]));
}

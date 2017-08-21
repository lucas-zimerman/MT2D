#pragma once
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/Vars.h>

/*
Spawn1{
Sprite0 1 A_Move
Sprite0 0 A_JumpifLowerEq "Reset" "Health" 0
Sprite1 1 A_Move
}
}
Reset{
Sprite0 0 A_PlaySound "Hit.wav
Sprite0 1 A_DEC    Pos->Y
Sprite0 1 A_DEC Pos->Y
Sprite0 1 A_INC Pos->Y 16
Sprite0 1 A_ADD Pos->Y 2
Sprite0 1 A_ADD Pos->Y 3
Sprite0 1 A_ADD Pos->Y 3
Sprite0 0 A_AddVar Pos->X 200
Sprite0 0 A_SetVar Pos->Y 17
Sprite0 0 A_SetVar Health 20
Sprite0 0 Goto "Spawn1"
}

*/

#pragma region AI FUNCTIONS
void Cscript_TeleportToCoord(Object *object, MT2D_VAR **X_Y);
void Cscript_Move(Object *object);
void Cscript_CreateObject(Object *Caller, Object *NewModel, MT2D_VAR **Vars);
void Object_GotoState_IfHit(Object *Caller, MT2D_VAR **Vars);

#pragma endregion


#pragma region LOGIC FUNCTIONS
void Cscript_JumpToStateIfRandom(Object *object, MT2D_VAR **JUMPTO_CHANCE_MAX);
void Cscript_JumpToStateIfVarLowerEq(Object *object, MT2D_VAR **Vars);
void Cscript_JumpToStateIfVarHigherEq(Object *object, MT2D_VAR **Vars);

void Cscript_JumpToStateIfVarEq(Object *object, MT2D_VAR **Vars);
void Cscript_JumpNextFrameIfVarEq(Object *object, MT2D_VAR **Vars);
void Cscript_Object_SetState(Object *Object, MT2D_VAR **StateName);
void Cscript_Set_Var(Object *Object, MT2D_VAR **StateName);
void Cscript_RemoveObject(Object *Caller);
#pragma endregion

#pragma region MATH FUNCTIONS
void Cscript_VAR_INC(Object *object, MT2D_VAR **VAR);
void Cscript_VAR_DEC(Object *object, MT2D_VAR **VAR);
void Cscript_VAR_ADD(Object *object, MT2D_VAR **VARS);
void Cscript_VAR_SUB(Object *object, MT2D_VAR **VARS);
#pragma endregion



#pragma region AUDIO FUNCTIONS
void Cscript_PlaySound(Object *object,MT2D_VAR **SoundName);
void Cscript_PlayMusic(Object *object,MT2D_VAR **MusicName);
#pragma endregion

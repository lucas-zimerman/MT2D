#pragma once
#include <MT2D/ObjectCore/Functions.h>
#include <MT2D/ObjectCore/Vars.h>

#pragma region AI FUNCTIONS
void ObjectCore_TeleportToCoord(Object *object, MT2D_VAR **X_Y, int TwoVars);
void ObjectCore_Move(Object *object);
void ObjectCore_CreateObject(Object *Caller, Object *NewModel, MT2D_VAR **Vars);
void Object_GotoState_IfHit(Object *Caller, MT2D_VAR **Vars, int State_AmDmg_HitHealthIndex);

#pragma endregion


#pragma region LOGIC FUNCTIONS
void ObjectCore_JumpToStateIfRandom(Object *object, MT2D_VAR **JUMPTO_CHANCE_MAX, int TreeVars);
void ObjectCore_JumpToStateIfVarLowerEq(Object *object, MT2D_VAR **Vars, int TreeVars);
void ObjectCore_JumpToStateIfVarEq(Object *object, MT2D_VAR **Vars, int TreeVars);
void ObjectCore_Object_SetState(Object *Object, MT2D_VAR **StateName);
void ObjectCore_Set_Var(Object *Object, MT2D_VAR **StateName, int TwoVars);
void ObjectCore_RemoveObject(Object *Caller);
#pragma endregion

#pragma region MATH FUNCTIONS
void ObjectCore_VAR_INC(Object *object, MT2D_VAR **VAR);
void ObjectCore_VAR_DEC(Object *object, MT2D_VAR **VAR);
void ObjectCore_VAR_ADD(Object *object, MT2D_VAR **VARS, int Two);
void ObjectCore_VAR_SUB(Object *object, MT2D_VAR **VARS, int Two);
#pragma endregion



#pragma region AUDIO FUNCTIONS
void ObjectCore_PlaySound(Object *object,MT2D_VAR **SoundName);
void ObjectCore_PlayMusic(Object *object,MT2D_VAR **MusicName);
#pragma endregion

#pragma once
#include <MT2D/ObjectCore/Functions.h>
#include <MT2D/ObjectCore/Vars.h>

#pragma region AI FUNCTIONS
void ObjectCore_TeleportToCoord(Object *object, MT2D_VAR *X_Y, int TwoVars);
void ObjectCore_Move(Object *object);
#pragma endregion


#pragma region LOGIC FUNCTIONS
void ObjectCore_JumpToStateIfRandom(Object *object, MT2D_VAR *JUMPTO_CHANCE_MAX, int TreeVars);
void ObjectCore_Object_SetState(Object *Object, MT2D_VAR *StateName);
#pragma endregion

#pragma region MATH FUNCTIONS
void ObjectCore_VAR_INC(Object *object, MT2D_VAR *VAR);
void ObjectCore_VAR_DEC(Object *object, MT2D_VAR *VAR);
#pragma endregion



#pragma region AUDIO FUNCTIONS
void ObjectCore_PlaySound(MT2D_VAR *SoundName);
void ObjectCore_PlayMusic(MT2D_VAR *MusicName);
#pragma endregion

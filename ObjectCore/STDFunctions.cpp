#include <MT2D/ObjectCore/Functions.h>
#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <cstring>
#include <cstdlib>

void ObjectCore_Object_SetState(Object *Object, MT2D_VAR *StateName) {
	int i = 0;
	while (i < Object->States_Count) {
		if (strcmp(Object->State[i].Name, (char*)StateName->Data) == 0) {
			Object->ActualState = i;
			Object->ActualFrame = 0;
			Object->ActualFrameWait = Object->State[i].WaitSprites[0];
			i = Object->States_Count;
		}
		i++;
	}
}

void ObjectCore_VAR_INC(Object *object, MT2D_VAR *VAR) {
	int i = 0;
	MT2D_VAR *One = MT2D_Object_Create_Var_Char("One",1);
	while (i < object->User_Vars_Count) {
		if (strcmp(object->User_Vars[i].Name, VAR->Name) == 0) {
			MT2D_Object_ADD(&object->User_Vars[i], One);
			i = object->User_Vars_Count;
		}
		i++;
	}
}

void ObjectCore_VAR_DEC(Object *object, MT2D_VAR *VAR) {
	int i = 0;
	MT2D_VAR *One = MT2D_Object_Create_Var_Char("One", 1);
	while (i < object->User_Vars_Count) {
		if (strcmp(object->User_Vars[i].Name, VAR->Name) == 0) {
			MT2D_Object_SUB(&object->User_Vars[i], One);
			i = object->User_Vars_Count;
		}
		i++;
	}
}
//Object *, MT2D_VAR *, int
/**
	Main Object
**/
void ObjectCore_JumpToStateIfRandom(Object *object, MT2D_VAR *JUMPTO_CHANCE_MAX, int TreeVars) {
	int i = 0;
	int MAX = MT2D_Object_VAR_GetInt(&JUMPTO_CHANCE_MAX[2]);
	int SEL_RANGE = MT2D_Object_VAR_GetInt(&JUMPTO_CHANCE_MAX[1]);
	if (rand() % MAX <= SEL_RANGE) {
		while (i < object->States_Count) {
			if (strcmp(object->State[i].Name, (char*)&JUMPTO_CHANCE_MAX[0].Data) == 0) {
				object->ActualState = i;
				object->ActualFrame = 0;
				object->ActualFrameWait = object->State[i].WaitSprites[0];
				i = object->States_Count;
			}
			i++;
		}
	}
}

void ObjectCore_PlaySound(MT2D_VAR *SoundName) {
	if (SoundName->Name) {
		MT2D_Play_Audio(SoundName->Name);
	}
}

void ObjectCore_PlayMusic(MT2D_VAR *MusicName) {
	if (MusicName->Name) {
		MT2D_Play_Music(MusicName->Name);
	}
}


void ObjectCore_TeleportToCoord(Object *object, MT2D_VAR *X_Y, int TwoVars) {
	int X = MT2D_Object_VAR_GetInt(&X_Y[0]);
	int Y = MT2D_Object_VAR_GetInt(&X_Y[1]);
	object->SpacePosition.X = X;
	object->SpacePosition.Y = Y;
}

void ObjectCore_Move(Object *object) {
	object->SpacePosition.X += object->Aceleration.X;
	object->SpacePosition.Y += object->Aceleration.Y;
}
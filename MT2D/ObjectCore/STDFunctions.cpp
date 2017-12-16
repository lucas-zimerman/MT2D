#ifndef __ANDROID__
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#else
#include "../ObjectCore/Cscript.h"
#include "../ObjectCore/Vars.h"
#include "../Audio/MT2D_Audio_core.h"
#endif
#include <cstring>
#include <cstdlib>


#pragma region INTERNAL

int Cscript_VAR_Get_Integer(Object *object, MT2D_VAR *Var);

/**
Return the index of a Var inside an Object where is
the first similar Var with the same requested name
by the Pointer.
**/
int Cscript_Get_Object_VarIndex(MT2D_VAR *Var) {
	int out = -1;
	Object *O = (Object*)Var->Data;
	int i = 0;
	for (i < 0; i < O->User_Vars_Count; i++) {
		if (strcmp(O->User_Vars[i].Name, Var->Name) == 0) {
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
	Object *O = (Object*)Var->Data;
	int Return = 0;
	if (O) {
		if (strcmp(Var->Name, "Acel_X") == 0) {
			Return = O->Aceleration.X;
		}
		else if (strcmp(Var->Name, "Acel_Y") == 0) {
			Return = O->Aceleration.Y;
		}
		else if (strcmp(Var->Name, "Pos_X") == 0) {
			Return = O->SpacePosition.X;
		}
		else if (strcmp(Var->Name, "Pos_Y") == 0) {
			Return = O->SpacePosition.Y;
		}
		else if (strcmp(Var->Name, "Size_X") == 0) {
			Return = O->Size.X;
		}
		else if (strcmp(Var->Name, "Size_Y") == 0) {
			Return = O->Size.Y;
		}
		else {
			//check vars
			int i = 0;
			for (i < 0; i < O->User_Vars_Count; i++) {
				if (strcmp(O->User_Vars[i].Name, Var->Name) == 0) {
					Return = MT2D_Object_VAR_GetInt(&O->User_Vars[i]);
					i = O->User_Vars_Count;
				}
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
void Cscript_Set_Object_Internal_Data(Object *Caller, MT2D_VAR *Var, MT2D_VAR *NewVar) {
	Object *O = (Object*)Var->Data;
	int Return = 0;
	if (O) {
		if (strcmp(Var->Name, "Acel_X") == 0) {
			O->Aceleration.X = Cscript_VAR_Get_Integer(Caller, NewVar);
		}
		else if (strcmp(Var->Name, "Acel_Y") == 0) {
			O->Aceleration.Y = Cscript_VAR_Get_Integer(Caller, NewVar);
		}
		else if (strcmp(Var->Name, "Pos_X") == 0) {
			O->SpacePosition.X = Cscript_VAR_Get_Integer(Caller, NewVar);
		}
		else if (strcmp(Var->Name, "Pos_Y") == 0) {
			O->SpacePosition.Y = Cscript_VAR_Get_Integer(Caller, NewVar);;
		}
		else if (strcmp(Var->Name, "Size_X") == 0) {
			O->Size.X = Cscript_VAR_Get_Integer(Caller, NewVar);;
		}
		else if (strcmp(Var->Name, "Size_Y") == 0) {
			O->Size.Y = Cscript_VAR_Get_Integer(Caller, NewVar);;
		}
		else {
			//check vars
			int i = 0;
			for (i < 0; i < O->User_Vars_Count; i++) {
				if (strcmp(O->User_Vars[i].Name, Var->Name) == 0) {
					MT2D_Object_VAR_SetVar(&O->User_Vars[i], NewVar);
					i = O->User_Vars_Count;
				}
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
		Var->Data = object;
		Return = Cscript_Get_Object_Internal_Data(Var);
		Var->Data = 0;
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
		Var->Data = object;
		if (NewData->Type == VAR_POINTER) {
			MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_Get_Object_Internal_Data(NewData));
			Cscript_Set_Object_Internal_Data(object, Var, Tmp);
			//Del tmp
		}
		else {
			Cscript_Set_Object_Internal_Data(object, Var, NewData);
		}
	}
	else if (NewData->Type == VAR_POINTER) {
		MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_Get_Object_Internal_Data(NewData));
		MT2D_Object_VAR_SetVar(Var, NewData);
		//		del tmp

	}
	else {
		MT2D_Object_VAR_SetVar(Var, NewData);
	}
}

#pragma endregion FUNCTIONS

#pragma region LOGIC



void Cscript_Object_SetState(Object *Object, MT2D_VAR **StateName) {
	int i = 0;
	while (i < Object->States_Count) {
		if (strcmp(Object->State[i]->Name, (char*)StateName[0]->Name) == 0) {
			Object->ActualState = i;
			Object->ActualFrame = -1;//after this call the function is going to be increased by one so we avoid the code to miss the first state.
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
		if (VARS[0]->Data == 0) {
			VARS[0]->Data = Object;
			if (VARS[1]->Type == VAR_POINTER) {
				VARS[1]->Data = Object;
				Cscript_VAR_Set_Integer(Object, VARS[0], VARS[1]);
				VARS[1]->Data = 0;
			}
			else {
				Cscript_VAR_Set_Integer(Object, VARS[0], VARS[1]);
			}
			VARS[0]->Data = 0;
		}
		else {
			Cscript_Set_Object_Internal_Data(Object, VARS[0], VARS[1]);
		}
	}
	else if (VARS[1]->Type == VAR_POINTER) {
		tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(Object, VARS[1]));
		MT2D_Object_VAR_SetVar(VARS[0], tmp);
		//del tmp
	}
	else {
		MT2D_Object_VAR_SetVar(VARS[0], VARS[1]);
	}
}


/**
object  = caller
VARS[0] = Name of the state to jump
VARS[1] = MAX vALUe to JUMP
VARS[2] = ACTUAL VALUE TO SELECT (Rand()%VARS[2]  <= VARS[1])
Main Object
**/
void Cscript_JumpToStateIfRandom(Object *object, MT2D_VAR **JUMPTO_CHANCE_MAX) {
	int i = 0;
	int MAX = Cscript_VAR_Get_Integer(object, JUMPTO_CHANCE_MAX[2]);
	int SEL_RANGE = Cscript_VAR_Get_Integer(object, JUMPTO_CHANCE_MAX[1]);
	if (rand() % MAX <= SEL_RANGE) {
		while (i < object->States_Count) {
			if (strcmp(object->State[i]->Name, (char*)JUMPTO_CHANCE_MAX[0]->Name) == 0) {
				object->ActualState = i;
				object->ActualFrame = 0;
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
[1] <= [2], jump if true
*/
void Cscript_JumpToStateIfVarLowerEq(Object *object, MT2D_VAR **Vars) {
	int i = 0;
	int Left = Cscript_VAR_Get_Integer(object, Vars[1]);
	int Right = Cscript_VAR_Get_Integer(object, Vars[2]);
	if (Left <= Right) {
        		while (i < object->States_Count) {
			if (strcmp(object->State[i]->Name, Vars[0]->Name) == 0) {
				object->ActualState = i;
				object->ActualFrame = -1;//after this call the function is going to be increased by one so we avoid the code to miss the first state.
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
			if (strcmp(object->State[i]->Name, Vars[0]->Name) == 0) {
				object->ActualState = i;
				object->ActualFrame = -1;//after this call the function is going to be increased by one so we avoid the code to miss the first state.
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
		if (strcmp(Vars[0]->Name, "Moving") == 0) {
			i = 0;
		}
		while (i < object->States_Count) {
			if (strcmp(object->State[i]->Name, Vars[0]->Name) == 0) {
				object->ActualState = i;
				object->ActualFrame = -1;//after this call the function is going to be increased by one so we avoid the code to miss the first state.
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
	int i = 0;
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
	int i = 0;
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
	int i = 0;
	int Left = Cscript_VAR_Get_Integer(object, Vars[0]);
	int Right = Cscript_VAR_Get_Integer(object, Vars[1]);
	if (Left >= Right) {
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
		if(strcmp(Var[i]->Name,Caller->User_Vars[i].Name) == 0){
			i = Caller->User_Vars_Count;
		}
		i++;		
	}		
	if(i ==  Caller->User_Vars_Count){
		/*We didn't found a duplicated*/
		Caller->User_Vars_Count++;
		Caller->User_Vars = (MT2D_VAR*)realloc(Caller->User_Vars,Caller->User_Vars_Count*sizeof(MT2D_VAR));
		Caller->User_Vars[i] = *MT2D_VAR_CLONE(Var[0]);
	}
}

/*TODO void Cscript_RemoveVar_FromObject(OBject *Caller, MT2D_VAR **Var);*/


#pragma endregion FUNCTIONS


#pragma region AI

void Cscript_TeleportToCoord(Object *object, MT2D_VAR **X_Y) {
	int X = Cscript_VAR_Get_Integer(object, X_Y[0]);
	int Y = Cscript_VAR_Get_Integer(object, X_Y[1]);
	object->SpacePosition.X = X;
	object->SpacePosition.Y = Y;
}

void Cscript_Move(Object *object) {
	object->SpacePosition.X += object->Aceleration.X;
	object->SpacePosition.Y += object->Aceleration.Y;
}
/**
Vars[0] = Pos X
Vars[1] = Pos Y
Vars[2] = Acel X
Vars[3] = Acel Y
**/
void Cscript_CreateObject(Object *Caller, Object *NewModel, MT2D_VAR **Vars) {
	Object *NewObject;
	int X[2] = { Cscript_VAR_Get_Integer(Caller,Vars[0]),Cscript_VAR_Get_Integer(Caller,Vars[2]) };
	int Y[2] = { Cscript_VAR_Get_Integer(Caller,Vars[1]),Cscript_VAR_Get_Integer(Caller,Vars[3]) };
	NewObject = Object_Create(NewModel->Solid, NewModel->RenderOnly, NewModel->Size.X, NewModel->Size.X, X[0], Y[0], NewModel->State, NewModel->States_Count);
	NewObject->Aceleration.X = X[1];
	NewObject->Aceleration.Y = Y[1];
	if (NewModel->User_Vars_Count > 0) {
		MT2D_VAR *NewVars = (MT2D_VAR*)malloc(NewModel->User_Vars_Count * sizeof(MT2D_VAR));
		for (int i = 0; i < NewModel->User_Vars_Count; i++) {
			NewVars[i] = *MT2D_VAR_CLONE(&NewModel->User_Vars[i]);
		}

		NewObject->User_Vars = NewVars;
		NewObject->User_Vars_Count = NewModel->User_Vars_Count;
	}
	ObjectScene_Add(Caller->MyScene, NewObject);
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
	while (i < Caller->MyScene->Count) {
		if (Caller->MyScene->ObjectGroup[i]->Solid == true) {
			if (Caller->SpacePosition.X <= Caller->MyScene->ObjectGroup[i]->SpacePosition.X + Caller->MyScene->ObjectGroup[i]->Size.X) {
				if (Caller->SpacePosition.X + Caller->Size.X >= Caller->MyScene->ObjectGroup[i]->SpacePosition.X) {
					if (Caller->SpacePosition.Y <= Caller->MyScene->ObjectGroup[i]->SpacePosition.Y + Caller->MyScene->ObjectGroup[i]->Size.Y) {
						if (Caller->SpacePosition.Y + Caller->Size.Y >= Caller->MyScene->ObjectGroup[i]->SpacePosition.Y) {
							//there's indeed a hit
							//so lets do some dmg in this object.
							Vars[2]->Data = Caller->MyScene->ObjectGroup[i];
							ObjVarInded = Cscript_Get_Object_VarIndex(Vars[2]);
							Vars[2]->Data = 0;
							MT2D_Object_SUB(&Caller->MyScene->ObjectGroup[i]->User_Vars[ObjVarInded], Vars[1]);
							i = Caller->MyScene->Count;
							Cscript_Object_SetState(Caller, Vars);
						}
					}
				}
			}
		}
		i++;
	}
}

#pragma endregion FUNCTIONS


#pragma region MATH

/**
VARS[0] the number to be increased
VARS[1] the amount to increase
**/
void Cscript_VAR_ADD(Object *object, MT2D_VAR **VARS) {
	int i = 0;
	if (VARS[0]->Type == VAR_POINTER) {
		MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(object, VARS[0]));
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp2 = MT2D_Object_Create_Var_Int("Tmp2", Cscript_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_ADD(Tmp, Tmp2);
			Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
		else {
			MT2D_Object_ADD(Tmp, VARS[1]);
			Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
	}
	else {
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_ADD(VARS[0], Tmp);
			//Del Tmp
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
	int i = 0;
	if (VARS[0]->Type == VAR_POINTER) {
		MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(object, VARS[0]));
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp2 = MT2D_Object_Create_Var_Int("Tmp2", Cscript_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_SUB(Tmp, Tmp2);
			Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
		else {
			MT2D_Object_SUB(Tmp, VARS[1]);
			Cscript_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
	}
	else {
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_SUB(VARS[0], Tmp);
			//Del Tmp
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
 MT2D_VAR *One = MT2D_Object_Create_Var_Char("One", 1);
	if (VAR[0]->Type == VAR_POINTER) {
		MT2D_VAR * tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(object, VAR[0]));
		MT2D_Object_ADD(tmp, One);
		Cscript_VAR_Set_Integer(object, VAR[0], tmp);
	}
	else {
		MT2D_Object_ADD(VAR[0], One);
	}
}

/**
object = caller
VAR[0] = pointer. (always)
**/
void Cscript_VAR_DEC(Object *object, MT2D_VAR **VAR) {
 	MT2D_VAR *One = MT2D_Object_Create_Var_Char("One", 1);
	if (VAR[0]->Type == VAR_POINTER) {
		MT2D_VAR * tmp = MT2D_Object_Create_Var_Int("Tmp", Cscript_VAR_Get_Integer(object, VAR[0]));
		MT2D_Object_SUB(tmp, One);
		Cscript_VAR_Set_Integer(object, VAR[0], tmp);
	}
	else {
		MT2D_Object_SUB(VAR[0], One);
	}
}


#pragma endregion FUNCTIONS

#pragma region AUDIO

void Cscript_PlaySound(Object *object,MT2D_VAR **SoundName) {
	if (SoundName[0]->Name) {
		MT2D_Play_Audio(SoundName[0]->Name);
	}
}

void Cscript_PlayMusic(Object *object,MT2D_VAR **MusicName) {
	if (MusicName[0]->Name) {
		MT2D_Play_Music(MusicName[0]->Name);
	}
}

#pragma endregion FUNCTIONS
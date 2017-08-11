#include <MT2D/ObjectCore/Functions.h>
#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <cstring>
#include <cstdlib>

int ObjectCore_VAR_Get_Integer(Object *object, MT2D_VAR *Var);

int ObjectCore_Get_Object_VarIndex(MT2D_VAR *Var) {
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
>> USE ObjectCore_VAR_Get_Integer
	Get the value from the internal struct of an object.
	use the function ObjectCore_VAR_Get_Integer in case you are dealing
	with other types of data other than object struct data.
**/
int ObjectCore_Get_Object_Internal_Data(MT2D_VAR *Var) {
	Object *O = (Object*)Var->Data;
	int Return = 0;
	if (O) {
		if (strcmp(Var->Name, "Acel_X")==0) {
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
			for (i < 0;i < O->User_Vars_Count; i++) {
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
Get the value from the internal struct of an object.
use the function ObjectCore_VAR_Get_Integer in case you are dealing
with other types of data other than object struct data.
**/
void ObjectCore_Set_Object_Internal_Data(Object *Caller, MT2D_VAR *Var, MT2D_VAR *NewVar) {
	Object *O = (Object*)Var->Data;
	int Return = 0;
	if (O) {
		if (strcmp(Var->Name, "Acel_X") == 0) {
			O->Aceleration.X = ObjectCore_VAR_Get_Integer(Caller,NewVar);
		}
		else if (strcmp(Var->Name, "Acel_Y") == 0) {
			O->Aceleration.Y = ObjectCore_VAR_Get_Integer(Caller, NewVar);
		}
		else if (strcmp(Var->Name, "Pos_X") == 0) {
			O->SpacePosition.X = ObjectCore_VAR_Get_Integer(Caller, NewVar);
		}
		else if (strcmp(Var->Name, "Pos_Y") == 0) {
			O->SpacePosition.Y = ObjectCore_VAR_Get_Integer(Caller, NewVar);;
		}
		else if (strcmp(Var->Name, "Size_X") == 0) {
			O->Size.X = ObjectCore_VAR_Get_Integer(Caller, NewVar);;
		}
		else if (strcmp(Var->Name, "Size_Y") == 0) {
			O->Size.Y = ObjectCore_VAR_Get_Integer(Caller, NewVar);;
		}
	}
}


/**
	gets an integer representation of the data pointed in MT2D_VAR.
	It checks what type of var is before taking directly the data.
**/
int ObjectCore_VAR_Get_Integer(Object *object, MT2D_VAR *Var) {
	int Return = 0;
	if (Var->Type == VAR_POINTER) {
		Var->Data = object;
		Return = ObjectCore_Get_Object_Internal_Data(Var);
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
void ObjectCore_VAR_Set_Integer(Object *object, MT2D_VAR *Var,MT2D_VAR *NewData) {
	if (Var->Type == VAR_POINTER) {
		Var->Data = object;
		ObjectCore_Set_Object_Internal_Data(object, Var, NewData);
	}
	else if (NewData->Type == VAR_POINTER) {
		MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", ObjectCore_Get_Object_Internal_Data(NewData));
		MT2D_Object_VAR_SetVar(Var, NewData);
//		del tmp

	}
	else {
		MT2D_Object_VAR_SetVar(Var, NewData);
	}
}

void ObjectCore_Object_SetState(Object *Object, MT2D_VAR **StateName) {
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

void ObjectCore_VAR_INC(Object *object, MT2D_VAR **VAR) {
	int i = 0;
	MT2D_VAR *One = MT2D_Object_Create_Var_Char("One",1);
	while (i < object->User_Vars_Count) {
		if (strcmp(object->User_Vars[i].Name, VAR[0]->Name) == 0) {
			MT2D_Object_ADD(&object->User_Vars[i], One);
			i = object->User_Vars_Count;
		}
		i++;
	}
}

void ObjectCore_VAR_DEC(Object *object, MT2D_VAR **VAR) {
	MT2D_VAR *One = MT2D_Object_Create_Var_Char("One", 1);
	MT2D_Object_SUB(VAR[0], One);
/*
	while (i < object->User_Vars_Count) {
		if (strcmp(object->User_Vars[i].Name, VAR[0]->Name) == 0) {
			MT2D_Object_SUB(&object->User_Vars[i], One);
			i = object->User_Vars_Count;
		}
		i++;
	}
	*/
}

/**
	VARS[0] = Pointer
	VARS[1] = Data to transfer
**/
void ObjectCore_Set_Var(Object *Object, MT2D_VAR **VARS, int TwoVars) {
	MT2D_VAR *tmp;
	if (VARS[0]->Type == VAR_POINTER) {
		ObjectCore_Set_Object_Internal_Data(Object, VARS[0], VARS[1]);
	}
	else if (VARS[1]->Type == VAR_POINTER) {
		tmp = MT2D_Object_Create_Var_Int("Tmp", ObjectCore_VAR_Get_Integer(Object,VARS[1]));
		MT2D_Object_VAR_SetVar(VARS[0],tmp);
		//del tmp
	}
	else {
		MT2D_Object_VAR_SetVar(VARS[0], VARS[1]);
	}
}

//Object *, MT2D_VAR *, int
/**
	Main Object
**/
void ObjectCore_JumpToStateIfRandom(Object *object, MT2D_VAR **JUMPTO_CHANCE_MAX, int TreeVars) {
	int i = 0;
	int MAX = ObjectCore_VAR_Get_Integer(object,JUMPTO_CHANCE_MAX[2]);
	int SEL_RANGE = ObjectCore_VAR_Get_Integer(object,JUMPTO_CHANCE_MAX[1]);
	if (rand() % MAX <= SEL_RANGE) {
		while (i < object->States_Count) {
			if (strcmp(object->State[i]->Name, (char*)JUMPTO_CHANCE_MAX[0]->Data) == 0) {
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
	[1] = the max value to jump
	[2] = the var where the check number is stored
*/
void ObjectCore_JumpToStateIfVarLowerEq(Object *object, MT2D_VAR **Vars, int TreeVars) {
	int i = 0;
	int MAX = ObjectCore_VAR_Get_Integer(object,Vars[1]);
	int Now = ObjectCore_VAR_Get_Integer(object,Vars[2]);
	if (Now <= MAX) {
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
void ObjectCore_JumpToStateIfVarEq(Object *object, MT2D_VAR **Vars, int TreeVars) {
	int i = 0;
	int MAX = ObjectCore_VAR_Get_Integer(object, Vars[1]);
	int Now = ObjectCore_VAR_Get_Integer(object, Vars[2]);
	if (object->ActualState == 2) {
		i = 0;
	}
	if (Now == MAX) {
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



void ObjectCore_PlaySound(Object *object,MT2D_VAR **SoundName) {
	if (SoundName[0]->Name) {
		MT2D_Play_Audio(SoundName[0]->Name);
	}
}

void ObjectCore_PlayMusic(Object *object,MT2D_VAR **MusicName) {
	if (MusicName[0]->Name) {
		MT2D_Play_Music(MusicName[0]->Name);
	}
}


void ObjectCore_TeleportToCoord(Object *object, MT2D_VAR **X_Y, int TwoVars) {
	int X = ObjectCore_VAR_Get_Integer(object,X_Y[0]);
	int Y = ObjectCore_VAR_Get_Integer(object,X_Y[1]);
	object->SpacePosition.X = X;
	object->SpacePosition.Y = Y;
}

void ObjectCore_Move(Object *object) {
	object->SpacePosition.X += object->Aceleration.X;
	object->SpacePosition.Y += object->Aceleration.Y;
}
/**
	Vars[0] = Pos X
	Vars[1] = Pos Y
	Vars[2] = Acel X
	Vars[3] = Acel Y
**/
void ObjectCore_CreateObject(Object *Caller, Object *NewModel,MT2D_VAR **Vars) {
	Object *NewObject;
	int X[2] = { ObjectCore_VAR_Get_Integer(Caller,Vars[0]),ObjectCore_VAR_Get_Integer(Caller,Vars[2]) };
	int Y[2] = { ObjectCore_VAR_Get_Integer(Caller,Vars[1]),ObjectCore_VAR_Get_Integer(Caller,Vars[3]) };
	NewObject = Object_Create(NewModel->Solid, NewModel->RenderOnly, NewModel->Size.X, NewModel->Size.X,X[0],Y[0],NewModel->State,NewModel->States_Count);
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
	It'll only mark that this object wants to be deleted, so you can manually do that or let
	the Scenecontrol to do that automatically.
**/
void ObjectCore_RemoveObject(Object *Caller) {
	Caller->CanDelete = true;
}

/**
	WhatShouldIdo:
	 0 = nothing
	 1 = damage touched object
	 2 = reflect aceleration
	 3 = nothing
	 4 = mark caller as touched
	 4 = damage touched object and mark caller as touched
	 5 = reflect aceleration and mark caller as touched
	 6 = mark both objects as touched.
	Vars:
	[0] = Damage Amount
	[1] = the name of the Var from Caller that should be marked as touched
	[2] = the name of the Var from the touched object that should be marked as touched

**/
void Object_Touched(Object *Caller, MT2D_VAR **Vars, int WhatShouldIdo) {

}

/**
	This function will check if a hit happened in every object inside the objectscene
	pointed by this object.
	VARS[0] = State to jump if object hit A solid object
	VARS[1] = Amount of damage in case (SHOULD NOT BE A POINTER TYPE)
	VARS[2] = The index of the var that's used to represent the health on the other object
**/
void Object_GotoState_IfHit(Object *Caller, MT2D_VAR **Vars, int State_AmDmg_HitHealthIndex) {
	int i = 0;
	int ObjVarInded = 0;
	while (i < Caller->MyScene->Count) {
		if (Caller->MyScene->ObjectGroup[i]->Solid == true) {
			if (Caller->SpacePosition.X <= Caller->MyScene->ObjectGroup[i]->SpacePosition.X + Caller->MyScene->ObjectGroup[i]->Size.X){
				if (Caller->SpacePosition.X + Caller->Size.X >= Caller->MyScene->ObjectGroup[i]->SpacePosition.X) {
					if (Caller->SpacePosition.Y <= Caller->MyScene->ObjectGroup[i]->SpacePosition.Y + Caller->MyScene->ObjectGroup[i]->Size.Y) {
						if (Caller->SpacePosition.Y + Caller->Size.Y >= Caller->MyScene->ObjectGroup[i]->SpacePosition.Y) {
							//there's indeed a hit
							//so lets do some dmg in this object.
							Vars[2]->Data = Caller->MyScene->ObjectGroup[i];
							ObjVarInded = ObjectCore_Get_Object_VarIndex(Vars[2]);
							Vars[2]->Data = 0; 
							MT2D_Object_SUB(&Caller->MyScene->ObjectGroup[i]->User_Vars[ObjVarInded], Vars[1]);
							i = Caller->MyScene->Count;
							ObjectCore_Object_SetState(Caller, Vars);
						}
					}
				}
			}
		}
		i++;
	}
}
/**
	VARS[0] the number to be increased
	VARS[1] the amount to increase
**/
void ObjectCore_VAR_ADD(Object *object, MT2D_VAR **VARS, int Two) {
	int i = 0;
	if (VARS[0]->Type == VAR_POINTER) {
		MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", ObjectCore_VAR_Get_Integer(object, VARS[0]));
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp2 = MT2D_Object_Create_Var_Int("Tmp2", ObjectCore_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_ADD(Tmp, Tmp2);
			ObjectCore_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
		else {
			MT2D_Object_ADD(Tmp, VARS[1]);
			ObjectCore_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
	}
	else {
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", ObjectCore_VAR_Get_Integer(object, VARS[1]));
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
void ObjectCore_VAR_SUB(Object *object, MT2D_VAR **VARS, int Two) {
	int i = 0;
	if (VARS[0]->Type == VAR_POINTER) {
		MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", ObjectCore_VAR_Get_Integer(object, VARS[0]));
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp2 = MT2D_Object_Create_Var_Int("Tmp2", ObjectCore_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_SUB(Tmp, Tmp2);
			ObjectCore_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
		else {
			MT2D_Object_SUB(Tmp, VARS[1]);
			ObjectCore_VAR_Set_Integer(object, VARS[0], Tmp);
			//Del Tmp
		}
	}
	else {
		if (VARS[1]->Type == VAR_POINTER) {
			MT2D_VAR *Tmp = MT2D_Object_Create_Var_Int("Tmp", ObjectCore_VAR_Get_Integer(object, VARS[1]));
			MT2D_Object_SUB(VARS[0], Tmp);
			//Del Tmp
		}
		else {
			MT2D_Object_SUB(VARS[0], VARS[1]);
		}
	}
}

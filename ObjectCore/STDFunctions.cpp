#include <MT2D/ObjectCore/Functions.h>
#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <cstring>
#include <cstdlib>

/**
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
	}
	return Return;
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

void ObjectCore_Object_SetState(Object *Object, MT2D_VAR **StateName) {
	int i = 0;
	while (i < Object->States_Count) {
		if (strcmp(Object->State[i]->Name, (char*)StateName[0]->Data) == 0) {
			Object->ActualState = i;
			Object->ActualFrame = 0;
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
	int i = 0;
	MT2D_VAR *One = MT2D_Object_Create_Var_Char("One", 1);
	while (i < object->User_Vars_Count) {
		if (strcmp(object->User_Vars[i].Name, VAR[0]->Name) == 0) {
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
				object->ActualFrame = 0;
				object->ActualFrameWait = object->State[i]->WaitSprites[0];
				i = object->States_Count;
			}
			i++;
		}
	}
}


void ObjectCore_PlaySound(MT2D_VAR **SoundName) {
	if (SoundName[0]->Name) {
		MT2D_Play_Audio(SoundName[0]->Name);
	}
}

void ObjectCore_PlayMusic(MT2D_VAR **MusicName) {
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
	Object *NewObject = (Object*)malloc(sizeof(Object));
	int X[2] = { ObjectCore_VAR_Get_Integer(Caller,Vars[0]),ObjectCore_VAR_Get_Integer(Caller,Vars[2]) };
	int Y[2] = { ObjectCore_VAR_Get_Integer(Caller,Vars[1]),ObjectCore_VAR_Get_Integer(Caller,Vars[3]) };
	Object_Create(NewModel->Solid, NewModel->RenderOnly, NewModel->Size.X, NewModel->Size.X,X[0],Y[0],NewModel->State,NewModel->States_Count);
	NewObject->Aceleration.X = X[1];
	NewObject->Aceleration.Y = Y[1];
	MT2D_VAR *NewVars = (MT2D_VAR*)malloc(NewModel->User_Vars_Count * sizeof(MT2D_VAR));
	for (int i = 0; i < NewModel->User_Vars_Count; i++) {
		NewVars[i] = *MT2D_VAR_CLONE(&NewModel->User_Vars[i]);
	}
	NewObject->User_Vars = NewVars;
	NewObject->User_Vars_Count = NewModel->User_Vars_Count;
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
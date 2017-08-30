/* Object.cpp created by Lucas Zimerman Fraulob

*/
#ifndef __ANDROID__
#include <MT2D/MT2D.h>
#include <MT2D/Objectcore/Object.h>
#include <MT2D/Objectcore/Sprites.h>
#else
#include "../MT2D.h"
#include "../Objectcore/Object.h"
#include "../Objectcore/Sprites.h"
#endif
#include <stdlib.h>
#include <stdio.h>
void Object_Render(Object *obj) {
	if (obj->Enabled) {//RENDER ONLY IF THE OBJECT IS ENABLED
		//Decrement the frame counter
		//Check if the frame tic has expired, if yes select the next frame
		//Render the frame
		Sprite_Render_on_Window(obj->State[obj->ActualState]->Sprites[obj->ActualFrame],DISPLAY_WINDOW1, obj->SpacePosition.X, obj->SpacePosition.Y);
	}
}

Object *Object_Create(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY,ObjectState **_States, int States_Count ) {
	Object *Obj;
	Obj = (Object*)malloc(sizeof(Object));

	Obj->Solid = Solid;
	Obj->RenderOnly = RenderOnly;
	Obj->Size.X = sizeX;
	Obj->Size.Y = sizeY;
	Obj->SpacePosition.X = PosX;
	Obj->SpacePosition.Y = PosY;
	Obj->ActualFrame = 0;
	Obj->ActualFrameWait = _States[0]->WaitSprites[0];
	Obj->ActualState = 0;
	Obj->State = _States;
	Obj->States_Count = States_Count;
	Obj->Enabled = true;
	Obj->CanDelete = false;
	Obj->MyScene = 0;
	Obj->Aceleration.X = 0;
	Obj->Aceleration.Y = 0;
	Obj->User_Vars_Count = 0;
	Obj->User_Vars = 0;
	return Obj;
}

void Object_Private_Run_Function(Object *O) {
	if (O->State[O->ActualState]->Functions == 0) {

	}
 	else if (O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj) {
		O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj(O);
	}
	else if (O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj_Vars) {
		O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj_Vars(
			O,
			O->State[O->ActualState]->Functions[O->ActualFrame]->TempVars
		);
	} 
	else if (O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj_Vars) {
		O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj_Vars(
			O,
			O->State[O->ActualState]->Functions[O->ActualFrame]->TempVars
		);
	}
	else if (O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj_Obj_Vars) {
		O->State[O->ActualState]->Functions[O->ActualFrame]->_Obj_Obj_Vars(
			O,
			O->State[O->ActualState]->Functions[O->ActualFrame]->TempObj,
			O->State[O->ActualState]->Functions[O->ActualFrame]->TempVars
		);
	}
}

/**
	Consume the next state of the Object
**/
void Object_Goto_NextStep(Object *O) {
	if (O->ActualFrameWait > 0) {
		O->ActualFrameWait--;
	}
	else {
		while (O->ActualFrameWait == 0) {
#ifdef DEBUG_OBJECTCORE
			if (O->ActualState == 3 && O->ActualFrame == 5) {
				O = O;
			}
			if(O->User_Vars != 0){
				printf("function Object tank Timeout %d State [%s] Frame %d\n", *(int*)O->User_Vars[0].Data, (O->ActualState == 0 ? "spawn" : (O->ActualState == 1 ? "dead" : (O->ActualState == 2 ? "check fire" : "fire"))), O->ActualFrame);
			}
			else {
				printf("function Object projectile State [%s] Frame %d pos X %d speed X %d\n",(O->ActualState == 0 ? "spawn" : "dead"), O->ActualFrame,O->SpacePosition.X,O->Aceleration.X);
			}
#endif
			Object_Private_Run_Function(O);
			O->ActualFrame++;
			if (O->ActualFrame < O->State[O->ActualState]->Count) {
				O->ActualFrameWait = O->State[O->ActualState]->WaitSprites[O->ActualFrame];
			}
			else {
				//no more frames in the parsed state, so we restart it
				O->ActualFrame = 0;
				O->ActualFrameWait = O->State[O->ActualState]->WaitSprites[0];
			}
		}
	}
}


ObjectScene *ObjectScene_CreateEmpty() {
	ObjectScene *Scene = (ObjectScene*)malloc(sizeof(ObjectScene));
	Scene->Count = 0;
	Scene->ObjectGroup = 0;
	return Scene;
}
void ObjectScene_Add(ObjectScene *Scene, Object *New) {
	if (Scene->Count) {
		Scene->ObjectGroup = (Object**)realloc(Scene->ObjectGroup, (Scene->Count + 1)* sizeof(Object*));
	}
	else {
		Scene->ObjectGroup = (Object**)malloc((Scene->Count + 1) * sizeof(Object*));
	}
	Scene->Count++;
	Scene->ObjectGroup[Scene->Count - 1] = New;
	New->MyScene = Scene;
}
void ObjectScene_Render(ObjectScene *Scene) {
	for (int i = 0; i < Scene->Count; i++) {
		Object_Render(Scene->ObjectGroup[i]);
	}
}
void ObjectScene_Goto_NextSteps(ObjectScene *Scene) {
	int i = 0;
	int j = 0;
	for (; i < Scene->Count; i++) {
		Object_Goto_NextStep(Scene->ObjectGroup[i]);
//		printf("DEBUG: object %d state %d frame %d candelete %d\n",i, Scene->ObjectGroup[i]->ActualState, Scene->ObjectGroup[i]->ActualFrame, Scene->ObjectGroup[i]->CanDelete);
		if (Scene->ObjectGroup[i]->CanDelete == true) {
			for (j = i; j < Scene->Count -1; j++) {
				Scene->ObjectGroup[j] = Scene->ObjectGroup[j + 1];
			}
			Scene->Count--;
			Scene->ObjectGroup = (Object**)realloc(Scene->ObjectGroup, (Scene->Count) * sizeof(Object*));
			i--;
		}
	}
}

void Object_Delete(Object *Me) {
	int i = 0;
	if (Me != 0) {
		if (Me->User_Vars_Count) {
			for (i = 0; i < Me->User_Vars_Count; i++) {
				free(Me->User_Vars[i].Data);
				free(Me->User_Vars[i].Name);
			}
		}
		free(Me);
	}
	//We do not delete the State because it's a shared memory data that's shared by all the objects.
	//you must manually delete the State that's shared by all those objects.
}
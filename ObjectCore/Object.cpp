/* Object.cpp created by Lucas Zimerman Fraulob

*/
#include <MT2D/MT2D.h>
#include <MT2D/Objectcore/Object.h>
#include <MT2D/Objectcore/Sprites.h>
#include <stdlib.h>

void Object_Render(Object *obj) {
	if (obj->Enabled) {//RENDER ONLY IF THE OBJECT IS ENABLED
		//Decrement the frame counter
		//Check if the frame tic has expired, if yes select the next frame
		//Render the frame
		Sprite_Render_on_Window(&obj->State[obj->ActualState].Sprites[obj->ActualFrame],DISPLAY_WINDOW1, obj->SpacePosition.X, obj->SpacePosition.Y);
	}
}
/**
void Object_Render(ObjectSceneControl *scene) {//Not Implemented
	int i = 0;
	for (i = 0; i < scene->SimpleCnt; i++) {
		Object_Render(&scene->ArraySimple[i]);
	}
	for (i = 0; i < scene->ComplexCnt; i++) {
		Object_Render(&scene->ArrayComplex[i]);
	}
	for (i = 0; i < scene->ProjectileCnt; i++) {
		Object_Render(&scene->ArrayProjectiles[i]);
	}
}
*/
/*void Object_Move(ObjectComplex *obj){
	bool Move_Alloyed = false;
	if (obj->Active) {// Only move if the object is activated
		switch (obj->DataCore.State) {//check if in the actual state the object can move
		case COMPLEX_ACTIVE_STATE:
			Move_Alloyed = true;
			break;
		case COMPLEX_ATTACK_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack1;
			break;
		case COMPLEX_ATTACK2_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack2;
			break;
		case COMPLEX_ATTACK3_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack3;
			break;
		case COMPLEX_ATTACK4_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack4;
			break;
		case COMPLEX_PAIN_STATE:
			Move_Alloyed = obj->CanMoveWhilePain;
			break;
		case COMPLEX_IDLE_STATE:
			// Always false
			break;
		case COMPLEX_DEATH_STATE:
			//Always false
			break;
		}
		if (Move_Alloyed) {
			obj->DataCore.SpacePosition.X += obj->Aceleration.X;
			obj->DataCore.SpacePosition.Y += obj->Aceleration.Y;
		}
	}
}

void Object_ForcePain(ObjectComplex *obj){
	if (obj->Active) {
		if (obj->Health > 0) {// no pain if no health
			obj->DataCore.State = COMPLEX_PAIN_STATE;
			obj->DataCore.Frame = 0;
			obj->DataCore.FrameCnt = obj->PainFramesDuration[0];
		}
	}
}
void Object_Kill(ObjectComplex *obj) {
	if (obj->Active) {
		if (obj->Health > 0) {// do not kill what's already dead
			obj->DataCore.State = COMPLEX_DEATH_STATE;
			obj->DataCore.Frame = 0;
			obj->DataCore.FrameCnt = obj->DeathFramesDuration[0];
		}
	}
}

void Object_Remove_FromMemory(ObjectComplex *obj) {//Not Implemented

}

void Object_Remove_FromMemory(ObjectSimple *obj) {//Not Implemented

}
*/
/*
void Object_ConsumeNext(Object *obj) {//Not Implemented

}
*/
Object *Object_Create(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY,MT2D_OBJECT_STATE *_States, int States_Count ) {
	Object *Obj;
	Obj = (Object*)malloc(sizeof(Object));

	Obj->Solid = Solid;
	Obj->RenderOnly = RenderOnly;
	Obj->Size.X = sizeX;
	Obj->Size.Y = sizeY;
	Obj->SpacePosition.X = PosX;
	Obj->SpacePosition.Y = PosY;
	Obj->ActualFrame = 0;
	Obj->ActualFrameWait = _States->WaitSprites[0];
	Obj->ActualState = 0;
	Obj->State = _States;
	Obj->States_Count = States_Count;
	Obj->Enabled = true;
	return Obj;
}

void Object_Private_Run_Function(Object *O) {
	if (O->State[O->ActualState].Functions[O->ActualFrame].vPtr1) {
		O->State[O->ActualState].Functions[O->ActualFrame].vPtr1(O);
	}else if (O->State[O->ActualState].Functions[O->ActualFrame].vPtr2) {
		O->State[O->ActualState].Functions[O->ActualFrame].vPtr2(
			O->State[O->ActualState].Functions[O->ActualFrame].ReturnVAR_ID,
			O
		);
	}
	else if (O->State[O->ActualState].Functions[O->ActualFrame].vPtr3) {
		O->State[O->ActualState].Functions[O->ActualFrame].vPtr3(
			O,
			O->State[O->ActualState].Functions[O->ActualFrame].TempVars
		);
	}
	else if (O->State[O->ActualState].Functions[O->ActualFrame].vPtr4) {
		O->State[O->ActualState].Functions[O->ActualFrame].vPtr4(
			O,
			O->State[O->ActualState].Functions[O->ActualFrame].TempVars,
			O->State[O->ActualState].Functions[O->ActualFrame].TotalVars
		);
	}
	else if (O->State[O->ActualState].Functions[O->ActualFrame].vPtr5) {
		O->State[O->ActualState].Functions[O->ActualFrame].vPtr5(
			O->State[O->ActualState].Functions[O->ActualFrame].ReturnVAR_ID,
			O,
			O->State[O->ActualState].Functions[O->ActualFrame].TempVars,
			O->State[O->ActualState].Functions[O->ActualFrame].TotalVars
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
			O->ActualFrame++;
			if (O->ActualFrame < O->State[O->ActualState].Count) {
				O->ActualFrameWait = O->State[O->ActualState].WaitSprites[O->ActualFrame];
				Object_Private_Run_Function(O);
			}
			else {
				//no more frames in the parsed state, so we restart it
				O->ActualFrame;
				O->ActualFrameWait = O->State[O->ActualState].WaitSprites[0];
			}
		}
	}
}

#ifndef __ANDROID__
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/State.h>
#else
#include "../ObjectCore/Sprites.h"
#include "../ObjectCore/State.h"
#endif
#include <cstdlib>

/**
	Create an empty state
**/
ObjectState *MT2D_OBJECT_CREATE_STATE(char *StateName)
{
	ObjectState * State = (ObjectState*)malloc(sizeof(ObjectState));
	State->Sprites = 0;
	State->Count = 0;
	State->Functions = 0;
	State->Name = StateName;
	State->WaitSprites = 0;
	return State;
}


void MT2D_OBJECT_ADD_STATE(ObjectState *State, Sprite *sprite, int Wait, Cscript *Function) {
	if (State) {
		if (State->Count == 0) {
			State->Sprites = (Sprite**)malloc(sizeof(Sprite*));
			State->Sprites[State->Count] = sprite;
			State->Functions = (Cscript**)malloc(sizeof(Cscript*));
			State->Functions[State->Count] = Function;
			State->WaitSprites = (int*)malloc(sizeof(int));
			State->WaitSprites[State->Count] = Wait;
			State->Count++;
		}
		else {
			State->Count++;
			State->Sprites = (Sprite**)realloc(State->Sprites, State->Count*sizeof(Sprite*));
			State->Functions = (Cscript**)realloc(State->Functions, State->Count*sizeof(Cscript*));
			State->WaitSprites = (int*)realloc(State->WaitSprites, State->Count*sizeof(int));
			State->Count--;
			State->Functions[State->Count] = Function;
			State->Sprites[State->Count] = sprite;
			State->WaitSprites[State->Count] = Wait;
			State->Count++;
		}
	}
}

void MT2D_ObjectState_ADD(ObjectState *State, Sprite *sprite, int Wait, Cscript *Script) {
	if (State->Count == 0) {
		State->Functions = (Cscript**)malloc(sizeof(Cscript*));
		State->Sprites = (Sprite**)malloc(sizeof(Sprite*));
		State->WaitSprites = (int*)malloc(sizeof(int));
	}
	else {
		State->Functions = (Cscript**)realloc(State->Functions,(State->Count+1) *sizeof(Cscript*));
		State->Sprites = (Sprite**)realloc(State->Sprites, (State->Count + 1) * sizeof(Sprite*));
		State->WaitSprites = (int*)realloc(State->WaitSprites, (State->Count + 1) * sizeof(int));
	}
	State->Functions[State->Count] = Script;
	State->Sprites[State->Count] = sprite;
	State->WaitSprites[State->Count] = Wait;
	State->Count++;
}

void MT2D_ObjectState_Delete(ObjectState *Me) {
	int i = 0;
	if (Me) {
		free(Me->WaitSprites);
		for (int i = 0; i < Me->Count; i++) {
			Cscript_RemoveFromMemory(Me->Functions[i]);
			Sprite_Delete(Me->Sprites[i]);
		}
	}
	free(Me);
}
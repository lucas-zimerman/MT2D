#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/State.h>
#include <cstdlib>

/**
	Create an empty state
**/
MT2D_OBJECT_STATE *MT2D_OBJECT_CREATE_STATE(char *StateName)
{
	MT2D_OBJECT_STATE * State = (MT2D_OBJECT_STATE*)malloc(sizeof(MT2D_OBJECT_STATE));
	State->Sprites = 0;
	State->Count = 0;
	State->Functions = 0;
	State->Name = StateName;
	State->WaitSprites = 0;
	return State;
}


void MT2D_OBJECT_ADD_STATE(MT2D_OBJECT_STATE *State, Sprite *sprite, int Wait, MT2D_OBJECT_FUNCTION *Function) {
	if (State) {
		if (State->Count == 0) {
			State->Sprites = (Sprite*)malloc(sizeof(Sprite));
			State->Sprites[State->Count] = *sprite;
			State->Functions = (MT2D_OBJECT_FUNCTION*)malloc(sizeof(MT2D_OBJECT_FUNCTION));
			State->Functions[State->Count] = *Function;
			State->WaitSprites = (int*)malloc(sizeof(int));
			State->WaitSprites[State->Count] = Wait;
			State->Count++;
		}
		else {
			State->Count++;
			State->Sprites = (Sprite*)realloc(State->Sprites, State->Count*sizeof(Sprite));
			State->Functions = (MT2D_OBJECT_FUNCTION*)realloc(State->Functions, State->Count*sizeof(MT2D_OBJECT_FUNCTION));
			State->WaitSprites = (int*)realloc(State->WaitSprites, State->Count*sizeof(int));
			State->Count--;
			State->Functions[State->Count] = *Function;
			State->Sprites[State->Count] = *sprite;
			State->WaitSprites[State->Count] = Wait;
			State->Count++;
		}
	}
}
#ifndef OBJ_STATE
#define OBJ_STATE
// State.h , created by: Lucas Zimerman Fraulob
//
//A state contains a set of sprites, a function call and a timer.
//A label contains a set of states.

#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Functions.h>
struct MT2D_OBJECT_FUNCTION;

struct MT2D_OBJECT_STATE
{
	Sprite *Sprites;// a set of sprites
	int *WaitSprites;// the time that you need to wait for each sprite
	int Count;
	MT2D_OBJECT_FUNCTION *Functions;//the functions for each sprite
	char *Name; //name from this state.
};

MT2D_OBJECT_STATE *MT2D_OBJECT_CREATE_STATE(char *StateName);
void MT2D_OBJECT_ADD_STATE(MT2D_OBJECT_STATE *State, Sprite *sprite, int Wait, MT2D_OBJECT_FUNCTION *Function);

#endif
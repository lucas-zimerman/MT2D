#ifndef OBJ_STATE
#define OBJ_STATE
// State.h , created by: Lucas Zimerman Fraulob
//
//A state contains a set of sprites, a function call and a timer.
//A label contains a set of states.

#include "Sprites.h"
#include "Vars.h"

struct MT2D_OBJECT_STATE
{
	Sprite *Sprites;// a set of sprites
	int Wait;// the time that you need to wait for each sprite
	int ActualSprite;
	int Count;
	MT2D_VAR *function_args;
};

struct MT2D_OBJECT_STATE_LABEL
{
	MT2D_OBJECT_STATE *States;
	int ActualState;
	int Count;
	MT2D_OBJECT_STATE_LABEL *NextStateLabel; // the next state that the object should go when we are out of states.
};

#endif
#ifndef OBJ_STATE
#define OBJ_STATE
/* State.h , created by: Lucas Zimerman Fraulob

	The state is an important part of ObjectCore because it allows the programmer
	to add "BRAIN" to the object, allowing it to follow a step by step order of 
	showing sprites and executing Cscript calls.

	This struct is shared by the objects and isn't replicated for each object
	(since all its data are "const" you can even use it in threads).



  A state contains a set of sprites, a function call and a timer.
  A label contains a set of states.

*/

#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/Cscript.h>
struct Cscript;

struct ObjectState
{
	Sprite *Sprites;// a set of sprites
	int *WaitSprites;// the time that you need to wait for each sprite
	int Count;
	Cscript *Functions;//the functions for each sprite
	char *Name; //name from this state.
};

ObjectState *MT2D_OBJECT_CREATE_STATE(char *StateName);
void MT2D_OBJECT_ADD_STATE(ObjectState *State, Sprite *sprite, int Wait, Cscript *Function);

/**
	Allows you a better control from your struct.
**/


void MT2D_ObjectState_ADD(ObjectState *State, Sprite *sprite, int Wait, Cscript *Script);

#endif
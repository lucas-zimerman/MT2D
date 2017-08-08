#ifndef OBJECT2_H
#define OBJECT2_H

#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/ObjectCore/Vars.h>

struct MT2D_OBJECT_STATE;

struct Object{
	bool RenderOnly;
	bool Solid;
	Coord SpacePosition;
	Coord Size;
	Coord Aceleration;
	Object *Target;
	MT2D_OBJECT_STATE *State; /*The first state is the initial state where the object is going to be initiated*/
	int States_Count;
	int ActualState;
	int ActualFrame;// the actual frame that is being showed by the object, the spriteblock to be used can be determined by the State.
	int ActualFrameWait; // count how many tics the actual frame has to be showed, start with x and ends with 0; -1 means restart.
	bool Enabled;
	MT2D_VAR *User_Vars;
	int User_Vars_Count;
};



void Object_Render(Object *obj);
Object *Object_Create(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY, MT2D_OBJECT_STATE *_States, int States_Count);
void Object_Goto_NextStep(Object *O);
#endif
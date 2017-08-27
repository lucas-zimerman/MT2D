#ifndef OBJECT2_H
#define OBJECT2_H

#ifdef _DEBUG
//#define DEBUG_OBJECTCORE
#endif

#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/ObjectCore/Vars.h>

struct ObjectState;
struct ObjectScene;

struct Object{
	bool RenderOnly;
	bool Solid;
	Coord SpacePosition;
	Coord Size;
	Coord Aceleration;
	Object *Target;
	ObjectScene *MyScene;
	ObjectState **State; /*The first state is the initial state where the object is going to be initiated*/
	int States_Count;
	int ActualState;
	int ActualFrame;// the actual frame that is being showed by the object, the spriteblock to be used can be determined by the State.
	int ActualFrameWait; // count how many tics the actual frame has to be showed, start with x and ends with 0; -1 means restart.
	bool Enabled;
	bool CanDelete; //mark if the object or other thing requested this object to be deleted. better used with dynamic objects that are created by the object code
	MT2D_VAR *User_Vars;
	int User_Vars_Count;
};

struct ObjectScene {
	Object **ObjectGroup;
	int Count;
};

void Object_Render(Object *obj);
Object *Object_Create(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY, ObjectState **_States, int States_Count);
void Object_Goto_NextStep(Object *O);


ObjectScene *ObjectScene_CreateEmpty();
void ObjectScene_Add(ObjectScene *Scene, Object *New);
void ObjectScene_Render(ObjectScene *Scene);
void ObjectScene_Goto_NextSteps(ObjectScene *Scene);
void Object_Delete(Object *Me);
#endif
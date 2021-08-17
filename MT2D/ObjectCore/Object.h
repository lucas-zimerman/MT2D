#ifndef OBJECT2_H
#define OBJECT2_H

#ifdef _DEBUG
//#define DEBUG_OBJECTCORE
#endif

#ifndef __ANDROID__
#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Coordinates.h>
#include <MT2D/ObjectCore/Vars.h>
#else
#include "../ObjectCore/State.h"
#include "../ObjectCore/Coordinates.h"
#include "../ObjectCore/Vars.h"
#endif

#define Object_PosX_Var_Index 0
#define Object_PosY_Var_Index 1
#define Object_SizeX_Var_Index 2
#define Object_SizeY_Var_Index 3
#define Object_AcelX_Var_Index 4
#define Object_AcelY_Var_Index 5
#define Object_First_User_Var_Index 6

struct ObjectState;
struct ObjectScene;

struct Object{
	bool RenderOnly;
	bool Solid;
//	Coord SpacePosition;
//	Coord Size;
//	Coord Aceleration;
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
void Object_Delete(Object *Me, bool deleteStates);
void Object_Add_Variable(Object* object, MT2D_VAR* variable);
void Object_Add_Variables(Object* object, MT2D_VAR* variables, int total);

#define ObjectPosXRef(object) ((int*)((Object*)object)->User_Vars[Object_PosX_Var_Index].Data) 
#define ObjectPosYRef(object) ((int*)((Object*)object)->User_Vars[Object_PosY_Var_Index].Data) 
#define ObjectAcelXRef(object) ((int*)((Object*)object)->User_Vars[Object_AcelX_Var_Index].Data) 
#define ObjectAcelYRef(object) ((int*)((Object*)object)->User_Vars[Object_AcelY_Var_Index].Data) 
#define ObjectSizeXRef(object) ((int*)((Object*)object)->User_Vars[Object_SizeX_Var_Index].Data) 
#define ObjectSizeYRef(object) ((int*)((Object*)object)->User_Vars[Object_SizeY_Var_Index].Data) 

#define ObjectPosX(object)  (*(int*)(((Object*)object)->User_Vars[Object_PosX_Var_Index].Data)) 
#define ObjectPosY(object)  (*(int*)((Object*)object)->User_Vars[Object_PosY_Var_Index].Data) 
#define ObjectAcelX(object) (*(int*)((Object*)object)->User_Vars[Object_AcelX_Var_Index].Data) 
#define ObjectAcelY(object) (*(int*)((Object*)object)->User_Vars[Object_AcelY_Var_Index].Data) 
#define ObjectSizeX(object) (*(int*)((Object*)object)->User_Vars[Object_SizeX_Var_Index].Data) 
#define ObjectSizeY(object) (*(int*)((Object*)object)->User_Vars[Object_SizeY_Var_Index].Data) 

#endif
#ifndef MT2D_OBJFUN_H
#define MT2D_OBJFUN_H

#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/Vars.h>

struct Object;

/**
	This function only store pointers, no malloc involved.
	So in order to clean the memory only free the MT2D_OBECT_FUNCTION
	and the other pointers you free externally by their original pointers.
	f>>>>>>>>>>>>>>>>>>>>>>>>>
	The TempVars are not going to be pointed directly to the vars itself but rather 
	to a string that has the same name from a var inside the object, so the function
	can be reused by other objects without affecting the logic.
**/
struct MT2D_OBJECT_FUNCTION
{
	void(*vPtr1)(Object *);//main object
	void(*vPtr2)(char *, Object *); // return (name of the object VAR), main object
	void(*vPtr3)(Object *, MT2D_VAR **); //single var
	void(*vPtr4)(Object *, MT2D_VAR **, int );//array of vars
	void(*vPtr5)(char *, Object *, MT2D_VAR **, int);//return (name of the object VAR), main object, array of vars
	void(*vPtr6)(Object*, Object*,MT2D_VAR**);//main object, other objects,selected object, array of vars
	char *ReturnVAR_ID;
	MT2D_VAR **TempVars;
	Object *TempObj;
	int TotalVars;
};

MT2D_OBJECT_FUNCTION *Object_Function_Create_New1(void(*Function)(Object *));
MT2D_OBJECT_FUNCTION *Object_Function_Create_New2(char *ReturnVAR_ID, void(*Function)(char *, Object *));
MT2D_OBJECT_FUNCTION *Object_Function_Create_New3(MT2D_VAR **ObjVarPointer, void(*Function)(Object *, MT2D_VAR **));
MT2D_OBJECT_FUNCTION *Object_Function_Create_New4(MT2D_VAR **ObjVarPointers, int TotalPointers, void(*Function)(Object *, MT2D_VAR **, int));
MT2D_OBJECT_FUNCTION *Object_Function_Create_New5(char *ReturnVAR_ID, MT2D_VAR *ObjVarPointers, int TotalPointers, void(*Function)(char *, Object *, MT2D_VAR **, int));

void Object_Function_Call(Object *Caller, MT2D_OBJECT_FUNCTION *Function);
/*
	Supported Operations: + - * / ^
*/

/* ================== OBJECT AI  ===================== */
/*void MT2D_OF_CHASE(Object *O); //moves in direction of a target
void MT2D_OF_MOVE(Object *O);  //moves in direction of the aceleration data.
void MT2D_OF_CLEAR_TARGET(Object *O);
void MT2D_OF_SET_TARGET(Object *O, Object *Target);
void MT2D_OF_DIE(Object *O);   //set this object in death state and set the health to zero.
void MT2D_OF_KILL_TARGET(Object *o);
void MT2D_OF_LOOK(Object *O,Object *ListTargets,MT2D_VAR *SizeofList);// Try to find the first target alive on the list.
void MT2D_OF_LOOK(Object *O,Object *ListTargets,MT2D_VAR *SizeofList,MT2D_VAR *MinRange,MT2D_VAR *MaxRange);
void MT2D_OF_TELEPORT(Object *O,MT2D_VAR *X, MT2D_VAR *Y);
void MT2D_OF_LOSE_TARGET(Object *O, MT2D_VAR *Chance, MT2D_VAR * MinDistance);// chance from 0 to 255, min distance to test it (pass null to test at any range)
*/
/* ================== OBJECT ATTACK  ===================== */

/* ================== OBJECT SOUND  ===================== */
//void MT2D_OF_PLAY_SOUND(MT2D_VAR *SndName);
//void MT2D_OF_PLAY_MUSIC(MT2D_VAR *MusicName);
/* ================== OBJECT STATE  ===================== */
//void MT2D_OF_JUMPSTATE_CHANCE(Object *o, MT2D_VAR *StateLabelName, MT2D_VAR *JumpChance);//select a random number from 0 to 255, if the number is lower than JumpChance so it'll perform a jump.
//void MT2D_OF_JUMP_IF(Object *o, MT2D_VAR *StateLabelName, MT2D_VAR *ConditionOne,MT2D_VAR *LogicTest, MT2D_AR *ConditionTwo);

#endif
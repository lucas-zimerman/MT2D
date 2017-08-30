/* Cscript, created by Lucas Zimerman Fraulob.
	With Cscript, you can make the ObjectCore much more powerfull, it allows you to 
	add any kind of function into your object, those includes accessing other objects data.
	
	The VARS stored in Cscript must be "const" (by that, their data value shall not be changed, so 
	you cannot reuse the same VAR stored in your object directly but you can create a VAR pointer
	by storing a const pointer to your object's VAR or even other objects.

	Cscript can only interact with const Vars given in his creation or to Vars referenced in Objects,
	you can extend that limit but you'll need to code your own solution to your problem.

	Also, Cscript never returns anything, but it uses the Vars arg to store their data, so you can
	give a pointer to a var where you want the function to store the returned value.

	for saving memory, you can share the same function with many objects because freeing the object
	from the memory are not going to clear the Cscript.
	-----------------------------------------------------------------------------------------------

	This File will only include the minimal struct for using Cscript, you can build your own functions
	and link it on Cscript or you could also use the build in functions in the file STDFunctions


*/

#ifndef MT2D_OBJFUN_H
#define MT2D_OBJFUN_H

#ifndef __ANDROID__
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/Vars.h>
#else
#include "../ObjectCore/Object.h"
#include "../ObjectCore/Vars.h"
#endif

struct Object;// this guy is here because we have a circular include.

/**
	This function only store pointers, no malloc involved.
	So in order to clean the memory only free the MT2D_OBECT_FUNCTION
	and the other pointers you free externally by their original pointers.
	f>>>>>>>>>>>>>>>>>>>>>>>>>
	The TempVars are not going to be pointed directly to the vars itself but rather 
	to a string that has the same name from a var inside the object, so the function
	can be reused by other objects without affecting the logic.
**/
struct Cscript
{
	void(*_void)(); //a function without args
	void(*_Obj)(Object*);//a function with only a single object as arg, normally the caller object
	void(*_Obj_Vars)(Object*, MT2D_VAR **);//a function with only a single object and one or more Vars as arg, normally the caller as an object
	void(*_Obj_Obj_Vars)(Object*, Object*, MT2D_VAR**);//a function with only a single object, one or more objects in the second arg and lastly one or more Vars as arg, normally the caller as the first object
	MT2D_VAR **TempVars;
	Object *TempObj;
};

Cscript *Cscript_Empty();
Cscript *Cscript_New1(void(*Function)());
Cscript *Cscript_New2(void(*Function)(Object *));
Cscript *Cscript_New3(MT2D_VAR **ObjVarPointer, void(*Function)(Object *, MT2D_VAR **));
Cscript *Cscript_New4(Object *ModelObj, MT2D_VAR **ObjVarPointers, void(*Function)(Object *, Object *, MT2D_VAR **));

void Cscript_Execute(Object *Caller, Cscript *Function);
void Cscript_RemoveFromMemory(Cscript *Me);
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
#ifndef MT2D_OBJFUN_H
#define MT2D_OBJFUN_H

#include "Vars.h"
#include "Object.h"


struct MT2D_OBJECT_FUNCTION
{
	void(*vPtr1)();
	void(*vPtr2)(Object *);
	void(*vPtr3)(Object *,MT2D_VAR *);
	void(*vPtr4)(Object *,MT2D_VAR *,MT2D_VAR *);
	void(*vPtr5)(Object *,MT2D_VAR *,MT2D_VAR *,MT2D_VAR *);
	void(*vPtr6)(Object *,MT2D_VAR *,MT2D_VAR *,MT2D_VAR *,MT2D_VAR *);
	void(*vPtr7)(Object *,Object *,MT2D_VAR *);
	void(*vPtr8)(Object *,Object *,MT2D_VAR *,MT2D_VAR *);
	void(*vPtr9)(Object *,Object *,MT2D_VAR *,MT2D_VAR *,MT2D_VAR *);
	int(*iPtr1)(Object *);
	int(*iPtr2)(Object *,Object *);
	int(*iPtr3)(Object *,MT2D_VAR *);
};

/*
	Supported Operations: + - * / ^
*/

/* ================== OBJECT AI  ===================== */
void MT2D_OF_CHASE(Object *O); //moves in direction of a target
void MT2D_OF_MOVE(Object *O);  //moves in direction of the aceleration data.
void MT2D_OF_CLEAR_TARGET(Object *O);
void MT2D_OF_SET_TARGET(Object *O, Object *Target);
void MT2D_OF_DIE(Object *O);   //set this object in death state and set the health to zero.
void MT2D_OF_KILL_TARGET(Object *o);
void MT2D_OF_LOOK(Object *O,Object *ListTargets,MT2D_VAR *SizeofList);// Try to find the first target alive on the list.
void MT2D_OF_LOOK(Object *O,Object *ListTargets,MT2D_VAR *SizeofList,MT2D_VAR *MinRange,MT2D_VAR *MaxRange);
void MT2D_OF_TELEPORT(Object *O,MT2D_VAR *X, MT2D_VAR *Y);
void MT2D_OF_LOSE_TARGET(Object *O, MT2D_VAR *Chance, MT2D_VAR * MinDistance);// chance from 0 to 255, min distance to test it (pass null to test at any range)

/* ================== OBJECT ATTACK  ===================== */

/* ================== OBJECT SOUND  ===================== */
void MT2D_OF_PLAY_SOUND(MT2D_VAR *SndName);
void MT2D_OF_PLAY_MUSIC(MT2D_VAR *MusicName);s
/* ================== OBJECT STATE  ===================== */
void MT2D_OF_JUMPSTATE_CHANCE(Object *o, MT2D_VAR *StateLabelName, MT2D_VAR *JumpChance);//select a random number from 0 to 255, if the number is lower than JumpChance so it'll perform a jump.
void MT2D_OF_JUMP_IF(Object *o, MT2D_VAR *StateLabelName, MT2D_VAR *ConditionOne,MT2D_VAR *LogicTest, MT2D_AR *ConditionTwo);

#endif
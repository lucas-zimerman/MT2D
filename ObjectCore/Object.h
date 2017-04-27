#ifndef OBJECT_
#define OBJECT_

#include "SpriteBlock.h"
#include "Coordinates.h"
#include "Vars.h"

#define OBJECT_DEATH -1
//Simple Object define
#define SIMPLE_STATE1 1
#define SIMPLE_STATE2 2
#define SIMPLE_STATE3 3
#define SIMPLE_STATE4 4
#define SIMPLE_STATE5 5
//Complex Object define

#define COMPLEX_IDLE_STATE 1
#define COMPLEX_ACTIVE_STATE 2
#define COMPLEX_ATTACK_STATE 3
#define COMPLEX_ATTACK2_STATE 4
#define COMPLEX_ATTACK3_STATE 5
#define COMPLEX_ATTACK4_STATE 6
#define COMPLEX_PAIN_STATE 7
#define COMPLEX_DEATH_STATE 8
//Projectile Object define
#define PROJECTILE_ACTIVE 1
#define PROJECTILE_DESTROYED 2

struct Object{
	bool RenderOnly;
	bool Solid;
	Coord SpacePosition;
	Coord Size;
	//below are vars that shouldn't be changed by the user
	int State; // -1 = Logic Dead,  0 = Idle, 1 = Active, 2 = Attack, 3 = Attack2, 4 = Attack3, 5 = Attack4, 6 = Death
	int Frame; // the actual frame that is being showed by the object, the spriteblock to be used can be determined by the State.
	int FrameCnt; // count how many tics the actual frame has to be showed, start with x and ends with 0; -1 means restart.
	bool Enabled;
	MT2D_VAR *User_Vars;
};

struct ObjectSimple{// used for simple stuff like board games where things are more hard coded
	Object DataCore; // where all the basic information is shared btw all objects
	SpriteBlock *State1;
	char *State1FramesDuration;
	SpriteBlock *State2;
	char *State2FramesDuration;
	SpriteBlock *State3;
	char *State3FramesDuration;
	SpriteBlock *State4;
	char *State4FramesDuration;
	SpriteBlock *State5;
	char *State5FramesDuration;
};

struct ObjectComplex{ // used for more complex games where the objects are more autonomous and less hard coded
	int Health;
	Object DataCore;
	Object *Target; //store the pointer to the target to be elimineted
	Coord Aceleration;
	bool CanMoveWhileAttack1;
	bool CanMoveWhileAttack2;
	bool CanMoveWhileAttack3;
	bool CanMoveWhileAttack4;
	bool CanMoveWhilePain;
	SpriteBlock *Idle;// innitial state block
	char *IdleFramesDuration;// indicates the duration that should be stayed over each animation
	SpriteBlock *Active;// walk mode, flying mode,...
	char *ActiveFramesDuration;// indicates the duration that should be stayed over each animation
	SpriteBlock *Attack;
	char *AttackFramesDuration;// indicates the duration that should be stayed over each animation
	SpriteBlock *Attack2;
	char *Attack2FramesDuration;// indicates the duration that should be stayed over each animation
	SpriteBlock *Attack3;
	char *Attack3FramesDuration;// indicates the duration that should be stayed over each animation
	SpriteBlock *Attack4;
	char *Attack4FramesDuration;// indicates the duration that should be stayed over each animation
	SpriteBlock *Pain;
	char *PainFramesDuration; // indicates the duration that should be stayed over each animation
	SpriteBlock *Death;
	char *DeathFramesDuration;// indicates the duration that should be stayed over each animation
};

struct ObjectProjectile {
	int Damage;
	Object DataCore;
	Object *Target; //store the pointer to the target to be elimineted, in case there're one
	Object *Creator; //do not hit the spawner, or do if it's null
	Coord Aceleration;
	SpriteBlock *Active;
	char *ActiveFramesDuration;
	SpriteBlock *Destroyed;
	char *DestroyedFramesDuration;
};



struct ObjectSceneControl{ // render and refresh the logic from all projectiles,particles and other effects created on the fly
	ObjectProjectile *ArrayProjectiles;
	int ProjectileCnt;
	ObjectSimple *ArraySimple; //
	int SimpleCnt;
	ObjectComplex *ArrayComplex; //
	int ComplexCnt;
};

void Object_Render(ObjectSimple *obj);
void Object_Render(ObjectComplex *obj);
void Object_Render(ObjectSceneControl *scene);

void Object_Move(ObjectComplex *obj);

void Object_ForcePain(ObjectComplex *obj);
void Object_Kill(ObjectComplex *obj);

void Object_Remove_FromMemory(ObjectComplex *obj);
void Object_Remove_FromMemory(ObjectSimple *obj);

ObjectSimple *Object_Create_Simple(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY);
ObjectComplex *Object_Create_Complex(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY, int Health);

void Object_Load_SpriteBlock(ObjectSimple *object, SpriteBlock *block1, SpriteBlock *block2, SpriteBlock *block3, SpriteBlock *block4, SpriteBlock *block5);
void Object_Load_SpriteBlock(ObjectComplex *object, SpriteBlock *blockIdle, SpriteBlock *blockActive, SpriteBlock *blockAttack, SpriteBlock *blockAttack2, SpriteBlock *blockAttack3, SpriteBlock *blockAttack4, SpriteBlock *blockPain, SpriteBlock *blockDeath);

void Object_Load_SpriteBlockAnimation(ObjectSimple *object, char *block1Tic, char *block2Tic, char *block3Tic, char *block4Tic, char *block5Tic);
void Object_Load_SpriteBlockAnimation(ObjectComplex*object, char *IdleTic, char *ActiveTic, char *AttackTic, char *Attack2Tic, char *Attack3Tic, char *Attack4Tic, char *PainTic, char *DeathTic);

ObjectSceneControl *Object_Create_SceneControl();
void ObjectScene_AddObject(ObjectSimple *object);
void ObjectScene_AddObject(ObjectComplex *object);
void ObjectScene_AddObject(ObjectProjectile *object);

void Object_Spawn_Projectile(ObjectSceneControl * scene, int sizeX, int sizeY, int damage, int direction, int speedx, int speedy, ObjectComplex *target, ObjectComplex *creator, SpriteBlock *active, char *activeTic, SpriteBlock *destroyed, char *destroyedTic);
#endif
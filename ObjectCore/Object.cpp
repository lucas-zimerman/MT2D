/* Object.cpp created by Lucas Zimerman Fraulob

*/
#include <MT2D/MT2D.h>
#include <MT2D/Objectcore/Object.h>
#include <MT2D/Objectcore/Sprites.h>
#include <stdlib.h>

void Object_Render(ObjectSimple *obj) {
	SpriteBlock *SB = 0;
	char *SBtic = 0;
	if (obj->DataCore.Enabled) {//RENDER ONLY IF THE OBJECT IS ENABLED
		//Decrement the frame counter
		obj->DataCore.FrameCnt--;
		switch (obj->DataCore.State) {//select the actual sprite block and actual spriteblock frame duration
		case SIMPLE_STATE1:
			SB = obj->State1;
			SBtic = obj->State1FramesDuration;
			break;
		case SIMPLE_STATE2:
			SB = obj->State2;
			SBtic = obj->State2FramesDuration;
			break;
		case SIMPLE_STATE3:
			SB = obj->State3;
			SBtic = obj->State3FramesDuration;
			break;
		case SIMPLE_STATE4:
			SB = obj->State4;
			SBtic = obj->State4FramesDuration;
			break;
		case SIMPLE_STATE5:
			SB = obj->State5;
			SBtic = obj->State5FramesDuration;
			break;
		}
		//Check if the frame tic has expired, if yes select the next frame
		if (obj->DataCore.FrameCnt <= -1) {
			if (obj->DataCore.Frame + 1 >= SB->cnt) {//if the next frame doesn't exist, point to the first frame
				obj->DataCore.Frame = 0;
			}
			else {
				obj->DataCore.Frame++;
			}
			obj->DataCore.FrameCnt = SBtic[obj->DataCore.Frame];//update the timer
		}
		//Render the frame
		Sprite_Render_on_Window(&SB->sprites[obj->DataCore.Frame], DISPLAY_WINDOW1, obj->DataCore.SpacePosition.X, obj->DataCore.SpacePosition.Y);
	}
}

void Object_Render(ObjectComplex *obj) {//Not Implemented
	SpriteBlock *SB = 0;
	char *SBtic = 0;
	if (obj->DataCore.Enabled) {//RENDER ONLY IF THE OBJECT IS ENABLED
								//Decrement the frame counter
		obj->DataCore.FrameCnt--;
		switch (obj->DataCore.State) {//select the actual sprite block and actual spriteblock frame duration
		case COMPLEX_ACTIVE_STATE:
			SB = obj->Active;
			SBtic = obj->ActiveFramesDuration;
			break;
		case COMPLEX_ATTACK_STATE:
			SB = obj->Attack;
			SBtic = obj->AttackFramesDuration;
			break;
		case COMPLEX_ATTACK2_STATE:
			SB = obj->Attack2;
			SBtic = obj->Attack2FramesDuration;
			break;
		case COMPLEX_ATTACK3_STATE:
			SB = obj->Attack3;
			SBtic = obj->Attack3FramesDuration;
			break;
		case COMPLEX_ATTACK4_STATE:
			SB = obj->Attack4;
			SBtic = obj->Attack4FramesDuration;
			break;
		case COMPLEX_DEATH_STATE:
			SB = obj->Death;
			SBtic = obj->DeathFramesDuration;
			break;
		case COMPLEX_IDLE_STATE:
			SB = obj->Idle;
			SBtic = obj->IdleFramesDuration;
			break;
		case COMPLEX_PAIN_STATE:
			SB = obj->Pain;
			SBtic = obj->PainFramesDuration;
			break;
		}
		//Check if the frame tic has expired, if yes select the next frame
		if (obj->DataCore.FrameCnt <= -1) {
			if (obj->DataCore.Frame + 1 >= SB->cnt) {//if the next frame doesn't exist, point to the first frame
				obj->DataCore.Frame = 0;
			}
			else {
				obj->DataCore.Frame++;
			}
			obj->DataCore.FrameCnt = SBtic[obj->DataCore.Frame];//update the timer
		}
		//Render the frame
		Sprite_Render_on_Window(&SB->sprites[obj->DataCore.Frame], DISPLAY_WINDOW1, obj->DataCore.SpacePosition.X, obj->DataCore.SpacePosition.Y);
	}
}

void Object_Render(ObjectProjectile *obj) {//Not Implemented
	SpriteBlock *SB = 0;
	char *SBtic = 0;
	if (obj->DataCore.Enabled) {//RENDER ONLY IF THE OBJECT IS ENABLED
								//Decrement the frame counter
		obj->DataCore.FrameCnt--;
		switch (obj->DataCore.State) {//select the actual sprite block and actual spriteblock frame duration
		case PROJECTILE_ACTIVE:
			SB = obj->Active;
			SBtic = obj->ActiveFramesDuration;
			break;
		case PROJECTILE_DESTROYED:
			SB = obj->Destroyed;
			SBtic = obj->DestroyedFramesDuration;
			break;
		}
		//Check if the frame tic has expired, if yes select the next frame
		if (obj->DataCore.FrameCnt <= -1) {
			if (obj->DataCore.Frame + 1 >= SB->cnt) {//if the next frame doesn't exist, point to the first frame
				obj->DataCore.Frame = 0;
			}
			else {
				obj->DataCore.Frame++;
			}
			obj->DataCore.FrameCnt = SBtic[obj->DataCore.Frame];//update the timer
		}
		//Render the frame
		Sprite_Render_on_Window(&SB->sprites[obj->DataCore.Frame], DISPLAY_WINDOW1, obj->DataCore.SpacePosition.X, obj->DataCore.SpacePosition.Y);
	}
}

void Object_Render(ObjectSceneControl *scene) {//Not Implemented
	int i = 0;
	for (i = 0; i < scene->SimpleCnt; i++) {
		Object_Render(&scene->ArraySimple[i]);
	}
	for (i = 0; i < scene->ComplexCnt; i++) {
		Object_Render(&scene->ArrayComplex[i]);
	}
	for (i = 0; i < scene->ProjectileCnt; i++) {
		Object_Render(&scene->ArrayProjectiles[i]);
	}
}

void Object_Move(ObjectComplex *obj){
	bool Move_Alloyed = false;
	if (obj->Active) {// Only move if the object is activated
		switch (obj->DataCore.State) {//check if in the actual state the object can move
		case COMPLEX_ACTIVE_STATE:
			Move_Alloyed = true;
			break;
		case COMPLEX_ATTACK_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack1;
			break;
		case COMPLEX_ATTACK2_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack2;
			break;
		case COMPLEX_ATTACK3_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack3;
			break;
		case COMPLEX_ATTACK4_STATE:
			Move_Alloyed = obj->CanMoveWhileAttack4;
			break;
		case COMPLEX_PAIN_STATE:
			Move_Alloyed = obj->CanMoveWhilePain;
			break;
		case COMPLEX_IDLE_STATE:
			// Always false
			break;
		case COMPLEX_DEATH_STATE:
			//Always false
			break;
		}
		if (Move_Alloyed) {
			obj->DataCore.SpacePosition.X += obj->Aceleration.X;
			obj->DataCore.SpacePosition.Y += obj->Aceleration.Y;
		}
	}
}

void Object_ForcePain(ObjectComplex *obj){
	if (obj->Active) {
		if (obj->Health > 0) {// no pain if no health
			obj->DataCore.State = COMPLEX_PAIN_STATE;
			obj->DataCore.Frame = 0;
			obj->DataCore.FrameCnt = obj->PainFramesDuration[0];
		}
	}
}
void Object_Kill(ObjectComplex *obj) {
	if (obj->Active) {
		if (obj->Health > 0) {// do not kill what's already dead
			obj->DataCore.State = COMPLEX_DEATH_STATE;
			obj->DataCore.Frame = 0;
			obj->DataCore.FrameCnt = obj->DeathFramesDuration[0];
		}
	}
}

void Object_Remove_FromMemory(ObjectComplex *obj) {//Not Implemented

}

void Object_Remove_FromMemory(ObjectSimple *obj) {//Not Implemented

}

void Object_Think(ObjectComplex *obj) {//Not Implemented

}

ObjectSimple *Object_Create_Simple(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY) {
	ObjectSimple *Obj;
	Obj = (ObjectSimple*)malloc(sizeof(ObjectSimple));

	Obj->DataCore.Solid = Solid;
	Obj->DataCore.RenderOnly = RenderOnly;
	Obj->DataCore.Size.X = sizeX;
	Obj->DataCore.Size.Y = sizeY;
	Obj->DataCore.SpacePosition.X = PosX;
	Obj->DataCore.SpacePosition.Y = PosY;
	Obj->DataCore.Frame = 0;
	Obj->DataCore.FrameCnt = 0;
	Obj->DataCore.State = 1;
	Obj->DataCore.Enabled = true;

	Obj->State1 = 0;
	Obj->State1FramesDuration = 0;
	Obj->State2 = 0;
	Obj->State2FramesDuration = 0;
	Obj->State3 = 0;
	Obj->State3FramesDuration = 0;
	Obj->State4 = 0;
	Obj->State4FramesDuration = 0;
	Obj->State5 = 0;
	Obj->State5FramesDuration = 0;
	Obj->DataCore.User_Vars =0;
	return Obj;
}

ObjectComplex *Object_Create_Complex(bool Solid, bool RenderOnly, int sizeX, int sizeY, int PosX, int PosY, int Health) {//Not Implemented
	ObjectComplex * Obj=0;
	Obj = (ObjectComplex*)malloc(sizeof(ObjectComplex));
	Obj->Aceleration.X = 0;
	Obj->Aceleration.Y = 0;
	Obj->Active = 0;
	Obj->ActiveFramesDuration = 0;
	Obj->Attack = 0;
	Obj->AttackFramesDuration = 0;
	Obj->Attack2 = 0;
	Obj->Attack2FramesDuration = 0;
	Obj->Attack3 = 0;
	Obj->Attack3FramesDuration = 0;
	Obj->Attack4 = 0;
	Obj->Attack4FramesDuration = 0;
	Obj->CanMoveWhileAttack1 = false;
	Obj->CanMoveWhileAttack2 = false;
	Obj->CanMoveWhileAttack3 = false;
	Obj->CanMoveWhileAttack4 = false;
	Obj->CanMoveWhilePain = false;

	Obj->DataCore.Enabled = true;
	Obj->DataCore.Frame = 0;
	Obj->DataCore.FrameCnt = 0;
	Obj->DataCore.RenderOnly = RenderOnly;
	Obj->DataCore.Size.X = sizeX;
	Obj->DataCore.Size.Y = sizeY;
	Obj->DataCore.Solid = Solid;
	Obj->DataCore.SpacePosition.X = PosX;
	Obj->DataCore.SpacePosition.Y = PosY;
	Obj->DataCore.State = 0;

	Obj->Death = 0;
	Obj->DeathFramesDuration = 0;
	Obj->Health = Health;
	Obj->Idle = 0;
	Obj->IdleFramesDuration = 0;
	Obj->Pain = 0;
	Obj->PainFramesDuration = 0;
	Obj->Target = 0;
	Obj->DataCore.User_Vars =0;
	return Obj;
}

void Object_Load_SpriteBlock(ObjectSimple *object, SpriteBlock *block1, SpriteBlock *block2, SpriteBlock *block3, SpriteBlock *block4, SpriteBlock *block5) {
	if (block1){
		object->State1 = block1;
	}
	if (block2){
		object->State2 = block2;
	}
	if (block3) {
		object->State3 = block3;
	}if (block4) {
		object->State4 = block4;
	}if (block5) {
		object->State5 = block5;
	}
}

void Object_Load_SpriteBlock(ObjectComplex *object, SpriteBlock *blockIdle, SpriteBlock *blockActive, SpriteBlock *blockAttack, SpriteBlock *blockAttack2, SpriteBlock *blockAttack3, SpriteBlock *blockAttack4, SpriteBlock *blockPain, SpriteBlock *blockDeath) {
	if (blockActive) {
		object->Active = blockActive;
	}
	if (blockAttack) {
		object->Attack = blockAttack;
	}
	if (blockAttack2) {
		object->Attack2 = blockAttack2;
	}
	if (blockAttack3) {
		object->Attack3 = blockAttack3;
	}
	if (blockAttack4) {
		object->Attack4 = blockAttack4;
	}
	if (blockDeath) {
		object->Death = blockDeath;
	}
	if (blockIdle) {
		object->Idle = blockIdle;
	}
	if (blockPain) {
		object->Pain = blockPain;
	}
}

void Object_Load_SpriteBlockAnimation(ObjectSimple *object, char *block1Tic, char *block2Tic, char *block3Tic, char *block4Tic, char *block5Tic){
	if (block1Tic) {
		object->State1FramesDuration= block1Tic;
	}
	if (block2Tic) {
		object->State2FramesDuration = block2Tic;
	}
	if (block3Tic) {
		object->State3FramesDuration = block3Tic;
	}if (block4Tic) {
		object->State4FramesDuration = block4Tic;
	}if (block5Tic) {
		object->State5FramesDuration = block5Tic;
	}
}

void Object_Load_SpriteBlockAnimation(ObjectComplex*object, char *IdleTic, char *ActiveTic, char *AttackTic, char *Attack2Tic, char *Attack3Tic, char *Attack4Tic, char *PainTic, char *DeathTic) {//Not Implemented

}

ObjectSceneControl *Object_Create_SceneControl() {//Not Implemented
	ObjectSceneControl *Obj=0;

	return Obj;
}

void ObjectScene_AddObject(ObjectSimple *object) {//Not Implemented

}

void ObjectScene_AddObject(ObjectComplex *object) {//Not Implemented

}

void ObjectScene_AddObject(ObjectProjectile *object) {//Not Implemented

}

void Object_Spawn_Projectile(ObjectSceneControl * scene, int sizeX, int sizeY, int damage, int direction, int speedx, int speedy, ObjectComplex *target, ObjectComplex *creator, SpriteBlock *active, char *activeTic, SpriteBlock *destroyed, char *destroyedTic) {//Not Implemented

}

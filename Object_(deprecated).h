#ifndef OBJECT_STRUCT
#define OBJECT_STRUCT

#define MOVE_UP    0
#define MOVE_DOWN  1
#define MOVE_LEFT  2
#define MOVE_RIGHT 3
#define IDLE_ACTION 0
#define DEATH_ACTION -1
#define WALK_ACTION 1
#define PAIN_ACTION 2
#define MELEE_ACTION 3
#define MISSILE_ACTION 4
#define SIMPLE_OBJECT 0
struct xy_value{//Used for two values that need a int x and int y, like speed x speed y, aceleration x, aceleration y,...
	float Y;
	float X;
};

struct sprite{
	char **Data;
	xy_value size;//The size of the sprite
	xy_value scale;//Ajust the 
};

struct sprite_block{
	sprite *sprites;
};

struct animation_sprites{
	int *frames;//show what sprites will be used on this animation
	int *frames_speed;//show whats the delay to be stayed on that sprite
};

struct missile_ai{
	sprite_block *Data;
	animation_sprites track;
	animation_sprites explosion;
	xy_value speed;
	xy_value position;
	int damage;
	bool destroyable;//0 = no, 1= yes;
	bool tracker;//0 = no, 1=yes
	int life;//if not destroyable, it'll be 0
};

struct object{
	sprite_block *Data;
	animation_sprites *idle;
	animation_sprites *walk;
	animation_sprites *melee;
	animation_sprites *missile;
	animation_sprites *death;
	animation_sprites *pain;
	xy_value speed;
	xy_value screen_position;//where the object is on screen || -1 if not on screen
	xy_value size;// the size of the object
	int pain_chance;//how much dam will receive to enter in pain state
	int life;//-1 if infinite
	int sprite_cnt;//How many sprites does this object have
	int type;// 0 <- simple object
	bool block;// 0 <- pacible, 1 <- impacible
	int actual_state;//0 iddle, -1 death, 1 walk, 2 pain, 3 melee, missile
	//Vars to changed by the game and not the user;
	int pain_status;//pain counter to enter in pain state	
	int actual_life;//See how many life rest for the object
	int id;//identify number of the object
};

sprite *Load_Sprite(char *file);
sprite_block *Load_Sprite_Block(char **files, int total_files);
object *Load_Object(int type, bool block, int life, int speed, int size_x, int size_y, int pain_chance,sprite_block *Sprites,animation_sprites *idle, animation_sprites *walk, animation_sprites *melee,animation_sprites *missile, animation_sprites *pain, animation_sprites *death);
animation_sprites *Load_Sprite_Animation();
bool put_sprite_on_window(sprite *img, int witch_window, int pos_x, int pos_y);
bool put_object_on_window(object *obj, int witch_window);
void debug_object_test();
void set_object_position(int pos_x, int pos_y,object *obj);
void move_object(int direction, object *obj);

#endif

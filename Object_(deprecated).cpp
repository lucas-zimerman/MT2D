#ifdef __MSDOS__
#include "mt2d\Object.h"
#include "..\..\mt2d\Window_core.h"
#include "..\..\mt2d\Io_FILE.h"
#include "Keyboard.h"
#endif
#ifdef _WIN32
#include "Object.h"
#include "Window_core.h"
#include "Io_FILE.h"
#include "Keyboard.h"
#endif
#ifdef linux
#include "Object.h"
#include "Window_core.h"
#include "Io_FILE.h"
#include "Keyboard.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned char WINDOW1[MAX_VER+1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER+1][MAX_HOR];

#define windows_file 0
#define MSDOS_file 1
#define unix_file 2

void debug_print_sprite(sprite *X);

int _size_x=0;
int _size_y=0;
//get the exact sprite size for an exact malloc
void get_sprite_size(char *name){
	_size_x=0;
	_size_y=0;
	int buf_x=0;
	int buff=0;
	FILE *x=0;
	if(name){
		x=fopen(name,"r");
		if(x){
			do{
				buff=fgetc(x);
				if(buff=='\n'){
					_size_y++;
					if(buf_x==0){
						buf_x=_size_x;
					}
				}else if(buf_x==0 && buff!=-1) _size_x++;
			}while(!feof(x));
			_size_y++;
			if(buf_x)_size_x=buf_x;
			fclose(x);
		}
	}
}



sprite_block *Load_Sprite_Block(char **files, int total_files){
	FILE *in=0;
	sprite_block *sb=0;
	int vecsize=0;
	sb=(sprite_block*)malloc(sizeof(sprite_block));
	if(!sb){
		printf("\a");
		exit(-1);
	}do{
		if(vecsize)sb=(sprite_block*)realloc(sb,(vecsize+1)*sizeof(sprite_block));//sb[2];
		if(!sb){//If sprite not loaded
			printf("ERROR, sprite not loaded to sprite block\a");
			Keyboard_keytouched(); // KEY WAIT LOCK
			exit(-1);
		}
		sb[vecsize].sprites=Load_Sprite(files[vecsize]);
		vecsize++;
	}while(vecsize<total_files);
	sb[vecsize].sprites=NULL;// sb[1]=null
	return sb;
}



int Load_Sprite_Bussy=0;//This will prevent Two Load_Sprite process to run at the sime time since this function needs global var datas intact
sprite *Load_Sprite(char *file){
//	printf("DEBUG: loading sprite %s\n",file);
	FILE *in=0;
	sprite *new_sprite=0;
	char BUFFER=0;
	int cnt=0;
	int i=0;
	in = open_file(file,"r");
	if(!in){
		printf("ERROR: FILE NOT FOUND:%s\a\n",file);
		printf("REPLACING %s by default error sprite...\a\n",file);
		in = open_file(file,"w");
		fputc('<',in);fputc('!',in);fputc('>',in);
		fclose(in);
		in = open_file(file,"r");
	}//else{
		while(Load_Sprite_Bussy)printf(".");//used to avoid threads to load things at the same time
		Load_Sprite_Bussy=true;
		get_sprite_size(file);
	//}
//	printf("DEBUG: sprite size readed, X:%d Y:%d\n",_size_x,_size_y);
	new_sprite=(sprite*)malloc(sizeof(sprite));
	if(!new_sprite){
		printf("\a\a");
		exit(-1);
//	printf("DEBUG: ponteiro da sprite alocado :%d\n",new_sprite);
	//Aloca linhas
	}new_sprite->Data=(char**)malloc(_size_y*sizeof(char*));
	if(!new_sprite->Data){
		printf("\a\a\a");
		exit(-3);
	}
	new_sprite->scale.Y=0;
	new_sprite->scale.X=0;
	new_sprite->size.X=(float)_size_x;
	new_sprite->size.Y=(float)_size_y;
	for(i=0;i<_size_y;i++){//Aloca colunas nas linhas
//		printf("DEBUG: for (i=0;%d<%d;i++)\n",i,_size_y);
		new_sprite->Data[i]=(char*)malloc((_size_x+1)*sizeof(char));
		if(!new_sprite->Data[i]){
			printf("exit -4\a\a\a\a");
			exit(-4);
		}
	}
	i=0;
	while(!feof(in)){//Alloc a line loop
		BUFFER=0;
		while(!feof(in) && BUFFER!='\n'){//Alloc a colun loop
			BUFFER=fgetc(in);
			if(BUFFER!=-1){
				new_sprite->Data[i][cnt]=BUFFER;// BREAKPOINT=>16-04-2014, investigar caso pass por aqui denovo, imagem "ENGINE_LOGO.txt", cnt=0,i=0, buffer=='/', _size_y=6 _size_x=33
				cnt++;
			}
		}
//		printf("\n");
		new_sprite->Data[i][cnt]='\0';
		cnt=0;
		i++;
	}
//	printf("\n\n");
	Load_Sprite_Bussy=false;
	//Loop para carregar string e converter em sprite
	return new_sprite;
}
//Load_Sprite 100% Completed


animation_sprites *Load_Sprite_Animation(){
	animation_sprites *X;
	X=(animation_sprites*)malloc(sizeof(animation_sprites));
	X->frames=(int*)malloc(sizeof(int));
	X->frames[0]=0;
	X->frames_speed=(int*)malloc(sizeof(int));
	X->frames_speed[0]=0;
	return X;
}

object *Load_Object(int type, bool block, int life, int speed, int size_x, int size_y, int pain_chance,sprite_block *Sprites,animation_sprites *idle, animation_sprites *walk, animation_sprites *melee,animation_sprites *missile, animation_sprites *pain, animation_sprites *death){
	object *obj;
	obj=(object*)malloc(sizeof(object));
	if(obj){
		obj->type=type;
		obj->block=block;
		obj->life=life;
		obj->speed.X=(float)speed;
		obj->speed.Y=(float)speed;
		obj->size.X=(float)size_x;
		obj->size.Y=(float)size_y;
		obj->Data=Sprites;
		//Need a better system for loading complex objects
		//Alloc Sprite animation memory
		obj->idle=Load_Sprite_Animation();
		obj->walk=Load_Sprite_Animation();
		obj->melee=Load_Sprite_Animation();
		obj->missile=Load_Sprite_Animation();
		obj->pain=Load_Sprite_Animation();
		obj->death=Load_Sprite_Animation();
		obj->id=0;

		//Transfer Sprite Animation memory
		if(idle){
			obj->idle->frames[0]=idle->frames[0];
			obj->idle->frames_speed[0]=idle->frames_speed[0];
		}else{
			obj->idle->frames[0]=0;
			obj->idle->frames_speed[0]=0;
		}if(walk){
			obj->walk->frames[0]=walk->frames[0];
			obj->walk->frames_speed[0]=walk->frames_speed[0];
		}else{
			obj->walk->frames[0]=0;
			obj->walk->frames_speed[0]=0;
		}if(melee){
			obj->melee->frames[0]=melee->frames[0];
			obj->melee->frames_speed[0]=melee->frames_speed[0];
		}else{
			obj->melee->frames[0]=0;
			obj->melee->frames_speed[0]=0;
		}if(missile){
			obj->missile->frames[0]=missile->frames[0];
			obj->missile->frames_speed[0]=missile->frames_speed[0];
		}else{
			obj->missile->frames[0]=0;
			obj->missile->frames_speed[0]=0;
		}if(pain){
			obj->pain->frames[0]=pain->frames[0];
			obj->pain->frames_speed[0]=pain->frames_speed[0];
		}else{
			obj->pain->frames[0]=0;
			obj->pain->frames_speed[0]=0;
		}if(death){
			obj->death->frames[0]=death->frames[0];
			obj->death->frames_speed[0]=death->frames_speed[0];
		}else{
			obj->death->frames[0]=0;
			obj->death->frames_speed[0]=0;
		}
		obj->actual_life=life;
		obj->screen_position.X=-1;
		obj->screen_position.Y=-1;
		obj->pain_chance=pain_chance;
		obj->pain_status=0;
	}
	return obj;
}

void set_object_position(int pos_x, int pos_y,object *obj){
	//if you get a break here, you didn't loaded the *obj
	obj->screen_position.X=(float)pos_x;
	obj->screen_position.Y=(float)pos_y;
}

void move_object(int direction, object *obj){
	switch(direction){
	case MOVE_UP:
		obj->screen_position.Y--;
		break;
	case MOVE_DOWN:
		obj->screen_position.Y++;
		break;
	case MOVE_LEFT:
		obj->screen_position.X--;
		break;
	case MOVE_RIGHT:
		obj->screen_position.X++;
		break;
	}
}

bool put_object_on_window(object *obj, int witch_window){
	int x=0,x1=0;//,x2; not used
	int y=0,y1=0;//,y2; not used
	int pos_x;
	int pos_y;
	int actual_img = 0;
	sprite *img=0;
	bool out=false;
	if(obj){
		if(obj->actual_life>0){
			pos_x=(int)obj->screen_position.X;
			pos_y=(int)obj->screen_position.Y;
			if(obj->type==SIMPLE_OBJECT){
				actual_img = obj->pain_chance++; //gambiarra
				if (actual_img > obj->sprite_cnt) {
					actual_img = 0;
				}
				img=&obj->Data[obj->idle->frames[0]].sprites[actual_img];// A SER TESTADO (14-4-2014
				//img=&obj->Data[obj->idle->frames[0]].sprites[0];
//				img=&obj->Data[0].sprites[obj->idle->frames[0]];// A SER TESTADO (14-4-2014
//				printf("debug, sprite block. imb=obj->Data[0].sprites[%d\n",obj->idle->frames[0]);
//				printf("teste 0\n");
//				debug_print_sprite(&obj->Data[0].sprites[0]);
//				printf("teste 1\n");
//				debug_print_sprite(&obj->Data[1].sprites[0]);
//				printf("teste 2\n");
//				debug_print_sprite(img);
			}else{//Falta Filtro de objetos complexos
				switch(obj->actual_state){
				case IDLE_ACTION:
					break;
				case DEATH_ACTION:
					break;
				case WALK_ACTION:
					break;
				case PAIN_ACTION:
					break;
				case MELEE_ACTION:
					break;
				case MISSILE_ACTION:
					break;
				}
			}if(img){
				//if(pos_x+img->size.X<MAX_HOR){
				//	if(pos_y+img->size.Y<MAX_VER){
						out=true;
						x=(int)pos_x+img->size.X;//if you break the code here, the image wasn't loaded into memory, check add
						y=(int)pos_y+img->size.Y;
						x1=pos_x;
						y1=pos_y;
						if(witch_window==DISPLAY_WINDOW1){
							while(y1<y){
								while(x1<x){
									if(y1<MAX_VER && y>=0 && x1<MAX_HOR && x>=0 && img->Data[y1-pos_y][x1-pos_x]!=' ' && x1 >=0)WINDOW1[y1][x1]=img->Data[y1-pos_y][x1-pos_x];
									x1++;
								}
								x1=pos_x;
								y1++;
							}
						}else{
							while(y1<y){
								while(x1<x){
									if(y1<MAX_VER && y>=0 && x1<MAX_HOR && x>=0 && img->Data[y1-pos_y][x1-pos_x]!=' ' && x1 >=0 )WINDOW2[y1][x1]=img->Data[y1-pos_y][x1-pos_x];
									x1++;
								}
								x1=pos_x;
								y1++;
							}
				}
			}
		}
	}
	return out;
}



bool put_sprite_on_window(sprite *img, int witch_window, int pos_x, int pos_y){
	int x=0,x1=0;//,x2; not used
	int y=0,y1=0;//,y2; not used
	bool out=false;
	if(img){
				out=true;
				x=(int)pos_x+img->size.X;
				y=(int)pos_y+img->size.Y;
				x1=pos_x;
				y1=pos_y;
				if(y1<0)y1=0;
				else if(y1>24)y1=y;
				if(x1<0)x1=0;
				else if(x1>80)y1=y;
				if(witch_window==DISPLAY_WINDOW1){
					while(y1<y){
						while(x1<x){
							if(img->Data[y1-pos_y][x1-pos_x]!=' ')
								if(y1<MAX_VER && y1>=0 && x1<MAX_HOR && x1>=0)
									WINDOW1[y1][x1]=img->Data[y1-pos_y][x1-pos_x];
							x1++;
						}
						x1=pos_x;
						y1++;
					}
				}else{
					while(y1<y){
						while(x1<x){
							if(img->Data[y1-pos_y][x1-pos_x]!=' ')
								if(y1<MAX_VER && y1>=0 && x1<MAX_HOR && x1>=0)
									WINDOW2[y1][x1]=img->Data[y1-pos_y][x1-pos_x];
							x1++;
						}
						x1=pos_x;
						y1++;
					}
				}
	}
	return out;
}

void main_test_object(){
	object x;
	x.Data=0;

}

void debug_print_sprite(sprite *X){
	int i=0;
	printf("-DEBUG. SPRITE LOADED:\n");
	printf("--SCALE X:%.0f\n--SCALE Y:%.0f\n--SIZE X:%.0f\n--SIZE Y:%.0f\nSPRITE:\n",X->scale.X,X->scale.Y,X->size.X,X->size.Y);
	while(i<X->size.Y){
		printf("%s\n",X->Data[i]);
		i++;
	}
}

void debug_object_test_spriteblock(){
	sprite_block *sprite_block1;
	char **teste2;
	teste2=(char**)malloc(4*sizeof(char*));
	teste2[0]=(char*)malloc(20*sizeof(char));
	teste2[1]=(char*)malloc(20*sizeof(char));
	teste2[2]=(char*)malloc(20*sizeof(char));
	teste2[3]=(char*)malloc(20*sizeof(char));
	teste2[4]=(char*)malloc(20*sizeof(char));
	strcpy(teste2[0],"sprite1.txt");
	strcpy(teste2[1],"sprite2.txt");
	strcpy(teste2[2],"sprite3.txt");
	strcpy(teste2[3],"sprite4.txt");
	sprite_block1=Load_Sprite_Block(teste2,4);
	printf("LOADED\n");
	debug_print_sprite(sprite_block1[0].sprites);
	debug_print_sprite(sprite_block1[1].sprites);
	debug_print_sprite(sprite_block1[2].sprites);
	debug_print_sprite(sprite_block1[3].sprites);
}
void debug_object_test_sprite(){
	sprite *sprite1;
	sprite *sprite2;
	sprite *sprite3;
	sprite *sprite4;
	//sprite *sprite5; not used
	printf("-loading sprite1.txt\n");
	sprite1=Load_Sprite("sprite1.txt");
	if(sprite1) printf("-Sprite1.txt loaded\n");
	debug_print_sprite(sprite1);
	printf("-loading sprite2.txt\n");
	sprite2=Load_Sprite("sprite2.txt");
	if(sprite2) printf("-Sprite2.txt loaded\n");
	debug_print_sprite(sprite2);
	printf("-loading sprite3.txt\n");
	sprite3=Load_Sprite("sprite3.txt");
	if(sprite3) printf("-Sprite3.txt loaded\n");
	debug_print_sprite(sprite3);
	printf("-loading sprite4.txt\n");
	sprite4=Load_Sprite("sprite4.txt");
	if(sprite4) printf("-Sprite4.txt loaded\n");
	debug_print_sprite(sprite4);
	printf("-loading sprite5.txt\n");
	sprite4=Load_Sprite("sprite5.txt");
	if(sprite4) printf("-Sprite5.txt loaded\n");
	debug_print_sprite(sprite4);

}
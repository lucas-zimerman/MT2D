/**********************************************************
    MT2D_DosVGA_Render, created by Lucas Zimerman Fraulob

    this code render sprites and also ascii in a default
    256 color 320x200 display, the font used is hardcoded
    inside the font.h file.
    You'll see flickering in the screen in case you do not
    enable the  MSDOS_USE_DOUBLE_BUFFER define in the terminal
    define (this will cost you an extra 64KB of ram to be used
    by the code).


    How to make the best render ever made?
    -Simple, just make one where it doesn't render nothing.

    NOTE: This code has a hard coded limit to 32,767 sprites
    being rendered on screen.
***********************************************************/
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
#include <dos.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/_MSDOS/Render/font.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <time.h>

#ifdef MSDOS_USE_SPRITE_TILE_RENDER
#include <MT2D/_MSDOS/Render/VGAsprTi.h>
extern SpritesTilesRender SpriteTiles[25][80];/*8KB of initial data ,will expand from size when needed*/
/***
	Used to store what parts of each sprite is under the tiles
   if the value is negative, then we have more than one sprites
   under the same tile so in that case we're going to search
   sprite by sprite that are on that tile
**/
#endif

const unsigned char TransparentPallete = 0;
Sprite *VGASpritesToRender =0;
int *VGASpriteX = 0;
int *VGASpriteY =0;
unsigned int VGASpritesCnt =0;
extern unsigned char *VGA;
#ifdef MSDOS_USE_DOUBLE_BUFFER
extern unsigned char *DOUBLE_BUFFER;
#endif
extern unsigned short VGA_MemOffset;
extern const unsigned char VGAFONT[];
unsigned char VGA_ASCII_Backup[25][80];/*2KB of RAM here*/
#ifdef MSDOS_USE_SPRITE_TILE_RENDER
void RenderSpritesTile(unsigned char *Tmp,SpritesTilesRender *SpriteTile, int tileX,int scrYoffset);
#else
void MT2D_Dos_Render_Sprites();
#endif
extern unsigned char VGA_ASCII_COLORS[2]; /*store the index of the colors to be used by a char*/

int frame =0;

void Dos_Clear_ImgBuffer(){
	if (VGASpritesCnt > 0) {
   	/*Clears the text buffer where the image is going to be placed on screen.*/
   	int i =0;
   	int i0;// = X >> 2; //(X*80)/320 ->  x/4  -> x>>2
   	int i1;// = (X+IMG->size.X) >> 2;
   	int j0;// = Y >> 3; //(Y*25)/200 -> Y/8  -> Y>>3
   	int j1;// = (Y+IMG->size.Y) >> 3;
      int tmp;
   	while(i < VGASpritesCnt){

   		i0 = VGASpriteX[i] >> 2; //(X*80)/320 ->  x/4  -> x>>2
   		i1 = 1 + ((VGASpriteX[i]+VGASpritesToRender[i].size.X) >> 2);
   		j0 = VGASpriteY[i] >> 3; //(Y*25)/200 -> Y/8  -> Y>>3
   		j1 = 1 + ((VGASpriteY[i]+VGASpritesToRender[i].size.Y) >> 3);

			/*If for some reason the sprite went out of screen, lets just avoid to clear stuff outside our memory block :) */
         if(j0 < 0){
          	j0 =0;
         }
   		if(j1 > 25){
    			j1 = 25;
   		}
         if(i0 < 0){
          	i0 = 0;
         }
   		if(i1 > 80){
    			i1 = 80;
   		}
         tmp = i0;
   		while(j0 < j1){
      		while(i0 < i1){
               #ifdef MSDOS_USE_SPRITE_TILE_RENDER
               //free(SpriteTiles[j0][i0].Ids);
               //SpriteTiles[j0][i0].Ids = 0;
               SpriteTiles[j0][i0].Count = 0;
//               SpriteTiles[j0][i0].OldData = SpriteTiles[j0][i0].NewData;
               SpriteTiles[j0][i0].NewData = 0;
               #else
         			VGA_ASCII_Backup[j0][i0] = 255; //clears the text buffer in the right place where the sprite is going to be
               #endif
         		i0++;
      		}
      		i0 = tmp;
   			j0++;
   		}
         i++;
   	}
   	/*now we clear the image buffers*/
		free(VGASpritesToRender);
		free(VGASpriteX);
		free(VGASpriteY);
   	VGASpritesCnt = 0;
   }
}

void Dos_Add_ImagetoBuffer(Sprite *IMG,int X, int Y) {
/*Store a pointer to the requested sprite to be rendered
and render it when the main render call is requested.*/
	if (VGASpritesCnt == 0) {
		VGASpritesToRender = (Sprite*)malloc(sizeof(Sprite));
		VGASpriteX = (int*)malloc(sizeof(int));
		VGASpriteY = (int*)malloc(sizeof(int));
		VGASpritesToRender[VGASpritesCnt] = *IMG;
		VGASpriteX[VGASpritesCnt] = X;
		VGASpriteY[VGASpritesCnt] = Y;
	}
	else {
		VGASpritesToRender = (Sprite*)realloc(VGASpritesToRender,(VGASpritesCnt+1)*sizeof(Sprite));
		VGASpriteY = (int*)realloc(VGASpriteY, (VGASpritesCnt + 1) * sizeof(int));
		VGASpriteX = (int*)realloc(VGASpriteX, (VGASpritesCnt + 1) * sizeof(int));
		VGASpritesToRender[VGASpritesCnt] = *IMG;
		VGASpritesToRender[VGASpritesCnt] = *IMG;
		VGASpriteX[VGASpritesCnt] = X;
		VGASpriteY[VGASpritesCnt] = Y;
	}
   #ifdef MSDOS_USE_SPRITE_TILE_RENDER
   int X2 = (X > 0 ? X : 0);
	int Xtmp;
   int EndX = X + IMG->size.X;
   int EndY = Y + IMG->size.Y;
   int NewX,NewY;
   int *Address = (int*)(IMG);
   if(EndX > 320){
   	EndX = 320;
   }
   if(EndY > 200){
    	EndY = 200;
   }

   NewY = (Y > 0 ? Y >> 3 : 0);/*y / 8*/
   for(;Y < EndY;Y+=8, NewY++){
       for(Xtmp = X2,NewX = X2 >> 2; Xtmp < EndX; Xtmp+=4,NewX++){
       		if(SpriteTiles[NewY][NewX].Count == SpriteTiles[NewY][NewX].DEBUG_OLD_COUNT){
            	if(SpriteTiles[NewY][NewX].Count == 0){
               	SpriteTiles[NewY][NewX].Ids = (unsigned int*)malloc(sizeof(unsigned int));
            	}
           	 	else{
               	SpriteTiles[NewY][NewX].Ids = (unsigned int*)realloc(SpriteTiles[NewY][NewX].Ids, (SpriteTiles[NewY][NewX].Count+1)*sizeof(unsigned int));
               }
	            SpriteTiles[NewY][NewX].DEBUG_OLD_COUNT++;
            }
            SpriteTiles[NewY][NewX].Ids[SpriteTiles[NewY][NewX].Count] = VGASpritesCnt; /*Save the sprite Id*/
            SpriteTiles[NewY][NewX].NewData =
            	(((SpriteTiles[NewY][NewX].NewData >> 10) /*Filter the pointers*/
               		  /*Pointers are 32bits but we are in a 16bits env, so we do some workaround to get the 2 slices of 16bits*/
							+ (Address[0]+Address[1]) /*Add the actual pointer*/
                  )
						<< 10 /*Move back the original pointer area*/
               )
					/*-----STEP 2, ADD THE HORIZONTAL COORD----*/
					|
					(
						(
							(
								((SpriteTiles[NewY][NewX].NewData >5) /*Move the Horizontal stuff to the left*/
									& 31 /*Filter the First 5 bits*/
								)
								+	X	/*Adds the actual pos X*/
							) << 5 /*Moves back where it belongs to*/
						) & 992/*Remove some garbage made after the 10 bits zone*/
					)
				|
				/*-----STEP 3, ADD THE VERTICAL COORD---*/
				(
					(
						(SpriteTiles[NewY][NewX].NewData & 31) /*Filter the first 5 bits*/
							+ Y /*Adds the actual pos Y*/
					)	 & 31 /*Filters back the original vertical zone.*/

				);
       		SpriteTiles[NewY][NewX].Count++;
       }
   }

   #endif
              if(VGASpritesCnt == 1)
   frame++;
	VGASpritesCnt++;
}


time_t t1= time(NULL);
time_t t2 = time(NULL);
int timer=0;
int fps =0;
int overdrawn =0;


//FILE *sos =fopen("SOS.txt","w");
//extern unsigned short ChangedMasks[200];
void MT2D_Dos_Render_VGA_ASCII(unsigned char BUFFER[][80]){


      timer++;
		t2 = time(NULL);
		if (abs(t2)-abs(t1) > 0) {

			fps = timer;
			timer = 0;
			t1 = time(NULL);
		}

     // fprintf(sos,"OVERDRAWN:");

/*
   32 chars for each line are stored in VGAFONT, each char has a bit representing a pixel if its on or off.
   so the ascii char is stored in a format like [4][1] where a single char stores two lines of chars (
   because in our case a char has the size of 8x4 instead of 8x8)
*/
     const int NextLine = 32;
     unsigned short screen_offset;
     unsigned short Font_Offset;
     unsigned char tmp[8];
     unsigned char Index;
     int x,y,FontLine;
     unsigned char Y_BUFFER;/*it will not pass from 200*/
     unsigned short Offset;
     unsigned short A,B,C;
     unsigned char i;
     overdrawn =0;
     frame++;
     for(x=0;x < 80;x++){
     		for(y=0;y <200;y+=8){
         	/*Get the char that we want*/
            Y_BUFFER =  y >> 3; /* y /8 */
            #ifdef MSDOS_USE_SPRITE_TILE_RENDER
            if(SpriteTiles[Y_BUFFER][x].NewData != SpriteTiles[Y_BUFFER][x].OldData){
            	/*Something changed in thesprite render, so force this tile to update*/
               SpriteTiles[Y_BUFFER][x].OldData = SpriteTiles[Y_BUFFER][x].NewData;
               VGA_ASCII_Backup[Y_BUFFER][x] = 255;
               overdrawn++;
            }
            #endif
         	if(VGA_ASCII_Backup[Y_BUFFER][x] != BUFFER[Y_BUFFER][x]){
            	screen_offset = y*320  + x ; /* *2 */
               Index = BUFFER[Y_BUFFER][x];
               VGA_ASCII_Backup[Y_BUFFER][x] = Index;
               Offset = (Index%32) + ((Index >>5) <<7);
					/*copy all the 8 lines from the char to the screen, just remember that in this case a char has 2 lines stored on it*/
              	for(i =0; i < 8; i+=2){
               	/* get a filtered bit and transform it into  the corresponding palette that it should be using*/
						/*       Xoffset      Yoffset */

                 // Offset = (Index%32) + ((Index /32)*32) + (i * 32);
               	//Offset = (Index %32) + ((i +(Index >> 5/* /32*/)) << 5) /* *5 */;

               	tmp[0] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 7) & 1 /*10000000 -> 00000001*/];
               	tmp[1] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 6) & 1 /*x1000000 -> 00000001*/];
               	tmp[2] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 5) & 1 /*xx100000 -> 00000001*/];
              		tmp[3] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 4) & 1 /*xxx10000 -> 00000001*/];
               	tmp[4] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 3) & 1 /*xxxx1000 -> 00000001*/];
               	tmp[5] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 2) & 1 /*xxxxx100 -> 00000001*/];
               	tmp[6] =  VGA_ASCII_COLORS[(VGAFONT[Offset] >> 1) & 1 /*xxxxxx10 -> 00000001*/];
               	tmp[7] =  VGA_ASCII_COLORS[ VGAFONT[Offset]       & 1 /*xxxxxxx1 -> 00000001*/];
                Offset= Offset + 32;
                #ifdef MSDOS_USE_SPRITE_TILE_RENDER
                if(SpriteTiles[Y_BUFFER][x].Count)
                	RenderSpritesTile(tmp,&SpriteTiles[Y_BUFFER][x],x,y + i);
                #endif

                /*Draw the first line of the the char stored in a byte*/
                #ifdef MSDOS_USE_DOUBLE_BUFFER
 //               ChangedMasks[y] =1;
 //               ChangedMasks[y+1] = 1;
 //               ChangedMasks[y+2] = 1;
 //               ChangedMasks[y+3] = 1;
                memcpy(&DOUBLE_BUFFER[/*vertical jump*/screen_offset +  /*horizontal jump*/3*x],&tmp,4);
                #else
                memcpy(&VGA[/*vertical jump*/screen_offset +  /*horizontal jump*/3*x],&tmp,4);
                #endif
               	screen_offset+= 320;
               	/*Draw the second line of the char stored in a byte*/
                #ifdef MSDOS_USE_DOUBLE_BUFFER
               	memcpy(&DOUBLE_BUFFER[/*vertical jump*/screen_offset +  /*horizontal jump*/3*x],&tmp[4],4);
               	#else
               	memcpy(&VGA[/*vertical jump*/screen_offset +  /*horizontal jump*/3*x],&tmp[4],4);
               	#endif

               	screen_offset+= 320;
               }
            }
         }
     }
     /*After all the text is rendered we now render the sprites*/
     #ifndef MSDOS_USE_SPRITE_TILE_RENDER
     MT2D_Dos_Render_Sprites();
     #endif
     #ifdef MSDOS_USE_DOUBLE_BUFFER
//     memcpy(&VGA[0],&DOUBLE_BUFFER[0],64000);
     y=0;
     screen_offset=0;
     while(y<4){
     memcpy(&VGA[screen_offset],&DOUBLE_BUFFER[screen_offset],16000);
     screen_offset+=16000;
     	y++;
     }
     #endif
     //fprintf(sos,"%d\n",overdrawn);
   //  fprintf(test,"%d\n",fps);
 }

#if !defined(MSDOS_USE_SPRITE_TILE_RENDER)
/* Render all the sprites requested to be rendered by the code*/
void MT2D_Dos_Render_Sprites()
{
    int imgX,imgY,i;
    unsigned short bitmap_offset;
    unsigned short screen_offset;
    unsigned short ImgFinal;
    unsigned char *data;
    int ImgMemArea[2];/*Start and the end representation from a horizontal line thats going to be selected*/
    unsigned int ActualSprite=0;
    int Yoffset[2];
    int Xoffset[2];

    while(ActualSprite < VGASpritesCnt){
        i=0;
        bitmap_offset =0;

        screen_offset = ((VGASpriteY[ActualSprite] < 0 ? 0 : VGASpriteY[ActualSprite])<<8)+((VGASpriteY[ActualSprite] < 0 ? 0 : VGASpriteY[ActualSprite])<<6)+(VGASpriteX[ActualSprite] < 0 ? 0 : VGASpriteX[ActualSprite]);
        ImgFinal = 0;
        data = VGASpritesToRender[ActualSprite].Data[0];
        i=0;
        if(VGASpriteY[ActualSprite]< 0){
            Yoffset[0] = -VGASpriteY[ActualSprite]; // we jump the vertical part of the sprite thats outside the screen
        }
        else{
            Yoffset[0] = 0; //no need to skip
        }
        if(VGASpriteX[ActualSprite]< 0){
            Xoffset[0] = -VGASpriteX[ActualSprite]; // we jump the horizontal part of the sprite thats outside the screen
        }
        else{
            Xoffset[0] = 0; //no need to skip
        }
        /*if the sprite start is after the top right of the screen area, the srite code will filter that and will not send it to render here*/
        if(VGASpriteY[ActualSprite]+VGASpritesToRender[ActualSprite].size.Y> 200){
            //the final part of the sprite will go below the screen, so we'll filter that
            Yoffset[1] = 200 - VGASpriteY[ActualSprite];
        }
        else{
            Yoffset[1] = VGASpritesToRender[ActualSprite].size.Y;
        }
        if(VGASpriteX[ActualSprite]+VGASpritesToRender[ActualSprite].size.X> 320){
            //the final part of the sprite will go below the screen, so we'll filter that
            Xoffset[1] = 320 -VGASpriteX[ActualSprite];
        }
        else{
            Xoffset[1] = VGASpritesToRender[ActualSprite].size.X;
        }
        Yoffset[0]*VGASpritesToRender[ActualSprite].size.X - Xoffset[0];
        /*Jump the vertical data that's before the vertical line of the video ram*/
        i = Yoffset[0]*VGASpritesToRender[ActualSprite].size.X;
        bitmap_offset = i;
        for(imgY=Yoffset[0];imgY<Yoffset[1];imgY++){
            ImgMemArea[0] = -1;
            ImgMemArea[1] = -1;
            /*Jump the next horizontal data that's before the horizontal line of the video ram*/
            i+=Xoffset[0];
           // bitmap_offset+=Xoffset[0];
            for(imgX=Xoffset[0];imgX < Xoffset[1];imgX++,i++){
              //	 printf("%d,",data[i]);
                if (data[i] !=TransparentPallete){
                    if(ImgMemArea[0] == -1){
                        ImgMemArea[0] = imgX;
                    }
                }else if(ImgMemArea[0] != -1){
                    ImgMemArea[1] = imgX;
                    if(ImgMemArea[1] == ImgMemArea[0]){
                        ImgMemArea[1]++;
                    }
                    #ifdef MSDOS_USE_DOUBLE_BUFFER
                    memcpy(&DOUBLE_BUFFER[screen_offset+ImgMemArea[0]-Xoffset[0]],&data[/*bitmap_offset+ImgMemArea[0]*/i-(ImgMemArea[1]-ImgMemArea[0])],ImgMemArea[1]-ImgMemArea[0]);
                    #else
                    memcpy(&VGA[screen_offset+ImgMemArea[0]-Xoffset[0]],&data[/*bitmap_offset+ImgMemArea[0]*/i-(ImgMemArea[1]-ImgMemArea[0])],ImgMemArea[1]-ImgMemArea[0]);
                    #endif
                    ImgMemArea[0] = -1;
                    ImgMemArea[1] = -1;
                }
            }
            if(ImgMemArea[0] != -1){   //a full line is ready to be rendered
                #ifdef MSDOS_USE_DOUBLE_BUFFER
                memcpy(&DOUBLE_BUFFER[screen_offset+ImgMemArea[0]-Xoffset[0]],&data[bitmap_offset+ImgMemArea[0]/*+Xoffset[0]*/],Xoffset[1]-ImgMemArea[0]);
                #else
                memcpy(&VGA[screen_offset+ImgMemArea[0]-Xoffset[0]],&data[bitmap_offset+ImgMemArea[0]/*+Xoffset[0]*/],Xoffset[1]-ImgMemArea[0]);
                #endif
                ImgMemArea[0] = -1;
            }
            screen_offset+=320;
            bitmap_offset+=VGASpritesToRender[ActualSprite].size.X;
            i+= (VGASpritesToRender[ActualSprite].size.X-Xoffset[1]);
        }
        ActualSprite++;
    }
}

#else
void RenderSpritesTile(unsigned char *Tmp,SpritesTilesRender *SpriteTile, int tileX,int scrYoffset){
	int PixelsNotRendered ;
	int SpritClipX[2],SpritClipY[2];/*Start and End where the sprite was going to be rendered on screen*/
	int Id;
	int i,j;
	int PreviousSprite;
	unsigned short PixelOffset;
	int GridOffset;
	int tileXtoPixel;
	unsigned char *data;
	PreviousSprite = SpriteTile->Count -1;
	PixelsNotRendered = 8;
	tileXtoPixel = (tileX << 2);
	unsigned char Tmp2[8] = {0,0,0,0,0,0,0,0};/*Rendered sprites clip*/
	while(PreviousSprite != -1 && PixelsNotRendered > 0){
		Id = SpriteTile->Ids[PreviousSprite]; /*We go from the top to below to avoid overdrawn*/
		/*There's a sprite touching a 4x8 but it may not be touching one of the 4x2 sub grid so the next if filter that kind of sprites*/
		if(tileXtoPixel + 4 > VGASpriteX[Id] && scrYoffset +  2 > VGASpriteY[Id] &&
		tileXtoPixel     < VGASpriteX[Id] + VGASpritesToRender[Id].size.X && scrYoffset <  VGASpriteY[Id] + VGASpritesToRender[Id].size.Y
		){
			/*Limit the start/end of the sprite in case it ends before the Tile End*/
			SpritClipX[0] = (tileXtoPixel < VGASpriteX[Id] ?  VGASpriteX[Id] : tileXtoPixel);
			SpritClipX[1] = (tileXtoPixel + 4 > VGASpriteX[Id] + VGASpritesToRender[Id].size.X ? VGASpriteX[Id] + VGASpritesToRender[Id].size.X : tileXtoPixel + 4);
			SpritClipY[0] = (scrYoffset < VGASpriteY[Id] ? VGASpriteY[Id] : scrYoffset);
			SpritClipY[1] = (scrYoffset + 2 > VGASpriteY[Id] + VGASpritesToRender[Id].size.Y ? VGASpriteY[Id] + VGASpritesToRender[Id].size.Y : scrYoffset + 2);

			i = SpritClipX[0];/*Backup from ScrX*/
			data = VGASpritesToRender[Id].Data[0]; /*Matrix to simple array convertion*/

			if(tileXtoPixel < VGASpriteX[Id]){
			GridOffset =  VGASpriteX[Id] - tileXtoPixel; /*Value goes from 0 to 4*/
			}
			else{
			GridOffset = 0;
			}
			j = GridOffset;
			PixelOffset = tileXtoPixel + GridOffset - VGASpriteX[Id];
			if(scrYoffset > VGASpriteY[Id]){			
				PixelOffset +=  (scrYoffset - VGASpriteY[Id]) * VGASpritesToRender[Id].size.X;
			}	
			while(SpritClipY[0] != SpritClipY[1]){
				while(SpritClipX[0] != SpritClipX[1]){
					/*Avoid OverDrawn*/
					if(Tmp2[j] == 0){
						/*Avoid Tranparent Pixels*/
						if(data[PixelOffset] !=0){
							PixelsNotRendered--;
							Tmp2[j] = data[PixelOffset];
							Tmp[j] = data[PixelOffset];
						}
					}
					PixelOffset++;
					SpritClipX[0]++;
					j++;
				}
				PixelOffset += VGASpritesToRender[Id].size.X  - (4 - GridOffset ) ;/*Go to the next line*/
				SpritClipX[0] = i;
				SpritClipY[0]++;
				j += 4-j + GridOffset;
			}
		}
		PreviousSprite--;
	}
}

#endif
#endif

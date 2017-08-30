#ifndef __MSDOS__
#include "../../MT2D_Terminal_Define.h"
#endif

#if defined(SDL_USE)

#include "../../SDL/MT2D_SDL_Redefine.h"
#include "../../SDL/MT2D_SDL_Event_Handler.h"
#include "../../MT2D.h"
#include <stdio.h>
#include <stdlib.h>

extern MT2D_SDL_Texture* mTexture;
extern MT2D_SDL_Rect gSpriteClips[256];
extern int FRAMEBUFFER[MAX_VER][MAX_HOR]; //used to store what was draw under the screen, (it should avoid overdrawn)
extern MT2D_SDL_Texture *CharSprite[256];
extern MT2D_SDL_Texture *CharSpriteRotated[256];
extern SDL_DisplayMode mode;
extern MT2D_SDL_Texture *OffscrBuff[2];
extern MT2D_SDL_Texture *ScreenBuffer;
extern MT2D_SDL_Events MainEvents;
extern MT2D_SDL_Window* gWindow;

void SDL_Render_Sprites();

void CheckVideoEvent() {
	MT2D_SDL_Event_Handler();
	if (MainEvents.Window_Started && MainEvents.Window_Resized) {
		if (MainEvents.Window_Resized = true){
			MainEvents.Window_Resized = false;
			mode.w = MainEvents.Window.data1;
			mode.h = MainEvents.Window.data2;
			MT2D_SDL_SetRenderTarget(MainEvents.Render, NULL);
			SDL_DestroyTexture(ScreenBuffer);
//			MainEvents.Render = MT2D_SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
//			SDL_RenderClear(MainEvents.Render);
			ScreenBuffer = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, mode.w, mode.h);
//			MT2D_SDL_SetRenderTarget(MainEvents.Render, ScreenBuffer);
//			SDL_SetRenderDrawColor(MainEvents.Render, 0, 0, 0, 255);

//			ScreenBuffer_Size.w = mode.w;
//			ScreenBuffer_Size.h = mode.h;

			for (int i = 0; i < MAX_HOR; i++) {
				for (int j = 0; j < MAX_VER; j++) {
					FRAMEBUFFER[j][i] = -1;
				}
			}
			//			MT2D_SDL_SetRenderTarget(gRenderer, ScreenBuffer);
//			SDL_RenderClear(MainEvents.Render);
//			SDL_SetRenderDrawColor(MainEvents.Render, 0, 0, 0, 255);
			//			MT2D_SDL_SetRenderTarget(gRenderer, NULL);
		}
	}
}
void CheckASCIIbelowSprites() {
	int Xstart, Xend, Ystart, Yend;
	int X;
	for (int i = 0; i < MainEvents.SpriteBuffer_Count; i++) {
		Xstart = (MAX_HOR * (MainEvents.SpriteBufferX[i]-1)) / 320;
		Xend = (MAX_HOR * (MainEvents.SpriteBufferX[i] + MainEvents.SpriteBuffer[i].scale.X +1)) / 320;
		Ystart = (MAX_VER * (MainEvents.SpriteBufferY[i]-1)) / 240;
		Yend = (MAX_VER * (MainEvents.SpriteBufferY[i] + MainEvents.SpriteBuffer[i].scale.Y + 1)) / 240;
		//now we clear the BUFFER behind the sprites
		for (; Ystart <= Yend && Ystart < MAX_VER; Ystart++) {
			for (X = Xstart; X <= Xend && X < MAX_HOR; X++) {
				FRAMEBUFFER[Ystart][X] = -2;
			}
		}

	}
}


void Clean_Render() {
	SDL_RenderClear(MainEvents.Render);
/*	MT2D_SDL_Rect renderQuad;
	renderQuad.x = 0;
	renderQuad.y = 0;
	renderQuad.w = mode.w;
	renderQuad.h = mode.h;
	MT2D_SDL_RenderCopy(MainEvents.Render, mTexture, &gSpriteClips[32], &renderQuad);
*/
}
void Render_New2(unsigned char BUFFER[][MAX_HOR]);

void Render_NewOld(unsigned char BUFFER[][MAX_HOR]) {
//	Render_New2(BUFFER);
//	return;
	int posx = 0;
	int posy = 0;
	int angle = 0;
	int NextA = 0;
	int NextB = 0;
	MT2D_SDL_Rect renderQuad;
	MT2D_SDL_Rect Original;
	void *mPixels;
	int mPitch;
	int CHAR_ResizedX = 0;
	int CHAR_ResizedY = 0;
	int Heigth;
	int Width;
	int OffsetX, OffsetY;
	char collide = 0;
	bool inverte = false;
	CheckVideoEvent();
	CheckASCIIbelowSprites();
	if (mode.h >= mode.w)
	{
		//smartphone
		angle = 0;
		inverte = true;
		Heigth = mode.w;
		Width = mode.h;
		CHAR_ResizedX =  Heigth / (MAX_VER);//afetando horizontal
		CHAR_ResizedY = Width / (MAX_HOR);//afetando a vertical
	}
	else {
		//desktop
		Heigth = mode.h;
		Width = mode.w;
		CHAR_ResizedX = Width / (MAX_HOR);
		CHAR_ResizedY = Heigth / (MAX_VER);
	}


	//Clean_Render();

	//	MT2D_SDL_RenderCopyEx(gRenderer, ScreenBuffer, &ScreenBuffer_Size, &ScreenBuffer_Size, NULL, NULL, SDL_FLIP_NONE);
	//	MT2D_SDL_SetRenderTarget(gRenderer, ScreenBuffer);

	//	SDL_RenderClear(gRenderer);
	//	SDL_SetRenderDrawColor(gRenderer, rand() % 255, rand() % 255, rand()%255, 255);
	for (posx = 0; posx < MAX_HOR; posx++) {
		NextA = 0;
		for (posy = 0; posy < MAX_VER; posy++) {
			collide = false;
			if (FRAMEBUFFER[posy][posx] == -2) {
				collide = true;
			}
			if (' ' != BUFFER[posy][posx]/*|| collide == true*/) {//avoids overdraw
				//if (collide == false) {
					FRAMEBUFFER[posy][posx] = BUFFER[posy][posx];
				//}
				Original.h = FONT_SIZEX;
				Original.w = FONT_SIZEY;
				Original.x = 0;
				Original.y = 0;
				if (mode.h >= mode.w){
					//90�
					renderQuad.x = mode.w - NextA - CHAR_ResizedX;
					renderQuad.y = NextB;
					renderQuad.w = CHAR_ResizedX;
					renderQuad.h = CHAR_ResizedY;
					NextA += CHAR_ResizedX;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSpriteRotated[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_HORIZONTAL);
				}
				else {
					renderQuad.x = NextB;
					renderQuad.y = NextA;
					renderQuad.w = CHAR_ResizedX;
					renderQuad.h = CHAR_ResizedY;
					NextA += CHAR_ResizedY;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSprite[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_NONE);
				}
			}
			else {
				NextA += (mode.h >= mode.w ? CHAR_ResizedX : CHAR_ResizedY);
			}

     		}
		if (mode.h >= mode.w) {
			NextB += CHAR_ResizedY;
		}
		else {
			NextB += CHAR_ResizedX;
		}

	}
	//	MT2D_SDL_SetRenderTarget(gRenderer, NULL);
	//	MT2D_SDL_RenderCopy(gRenderer, ScreenBuffer, &ScreenBuffer_Size, &ScreenBuffer_Size);
	SDL_Render_Sprites();
}

void Render_New(unsigned char BUFFER[][MAX_HOR]) {
	int posx = 0;
	int posy = 0;
	int angle = 0;
	int NextA = 0;
	int NextB = 0;
	MT2D_SDL_Rect renderQuad;
	MT2D_SDL_Rect Original;
	void *mPixels;
	int mPitch;
	int CHAR_ResizedX = 0;
	int CHAR_ResizedY = 0;
	int Heigth;
	int Width;
	int OffsetX, OffsetY;
	bool inverte = false;
	CheckVideoEvent();
	if (mode.h >= mode.w)
	{
		//smartphone
		angle = 0;
		inverte = true;
		Heigth = mode.w;
		Width = mode.h;
		MT2D_SDL_SetRenderTarget(MainEvents.Render, OffscrBuff[1]);
	}
	else {
		//desktop
		Heigth = mode.h;
		Width = mode.w;
		MT2D_SDL_SetRenderTarget(MainEvents.Render, OffscrBuff[0]);
	}
	Original.h = FONT_SIZEY;
	Original.w = FONT_SIZEX;
	Original.x = 0;
	Original.y = 0;

	for (posx = 0; posx < MAX_HOR; posx++) {
		NextA = 0;
		for (posy = 0; posy < MAX_VER; posy++) {
			if (FRAMEBUFFER[posy][posx] != BUFFER[posy][posx]) {//avoids overdraw
				FRAMEBUFFER[posy][posx] = BUFFER[posy][posx];
				if (mode.h >= mode.w) {
					//90�
					renderQuad.x = mode.w - NextA - FONT_SIZEX;
					renderQuad.y = NextB;
					renderQuad.w = FONT_SIZEX;
					renderQuad.h = FONT_SIZEY;
					NextA += CHAR_ResizedX;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSpriteRotated[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_HORIZONTAL);
/**/
					renderQuad.y =  posx * FONT_SIZEX;
					renderQuad.x = ((MAX_VER-1) * FONT_SIZEY) - posy * FONT_SIZEY;
					renderQuad.w = FONT_SIZEX;
					renderQuad.h = FONT_SIZEY;
					NextA += CHAR_ResizedX;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSpriteRotated[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_HORIZONTAL);
				}
				else {
					renderQuad.x = posx * FONT_SIZEX;
					renderQuad.y = posy * FONT_SIZEY;
					renderQuad.w = FONT_SIZEX;
					renderQuad.h = FONT_SIZEY;
					NextA += CHAR_ResizedY;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSprite[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_NONE);
				}
			}

		}
	}
	MT2D_SDL_SetRenderTarget(MainEvents.Render, NULL);
	if (!inverte) {
		Original.h = FONT_SIZEY*MAX_VER;
		Original.w = FONT_SIZEX*MAX_HOR;
		Original.x = 0;
		Original.y = 0;
		renderQuad.x = 0;
		renderQuad.y = 0;
		renderQuad.w = mode.w;
		renderQuad.h = mode.h;
		MT2D_SDL_RenderCopy(MainEvents.Render, OffscrBuff[0], &Original, &renderQuad);
	}
	else {
		Original.w = FONT_SIZEY*MAX_VER;
		Original.h = FONT_SIZEX*MAX_HOR;
		Original.x = 0;
		Original.y = 0;
		renderQuad.x = 0;
		renderQuad.y = 0;
		renderQuad.w = mode.w;
		renderQuad.h = mode.h;
		MT2D_SDL_RenderCopy(MainEvents.Render, OffscrBuff[1], &Original, &renderQuad);
	}
	SDL_Render_Sprites();
}

void SDL_Render() {
	MT2D_SDL_RenderPresent(MainEvents.Render);
}


void SDL_Add_ImagetoBuffer(Sprite *IMG,int X, int Y) {
	if (MainEvents.SpriteBuffer_Count == 0) {
		MainEvents.SpriteBuffer = (Sprite*)malloc(sizeof(Sprite));
		MainEvents.SpriteBufferX = (int*)malloc(sizeof(int));
		MainEvents.SpriteBufferY = (int*)malloc(sizeof(int));
		MainEvents.SpriteBuffer[MainEvents.SpriteBuffer_Count] = *IMG;
		MainEvents.SpriteBufferX[MainEvents.SpriteBuffer_Count] = X;
		MainEvents.SpriteBufferY[MainEvents.SpriteBuffer_Count] = Y;
	}
	else {
		MainEvents.SpriteBuffer = (Sprite*)realloc(MainEvents.SpriteBuffer,(MainEvents.SpriteBuffer_Count+1)*sizeof(Sprite));
		MainEvents.SpriteBufferY = (int*)realloc(MainEvents.SpriteBufferY, (MainEvents.SpriteBuffer_Count + 1) * sizeof(int));
		MainEvents.SpriteBufferX = (int*)realloc(MainEvents.SpriteBufferX, (MainEvents.SpriteBuffer_Count + 1) * sizeof(int));
		MainEvents.SpriteBuffer[MainEvents.SpriteBuffer_Count] = *IMG;
		MainEvents.SpriteBuffer[MainEvents.SpriteBuffer_Count] = *IMG;
		MainEvents.SpriteBufferX[MainEvents.SpriteBuffer_Count] = X;
		MainEvents.SpriteBufferY[MainEvents.SpriteBuffer_Count] = Y;
	}
	MainEvents.SpriteBuffer_Count++;
}


void SDL_Render_Sprites() {
	MT2D_SDL_Rect renderQuad;
	MT2D_SDL_Rect Original;
	SDL_Point Center;
	void *mPixels;
	int i = 0;
	int angle = 0;
	while (i < MainEvents.SpriteBuffer_Count) {
		if (mode.h >= mode.w) {
			Original.h = MainEvents.SpriteBuffer[i].size.X;
			Original.w = MainEvents.SpriteBuffer[i].size.Y;
			Original.x = 0;
			Original.y = 0;
			//90�
			renderQuad.x = (( 320*(240 - MainEvents.SpriteBuffer[i].scale.Y - MainEvents.SpriteBufferY[i])/240 ) * mode.w) / 320;
			renderQuad.y = ( (240*MainEvents.SpriteBufferX[i])/ 320  * mode.h) / 240;

//			renderQuad.y = MainEvents.SpriteBuffer[i].scale.X /2  +((MainEvents.SpriteBufferX[i] + 0) * mode.h) / 240;
			renderQuad.h = (MainEvents.SpriteBuffer[i].scale.X * mode.h ) / 320;
			renderQuad.w = (MainEvents.SpriteBuffer[i].scale.Y * mode.w ) / 240;
			MT2D_SDL_RenderCopyEx(MainEvents.Render, (MT2D_SDL_Texture*)MainEvents.SpriteBuffer[i].RotatedTexture, &Original, &renderQuad,0,0, SDL_FLIP_HORIZONTAL);
		}
 		else {
			Original.h = MainEvents.SpriteBuffer[i].size.Y;
			Original.w = MainEvents.SpriteBuffer[i].size.X;
			Original.x = 0;
			Original.y = 0;
			renderQuad.x = (MainEvents.SpriteBufferX[i] * mode.w) / 320;
			renderQuad.y = (MainEvents.SpriteBufferY[i] * mode.h) / 240;
			renderQuad.w = (MainEvents.SpriteBuffer[i].scale.X * mode.w ) / 320;
			renderQuad.h = (MainEvents.SpriteBuffer[i].scale.Y * mode.h) / 240;
			MT2D_SDL_RenderCopyEx(MainEvents.Render, (MT2D_SDL_Texture*)MainEvents.SpriteBuffer[i].Data, &Original, &renderQuad, 0, NULL, SDL_FLIP_NONE);
		}
		i++;
	}
}

/**
	Clear the sprite buffer and also the backup from the ASCII window so we can redraw everything again.
**/
void SDL_Clear_RenderBuffers() {
	if (MainEvents.SpriteBuffer) {
		free(MainEvents.SpriteBuffer);
		MainEvents.SpriteBuffer = 0;
		MainEvents.SpriteBuffer_Count = 0;
		free(MainEvents.SpriteBufferX);
		free(MainEvents.SpriteBufferY);
		MainEvents.SpriteBufferY = 0;
		MainEvents.SpriteBufferX = 0;
	}
}

#endif
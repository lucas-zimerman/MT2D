#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>


extern MT2D_SDL_Texture* mTexture;
extern MT2D_SDL_Rect gSpriteClips[256];
extern int FRAMEBUFFER[MAX_VER][MAX_HOR]; //used to store what was draw under the screen, (it should avoid overdrawn)
extern MT2D_SDL_Texture *CharSprite[256];
extern MT2D_SDL_Texture *CharSpriteRotated[256];
extern SDL_DisplayMode mode;
extern MT2D_SDL_Texture *OffscrBuff[2];

extern MT2D_SDL_Events MainEvents;


void SDL_Render_Sprites();

void CheckVideoEvent() {
	MT2D_SDL_Event_Handler();
	if (MainEvents.Window_Started && MainEvents.Window_Resized) {
		if (MainEvents.Window_Resized = true){
			MainEvents.Window_Resized = false;
			mode.w = MainEvents.Window.data1;
			mode.h = MainEvents.Window.data2;
//			ScreenBuffer_Size.w = mode.w;
//			ScreenBuffer_Size.h = mode.h;

			for (int i = 0; i < MAX_HOR; i++) {
				for (int j = 0; j < MAX_VER; j++) {
					FRAMEBUFFER[j][i] = -1;
				}
			}
			//			MT2D_SDL_SetRenderTarget(gRenderer, ScreenBuffer);
			SDL_RenderClear(MainEvents.Render);
			SDL_SetRenderDrawColor(MainEvents.Render, 0, 0, 0, 255);
			//			MT2D_SDL_SetRenderTarget(gRenderer, NULL);
		}
	}
}


void Clean_Render() {
	MT2D_SDL_Rect renderQuad;
	renderQuad.x = 0;
	renderQuad.y = 0;
	renderQuad.w = mode.w / 2;
	renderQuad.h = mode.h / 2;
	MT2D_SDL_RenderCopy(MainEvents.Render, mTexture, &gSpriteClips[32], &renderQuad);
}
void Render_New2(unsigned char BUFFER[][MAX_HOR]);
void Render_New(unsigned char BUFFER[][MAX_HOR]) {
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
	bool inverte = false;
	CheckVideoEvent();
	if (mode.h >= mode.w)
	{
		//smartphone
		angle = 0;
		inverte = true;
		Heigth = mode.w;
		Width = mode.h;
		CHAR_ResizedX = Heigth / (MAX_VER);//afetando horizontal
		CHAR_ResizedY = Width / (MAX_HOR);//afetando a vertical
	}
	else {
		//desktop
		Heigth = mode.h;
		Width = mode.w;
		CHAR_ResizedX = Width / (MAX_HOR);
		CHAR_ResizedY = Heigth / (MAX_VER);
	}




	//	MT2D_SDL_RenderCopyEx(gRenderer, ScreenBuffer, &ScreenBuffer_Size, &ScreenBuffer_Size, NULL, NULL, SDL_FLIP_NONE);
	//	MT2D_SDL_SetRenderTarget(gRenderer, ScreenBuffer);

	//	SDL_RenderClear(gRenderer);
	//	SDL_SetRenderDrawColor(gRenderer, rand() % 255, rand() % 255, rand()%255, 255);
	for (posx = 0; posx < MAX_HOR; posx++) {
		NextA = 0;
		for (posy = 0; posy < MAX_VER; posy++) {
			if (FRAMEBUFFER[posy][posx] != BUFFER[posy][posx]) {//avoids overdraw
																//FRAMEBUFFER[posy][posx] = BUFFER[posy][posx];
				Original.h = FONT_SIZEX;
				Original.w = FONT_SIZEY;
				Original.x = 0;
				Original.y = 0;
				if (mode.h >= mode.w) {
					//90º
					renderQuad.x = mode.w - NextA - CHAR_ResizedX;
					renderQuad.y = NextB;
					renderQuad.w = CHAR_ResizedX;
					renderQuad.h = CHAR_ResizedY;
					NextA += CHAR_ResizedX;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSpriteRotated[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_NONE);
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

void Render_New2(unsigned char BUFFER[][MAX_HOR]) {
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
																//FRAMEBUFFER[posy][posx] = BUFFER[posy][posx];
				if (mode.h >= mode.w) {
					//90º
					renderQuad.y = posx * FONT_SIZEX;
					renderQuad.x = posy * FONT_SIZEY;
					renderQuad.w = FONT_SIZEX;
					renderQuad.h = FONT_SIZEY;
					NextA += CHAR_ResizedX;
					MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSpriteRotated[BUFFER[posy][posx]], &Original, &renderQuad, angle, NULL, SDL_FLIP_NONE);
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
	if(!inverte){
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
	int posx = 0;
	int posy = 0;
	int angle = 0;
	int NextA = 0;
	int NextB = 0;
	int i = 0;
	MT2D_SDL_Rect renderQuad;
	MT2D_SDL_Rect Original;
	void *mPixels;
	int mPitch;
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
	}
	else {
		//desktop
		Heigth = mode.h;
		Width = mode.w;
	}
	while (i < MainEvents.SpriteBuffer_Count) {
		Original.w = MainEvents.SpriteBuffer[i].size.X;
		Original.h = MainEvents.SpriteBuffer[i].size.Y;
		Original.x = 0;
		Original.y = 0;
		if (mode.h >= mode.w) {
			//90º
			renderQuad.x = (MainEvents.SpriteBuffer[i].size.Y * mode.w) / 240;
			renderQuad.y = (MainEvents.SpriteBuffer[i].size.X * mode.h) / 320;
			renderQuad.h = (MainEvents.SpriteBuffer[i].scale.X * mode.h) / 320;
			renderQuad.w = (MainEvents.SpriteBuffer[i].scale.Y * mode.w) / 240;
			MT2D_SDL_RenderCopyEx(MainEvents.Render, (MT2D_SDL_Texture*)MainEvents.SpriteBuffer[i].Data, &Original, &renderQuad, angle, NULL, SDL_FLIP_NONE);
		}
 		else {
 			renderQuad.x = (MainEvents.SpriteBufferX[i] * mode.w) / 320;
			renderQuad.y = (MainEvents.SpriteBufferY[i] * mode.h) / 240;
			renderQuad.w = (MainEvents.SpriteBuffer[i].scale.X * mode.w) / 320;
			renderQuad.h = (MainEvents.SpriteBuffer[i].scale.Y * mode.h) / 240;
			MT2D_SDL_RenderCopyEx(MainEvents.Render, (MT2D_SDL_Texture*)MainEvents.SpriteBuffer[i].Data, &Original, &renderQuad, angle, NULL, SDL_FLIP_NONE);
		}
		i++;
	}
}

void SDL_Clear_Sprites() {
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
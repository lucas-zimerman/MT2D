#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>


extern MT2D_SDL_Texture* mTexture;
extern MT2D_SDL_Rect gSpriteClips[256];
extern int FRAMEBUFFER[MAX_VER][MAX_HOR]; //used to store what was draw under the screen, (it should avoid overdrawn)
extern MT2D_SDL_Texture *CharSprite[256];
extern MT2D_SDL_Texture *CharSpriteRotated[256];
extern SDL_DisplayMode mode;

extern MT2D_SDL_Events MainEvents;

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

}

void SDL_Render() {
	MT2D_SDL_RenderPresent(MainEvents.Render);
}
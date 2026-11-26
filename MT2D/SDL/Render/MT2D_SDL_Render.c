#include <MT2D/MT2D_Terminal_Define.h>

#if defined(SDL_USE)

#include "../../SDL/MT2D_SDL_Redefine.h"
#include "../../SDL/MT2D_SDL_Event_Handler.h"
#include "../../SDL/MT2D_SDL_Internal_Image.h"
#include "../../MT2D.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void SDL_Render_Sprites(void);

void CheckVideoEvent(void) {
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
void CheckASCIIbelowSprites(void) {
	int Xstart, Xend, Ystart, Yend;
	int X;
	for (int i = 0; i < MainEvents.SpriteBuffer_Count; i++) {
		Xstart = (MAX_HOR * (MainEvents.SpriteBufferX[i]-1)) / 320;
		Xend = (MAX_HOR * (MainEvents.SpriteBufferX[i] + MainEvents.SpriteBuffer[i].ScaleX +1)) / 320;
		Ystart = (MAX_VER * (MainEvents.SpriteBufferY[i]-1)) / 240;
		Yend = (MAX_VER * (MainEvents.SpriteBufferY[i] + MainEvents.SpriteBuffer[i].ScaleY + 1)) / 240;
		//now we clear the BUFFER behind the sprites
		for (; Ystart <= Yend && Ystart < MAX_VER; Ystart++) {
			for (X = Xstart; X <= Xend && X < MAX_HOR; X++) {
				FRAMEBUFFER[Ystart][X] = -2;
			}
		}

	}
}


void Clean_Render(void) {
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

void SDL_Render(void) {
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


void SDL_Render_Sprites(void) {
	MT2D_SDL_Rect renderQuad;
	MT2D_SDL_Rect Original;
	SDL_Point Center;
	void *mPixels;
	int i = 0;
	int angle = 0;
	while (i < MainEvents.SpriteBuffer_Count) {
		if (mode.h >= mode.w) {
			Original.h = MainEvents.SpriteBuffer[i].SizeX;
			Original.w = MainEvents.SpriteBuffer[i].SizeY;
			Original.x = 0;
			Original.y = 0;
			//90�
			renderQuad.x = (( 320*(240 - MainEvents.SpriteBuffer[i].ScaleY - MainEvents.SpriteBufferY[i])/240 ) * mode.w) / 320;
			renderQuad.y = ( (240*MainEvents.SpriteBufferX[i])/ 320  * mode.h) / 240;

//			renderQuad.y = MainEvents.SpriteBuffer[i].ScaleX /2  +((MainEvents.SpriteBufferX[i] + 0) * mode.h) / 240;
			renderQuad.h = (MainEvents.SpriteBuffer[i].ScaleX * mode.h ) / 320;
			renderQuad.w = (MainEvents.SpriteBuffer[i].ScaleY * mode.w ) / 240;
			MT2D_SDL_RenderCopyEx(MainEvents.Render, MainEvents.SpriteBuffer[i].Image->RotatedTexture, &Original, &renderQuad,0,0, SDL_FLIP_HORIZONTAL);
		}
 		else {
			Original.h = MainEvents.SpriteBuffer[i].SizeY;
			Original.w = MainEvents.SpriteBuffer[i].SizeX;
			Original.x = 0;
			Original.y = 0;
			renderQuad.x = (MainEvents.SpriteBufferX[i] * mode.w) / 320;
			renderQuad.y = (MainEvents.SpriteBufferY[i] * mode.h) / 240;
			renderQuad.w = (MainEvents.SpriteBuffer[i].ScaleX * mode.w ) / 320;
			renderQuad.h = (MainEvents.SpriteBuffer[i].ScaleY * mode.h) / 240;
			MT2D_SDL_RenderCopyEx(MainEvents.Render, MainEvents.SpriteBuffer[i].Image->Texture, &Original, &renderQuad, 0, NULL, SDL_FLIP_NONE);
		}
		i++;
	}
}

/**
	Clear the sprite buffer and also the backup from the ASCII window so we can redraw everything again.
**/
void SDL_Clear_RenderBuffers(void) {
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


void MT2D_SDL_Clear_Main_Window(void) {
	int i = 0, j = 0;
	while (i <= MAX_VER) {
		while (j<MAX_HOR) {
			WINDOW1[i][j] = ' ';
			j++;
		}
		i++;
		j = 0;
	}
	WINDOW1[MAX_VER][MAX_HOR] = '\0';
	SDL_Clear_RenderBuffers();
}


void MT2D_SDL_Draw_Window(unsigned char (*window)[MAX_HOR]) {
	int i = 0;
	int j = 0;
	Render_New(MT2D_WINDOW_OR_MAIN(window));
	SDL_Render();
}


/* ------------------------------------------------------------------------------------------------
   Image sprites made or edited from RGBA cells (MT2D_SpriteRaw, see ObjectCore/Sprites.h).
   Every buffer here is SDL_PIXELFORMAT_RGBA32: the bytes R,G,B,A in memory order.
   The rotated texture is the transposed image (pixel (x,y) of the sprite is (y,x) there), the same
   layout MT2D_SDL_Create_Rotated_Texture makes, SDL_Render_Sprites flips it when it is drawn.
   ------------------------------------------------------------------------------------------------ */

/* w*h RGBA block (rows pitch bytes apart) -> h*w transposed packed RGBA block (malloc) */
static unsigned char *MT2D_SDL_Transpose_Rgba(const unsigned char *rgba, int w, int h, int pitch) {
	unsigned char *t = (unsigned char *)malloc((size_t)w * (size_t)h * 4);
	int x, y;
	if (!t) return 0;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			const unsigned char *s = rgba + (size_t)y * (size_t)pitch + (size_t)x * 4;
			unsigned char *d = t + ((size_t)x * (size_t)h + (size_t)y) * 4;
			d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
		}
	}
	return t;
}

#ifdef MT2D_SDL_GPU

static MT2D_SDL_Texture *MT2D_SDL_Image_New(int w, int h) {
	return GPU_CreateImage((Uint16)w, (Uint16)h, GPU_FORMAT_RGBA);
}

/* SDL_gpu takes the bytes in the format of the image: only RGBA images can be written */
static int MT2D_SDL_Image_Write(MT2D_SDL_Texture *tex, int x, int y, int w, int h, const unsigned char *rgba, int pitch) {
	GPU_Rect r;
	if (!tex || tex->format != GPU_FORMAT_RGBA) return 0;
	r.x = (float)x;
	r.y = (float)y;
	r.w = (float)w;
	r.h = (float)h;
	GPU_UpdateImageBytes(tex, &r, rgba, pitch);
	return 1;
}

#else

static MT2D_SDL_Texture *MT2D_SDL_Image_New(int w, int h) {
	SDL_Texture *tex = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
	if (tex) SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	return tex;
}

/* writes an RGBA block into a texture of any format (loaded images keep the format SDL picked for them) */
static int MT2D_SDL_Image_Write(MT2D_SDL_Texture *tex, int x, int y, int w, int h, const unsigned char *rgba, int pitch) {
	Uint32 format;
	SDL_Rect r;
	int ok;
	if (!tex || SDL_QueryTexture(tex, &format, NULL, NULL, NULL) != 0) return 0;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	if (format == SDL_PIXELFORMAT_RGBA32) {
		ok = SDL_UpdateTexture(tex, &r, rgba, pitch) == 0;
	}
	else {
		int bpp = SDL_BYTESPERPIXEL(format);
		unsigned char *buf;
		if (SDL_ISPIXELFORMAT_FOURCC(format) || bpp <= 0 || bpp > 4) return 0;
		buf = (unsigned char *)malloc((size_t)w * (size_t)h * 4);
		if (!buf) return 0;
		ok = SDL_ConvertPixels(w, h, SDL_PIXELFORMAT_RGBA32, rgba, pitch, format, buf, w * bpp) == 0 &&
			SDL_UpdateTexture(tex, &r, buf, w * bpp) == 0;
		free(buf);
	}
	return ok;
}

#endif

/* writes the block into the texture and its transposed copy into the rotated texture */
static int MT2D_SDL_Image_Write_Both(MT2D_Internal_Image *img, int x, int y, int w, int h, const unsigned char *rgba, int pitch) {
	unsigned char *t;
	int ok;
	if (!MT2D_SDL_Image_Write(img->Texture, x, y, w, h, rgba, pitch)) return 0;
	if (!img->RotatedTexture) return 1;
	t = MT2D_SDL_Transpose_Rgba(rgba, w, h, pitch);
	if (!t) return 0;
	ok = MT2D_SDL_Image_Write(img->RotatedTexture, y, x, h, w, t, h * 4);
	free(t);
	return ok;
}

MT2D_Internal_Image *MT2D_SDL_Image_Create_Rgba(int w, int h, const unsigned char *rgba) {
	MT2D_Internal_Image *img;
	if (w <= 0 || h <= 0 || !rgba) return 0;
	img = (MT2D_Internal_Image *)malloc(sizeof(MT2D_Internal_Image));
	if (!img) return 0;
	img->Texture = MT2D_SDL_Image_New(w, h);
	img->RotatedTexture = MT2D_SDL_Image_New(h, w);
	if (!img->Texture || !img->RotatedTexture || !MT2D_SDL_Image_Write_Both(img, 0, 0, w, h, rgba, w * 4)) {
		if (img->Texture) MT2D_SDL_DestroyTexture(img->Texture);
		if (img->RotatedTexture) MT2D_SDL_DestroyTexture(img->RotatedTexture);
		free(img);
		return 0;
	}
	return img;
}

int MT2D_SDL_Image_Update_Rgba(MT2D_Internal_Image *img, int x, int y, int w, int h, const unsigned char *rgba, int pitch) {
	if (!img || !rgba || w <= 0 || h <= 0) return 0;
	return MT2D_SDL_Image_Write_Both(img, x, y, w, h, rgba, pitch);
}

int MT2D_SDL_Image_Read_Rgba(MT2D_Internal_Image *img, int w, int h, unsigned char *out) {
#ifdef MT2D_SDL_GPU
	SDL_Surface *s, *c;
	int y;
	if (!img || !img->Texture || !out) return 0;
	s = GPU_CopySurfaceFromImage(img->Texture);
	if (!s) return 0;
	c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(s);
	if (!c) return 0;
	if (c->w != w || c->h != h) {
		SDL_FreeSurface(c);
		return 0;
	}
	for (y = 0; y < h; y++) {
		memcpy(out + (size_t)y * (size_t)w * 4, (unsigned char *)c->pixels + (size_t)y * (size_t)c->pitch, (size_t)w * 4);
	}
	SDL_FreeSurface(c);
	return 1;
#else
	/* static textures can not be read: draw it 1:1 into a temporary render target and read that */
	SDL_Renderer *r = MainEvents.Render;
	SDL_Texture *target, *old;
	SDL_BlendMode blend;
	Uint8 cr, cg, cb, ca;
	SDL_Rect dst;
	int ok;
	if (!img || !img->Texture || !out || !r || !SDL_RenderTargetSupported(r)) return 0;
	target = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
	if (!target) return 0;
	old = SDL_GetRenderTarget(r);
	SDL_GetRenderDrawColor(r, &cr, &cg, &cb, &ca);
	SDL_GetTextureBlendMode(img->Texture, &blend);
	dst.x = 0;
	dst.y = 0;
	dst.w = w;
	dst.h = h;
	ok = SDL_SetRenderTarget(r, target) == 0;
	if (ok) {
		SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
		SDL_RenderClear(r);
		/* no blending: the pixels (alpha too) are copied as they are */
		SDL_SetTextureBlendMode(img->Texture, SDL_BLENDMODE_NONE);
		ok = SDL_RenderCopy(r, img->Texture, NULL, &dst) == 0 &&
			SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_RGBA32, out, w * 4) == 0;
		SDL_SetTextureBlendMode(img->Texture, blend);
	}
	SDL_SetRenderTarget(r, old);
	SDL_SetRenderDrawColor(r, cr, cg, cb, ca);
	SDL_DestroyTexture(target);
	return ok;
#endif
}

#endif
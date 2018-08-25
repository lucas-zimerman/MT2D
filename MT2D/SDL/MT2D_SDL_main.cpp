/*#ifdef __MSDOS__
#include "../../MT2D/MT2D_Terminal_Define.h"
#else
#include "../MT2D_Terminal_Define.h"
#endif

#ifdef SDL_USE
 */
#include "MT2D_SDL_Defines.h"
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include "MT2D_SDL_Event_Handler.h"
#include "../MT2D_Debug.h"

/*GLOBAL VARS*/
int FRAMEBUFFER[MAX_VER][MAX_HOR]; //used to store what was draw under the screen, (it should avoid overdrawn)
SDL_DisplayMode mode;
//The actual hardware texture
MT2D_SDL_Texture* mTexture;
//The window we'll be rendering to
MT2D_SDL_Window* gWindow = NULL;
//Scene sprites
MT2D_SDL_Rect gSpriteClips[256];
SDL_Surface* NEWloadedSurface;
MT2D_SDL_Texture *CharSprite[256] = { NULL };;
MT2D_SDL_Texture *CharSpriteRotated[256] = { NULL };
MT2D_SDL_Rect ScreenBuffer_Size;
MT2D_SDL_Texture *OffscrBuff[2];
MT2D_SDL_Texture *ScreenBuffer;
bool fullscreen = false;
SDL_Surface *surface;
extern MT2D_SDL_Events MainEvents;

#ifdef WINDOWS_TARGET
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
//#pragma comment(lib, "SDL2_ttf.lib")
#pragma comment(lib, "SDL2_mixer.lib")
#endif

#pragma region SDL_stbimage_adapted_code

void SDL_Render();
#ifdef SDL_stbimage
SDL_Surface *IMG_Load(const char *file) {
	return STBIMG_Load(file);
}

void IMG_Quit() {}  // not required

int IMG_Init(int flags) { return true; }

#define IMG_INIT_PNG true

#endif
#pragma endregion

void free_Texture() {
	//Free texture if it exists
	if (mTexture != NULL)
	{
		MT2D_SDL_DestroyTexture(mTexture);
		mTexture = NULL;
	}
}

Uint32 get_pixel32(SDL_Surface *surface, int x, int y)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	//Get the requested pixel
	return pixels[(y * surface->w) + x];
}

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *)p;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

MT2D_SDL_Texture *MT2D_SDL_Create_Rotated_Texture(SDL_Surface *Surface, MT2D_SDL_Texture *newTexture) {
	MT2D_SDL_Rect RotateClip;
	RotateClip.h = Surface->h;
	RotateClip.w = Surface->w;
	RotateClip.x = 0;
	RotateClip.y = 0;
	MT2D_SDL_Rect RotateClip2;
	RotateClip2.h = Surface->w;
	RotateClip2.w = Surface->h;
	RotateClip2.x = 0;
	RotateClip2.y = 0;
	SDL_Surface *RotatedSurface = SDL_CreateRGBSurface(Surface->flags,Surface->h,Surface->w, Surface->format->BitsPerPixel,Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);
	SDL_SetColorKey(RotatedSurface, SDL_RLEACCEL, SDL_MapRGB(RotatedSurface->format, 0, 0xFF, 0xFF));

	for (int Y = 0; Y < Surface->h; Y++) {
		for (int X = 0; X < Surface->w; X++) {
			putpixel(RotatedSurface, Y, X, getpixel(Surface, X, Y));
		}
	}
	MT2D_SDL_Texture *StreamTexture = MT2D_SDL_CreateTextureFromSurface(MainEvents.Render, RotatedSurface);

	/*
		Textures of type SDL_TEXTUREACCESS_TARGET are lost when the screen is resized '-'
	*/

//	SDL_RenderReadPixels(MainEvents.Render, &RotateClip2, SDL_PIXELFORMAT_RGBA8888,;
	//	MT2D_SDL_BlitSurface(NEWloadedSurface, &RotateClip, tmpSurface, NULL);
// 	MT2D_SDL_Texture *TempRenderTexture = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, Surface->h, Surface->w);

//	MT2D_SDL_SetRenderTarget(MainEvents.Render, TempRenderTexture);
	/* Lets render a rotated Image into the gpu buffer*/
//	MT2D_SDL_RenderCopyEx(MainEvents.Render, newTexture, &RotateClip, &RotateClip, 90, NULL, SDL_FLIP_NONE);
//	MT2D_SDL_SetRenderTarget(MainEvents.Render, NULL);
//	MT2D_SDL_RenderCopy(MainEvents.Render, TempRenderTexture, NULL, &RotateClip);
	
	/* Since that type of buffer is prone of failure, we're gonna store that into a safer location*/
	/* And here's our savior*/
//	MT2D_SDL_Texture *StreamTexture = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, Surface->h, Surface->w);
//	void* streamingPixels;
//	int streamingPitch;
//	RotatedSurface = SDL_CreateRGBSurface(0, Surface->h, Surface->w, 24, 0, 0, 0, 0);
//	MT2D_SDL_Texture *StreamTexture = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, Surface->h, Surface->w);
//	SDL_LockTexture(StreamTexture, NULL, &streamingPixels, &streamingPitch);
//	SDL_RenderReadPixels(MainEvents.Render, NULL, SDL_PIXELFORMAT_RGBA8888, streamingPixels, streamingPitch);
////	SDL_UnlockTexture(StreamTexture);
//	SDL_DestroyTexture(TempRenderTexture);

	return StreamTexture;
}


MT2D_SDL_Texture *MT2D_SDL_Create_Texture(SDL_Surface *Surface) {
	SDL_SetColorKey(Surface, SDL_RLEACCEL, SDL_MapRGB(Surface->format, 0, 0xFF, 0xFF));
	MT2D_SDL_Texture *newTexture = MT2D_SDL_CreateTextureFromSurface(MainEvents.Render, Surface);
	return newTexture;
}


bool NewLoadFromFile(char *string) {
	//Get rid of preexisting texture
	int mWidth;
	int mHeight;
	free_Texture();
	int TAccess = SDL_TEXTUREACCESS_STREAMING;
	MT2D_SDL_Rect RotateClip;

	RotateClip.h = FONT_SIZEY;
	RotateClip.w = FONT_SIZEX;
	RotateClip.x = 0;
	RotateClip.y = 0;

	//The final texture
	MT2D_SDL_Texture* newTexture = NULL;
	SDL_Surface *tmpSurface = NULL;

	//Load image at specified path
#ifdef __ANDROID__
	SDL_RWops *File = SDL_RWFromFile(string, "r");
	NEWloadedSurface = IMG_Load_RW(File,1);
	//SDL_RWclose(File);
#else
	NEWloadedSurface = IMG_Load(string);
#endif
	if (NEWloadedSurface == NULL)
	{
//#ifdef _DEBUG
		SDL_Log( "Unable to load image %s! SDL_image Error: %s\n", string, IMG_GetError() );
//#endif
	}
	else
	{
		//Color key image
		SDL_SetColorKey(NEWloadedSurface, SDL_RLEACCEL, SDL_MapRGB(NEWloadedSurface->format, 0, 0xFF, 0xFF));
		//Create texture from surface pixels
		//SDL_TEXTUREACCESS_STREAMING
		newTexture = MT2D_SDL_CreateTextureFromSurface(MainEvents.Render, NEWloadedSurface);
		if (newTexture == NULL)
		{
			//printf( "Unable to create texture from %s! SDL Error: %s\n", string, SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = NEWloadedSurface->w;
			mHeight = NEWloadedSurface->h;
		}
		int hor = 0, ver = 0;

		ScreenBuffer_Size.x = 0;
		ScreenBuffer_Size.y = 0;
		ScreenBuffer_Size.w = mode.w;
		ScreenBuffer_Size.h = mode.h;
		OffscrBuff[0] = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAX_HOR *FONT_SIZEX, MAX_VER * FONT_SIZEY);
		OffscrBuff[1] = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MAX_VER * FONT_SIZEY,MAX_HOR *FONT_SIZEX);
		ScreenBuffer = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,mode.w,mode.h);
		for (int i = 0; i<256; i++) {
			if (hor>256 - 8) {
				hor = 0;
				ver += 8;
			}
			gSpriteClips[i].x = hor;//posicao x
			gSpriteClips[i].y = ver;//posicao y
			gSpriteClips[i].w = FONT_SIZEX;// tamanho x
			gSpriteClips[i].h = FONT_SIZEY;// tamanho y
			hor += 8;
			tmpSurface = SDL_CreateRGBSurface(0, FONT_SIZEX, FONT_SIZEY, 24, 0, 0, 0, 0);
			MT2D_SDL_BlitSurface(NEWloadedSurface, &gSpriteClips[i], tmpSurface, NULL);
			CharSprite[i] = MT2D_SDL_CreateTextureFromSurface(MainEvents.Render, tmpSurface);
/*			CharSpriteRotated[i] = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, FONT_SIZEY, FONT_SIZEX);
			//			CharSpriteRotated[i] = SDL_CreateTextureFromSurface(gRenderer, tmpSurface);
			//			SDL_SetTextureBlendMode(CharSpriteRotated[i], SDL_BLENDMODE_BLEND);
			MT2D_SDL_SetRenderTarget(MainEvents.Render, CharSpriteRotated[i]);
			MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSprite[i], &RotateClip, &RotateClip, 90, NULL, SDL_FLIP_NONE);
			MT2D_SDL_SetRenderTarget(MainEvents.Render, NULL);
			MT2D_SDL_RenderCopy(MainEvents.Render, CharSpriteRotated[i], NULL, &RotateClip);
			*/
			CharSpriteRotated[i] = MT2D_SDL_Create_Rotated_Texture(tmpSurface, CharSprite[i]);
		}
		//Get rid of old loaded surface
		SDL_FreeSurface(NEWloadedSurface);


	}
	//Return success



	mTexture = newTexture;
	return mTexture != NULL;

}

bool loadMedia()
{
	SDL_Surface *tmpSurface = 0;
	SDL_Rect R;
	int ver = 0;
	int hor = 0;
	int dif = 0;
	//Loading success flag
	bool success = true;
#ifdef _DEBUG
	MT2D_Ide_Printf("MT2D: SDL Load tilemap char - ");
#endif;
	success = NewLoadFromFile("msdos_fnt.png");
	//Load sprite sheet texture
	if (!success) {
		success = NewLoadFromFile("../../MT2D/SDL_MT2D/msdos_fnt.png");
	}
	if (success)
	{
		R.h = 0;
		R.w = 0;
		R.x = FONT_SIZEX;
		R.y = FONT_SIZEY;
		tmpSurface = SDL_CreateRGBSurface(0, FONT_SIZEX, FONT_SIZEY, 24, 0, 0, 0, 0);
		for (int i = 0; i<256; i++) {
			if (hor>256 - 8) {
				hor = 0;
				ver += 8;
			}
			gSpriteClips[i].x = hor;//posicao x
			gSpriteClips[i].y = ver;//posicao y
			gSpriteClips[i].w = FONT_SIZEX;// tamanho x
			gSpriteClips[i].h = FONT_SIZEY;// tamanho y
			hor += 8;

		}
#ifdef _DEBUG
		MT2D_Ide_Printf("Success\n");
#endif
	}
	else {
#ifdef _DEBUG
		MT2D_Ide_Printf("Failed\n");
#endif
	}
	return success;
}

void MT2D_SDL_Init()
{
	//Initialization flag
	bool success = true;
	int i = 0, j = 0;
	MainEvents.RequestRender = false;
	MainEvents.Keyboard;
	MainEvents.Keyboard_Pressed = false;
	MainEvents.Window_Started = false;
	MainEvents.Window_Resized = false;
	MainEvents.Close_Program = false;
	MainEvents.SpriteBuffer = 0;
	MainEvents.SpriteBuffer_Count = 0;
	MainEvents.Keyboard_Buffer = 0;
	MainEvents.Keyboard_Buffer_Count = 0;

	MT2D_Ide_Printf("MT2D: Starting SDL\n");

	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		//printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
    else
	{
#ifndef MT2D_SDL_USING_DIRECTX_RENDER
//		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
//		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
//		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		//Get screen resolution
		if (SDL_GetCurrentDisplayMode(0, &mode) != 0) {
			return;
		}
		#if  !defined(MT2D_WINDOWED_MODE) || defined(MT2D_SCREEN_RESIZE) //set the screen size as the size of the screen in case of fullscreen.
//            SCREEN_WIDTH = mode.w;
 //           SCREEN_HEIGHT = mode.h;
		#endif

		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" ) )
		{
			//printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create a window display
		#ifdef MT2D_WINDOWED_MODE
            gWindow = MT2D_SDL_CreateWindow( "MT2D SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		#else
            gWindow = MT2D_SDL_CreateWindow( "MT2D SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
		#endif
			MainEvents.Render = MT2D_SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
		if( gWindow == NULL )
		{
			//printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else //load the image from the testing MT2D directory or from the actual exe folder
		{
			surface = IMG_Load("../../MT2D/SDL_MT2D/msdos.ico");
			if (!surface) {
				surface = IMG_Load("./msdos.ico");
			}
			MT2D_SDL_SetWindowIcon(gWindow,surface);
			// ...and the surface containing the icon pixel data is no longer required.
			//SDL_FreeSurface(surface);

			if(MainEvents.Render == NULL )
			{
				//printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				MT2D_SDL_SetRenderDrawColor(MainEvents.Render, 0, 0, 0, 0 );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					//printf( "SDL_image could not initialize! SDL_mage Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	if(success){
		success=loadMedia();
		SDL_GetWindowSize(gWindow, &mode.w, &mode.h);//get the window size
		for (i = 0; i < MAX_HOR; i++) {
			for (j = 0; j < MAX_VER; j++) {
				FRAMEBUFFER[j][i] = -1;
			}
		}
//#ifdef  SDL_stbimage
        #ifndef MT2D_WINDOWED_MODE
		if (mode.w > mode.h) {
			FONT_RESIZEX = (mode.w / 80) - FONT_SIZEX;
			FONT_RESIZEY = (mode.h / 25) - FONT_SIZEY;
		}
		else {

			FONT_RESIZEX = (mode.w / 25);
			FONT_RESIZEY = (mode.h / 80);
			SCREEN_MISSING_WIDTH = mode.w - FONT_RESIZEX * 25;
			SCREEN_MISSING_HEIGHT = mode.h - FONT_RESIZEY * 80;

		}
		#endif
//#endif //  SDL_stbimage
    }
}


void close()
{
	free_Texture();

	//Destroy window
	MT2D_SDL_DestroyRenderer(MainEvents.Render);
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	MainEvents.Render = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

//#else
//#pragma message("MT2D_SDL_main:You must define SDL_USE to use the SDL code")
//#endif


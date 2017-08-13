#include <MT2D/MT2D_Terminal_Define.h>

#ifdef SDL_USE
#include <MT2D/SDL/MT2D_SDL_Redefine.h>
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#include <MT2D/MT2D_Debug.h>

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
bool fullscreen = false;
SDL_Surface *surface;
extern MT2D_SDL_Events MainEvents;


#pragma region SDL_stbimage_adapted_code

void SDL_Render();
#ifdef SDL_stbimage;
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
	NEWloadedSurface = IMG_Load(string);
	if (NEWloadedSurface == NULL)
	{
		//printf( "Unable to load image %s! SDL_image Error: %s\n", string, IMG_GetError() );
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
		OffscrBuff[1]= SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, FONT_SIZEY*MAX_VER, FONT_SIZEX*MAX_HOR);
		OffscrBuff[0] = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, FONT_SIZEX*MAX_HOR, FONT_SIZEY*MAX_VER);
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
			CharSpriteRotated[i] = SDL_CreateTexture(MainEvents.Render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, FONT_SIZEY, FONT_SIZEX);
			//			CharSpriteRotated[i] = SDL_CreateTextureFromSurface(gRenderer, tmpSurface);
			//			SDL_SetTextureBlendMode(CharSpriteRotated[i], SDL_BLENDMODE_BLEND);
			MT2D_SDL_SetRenderTarget(MainEvents.Render, CharSpriteRotated[i]);
			MT2D_SDL_RenderCopyEx(MainEvents.Render, CharSprite[i], &RotateClip, &RotateClip, 90, NULL, SDL_FLIP_NONE);
			MT2D_SDL_SetRenderTarget(MainEvents.Render, NULL);
			MT2D_SDL_RenderCopy(MainEvents.Render, CharSpriteRotated[i], NULL, &RotateClip);
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
	success = NewLoadFromFile("./msdos_fnt.png");
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

	MT2D_Ide_Printf("MT2D: Starting SDL\n");

	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		//printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
    else
	{
//		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
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

void SDL_MT2DDelay(int Milliseconds) {
	SDL_Delay((Uint32)Milliseconds);
}

#else
#warning You must define "SDL_USE" to use the SDL code
#endif	


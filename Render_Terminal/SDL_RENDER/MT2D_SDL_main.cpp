#include "MT2D/MT2D_Terminal_Define.h"
#include "MT2D/Render_Terminal/SDL_RENDER/MT2D_SDL_Redefine.h"

#ifdef SDL_USE

int FRAMEBUFFER[MAX_VER][MAX_HOR]; //used to store what was draw under the screen, (it should avoid overdrawn)



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

SDL_DisplayMode mode;

int SDL_Keyboard_touched();
/*
extern int SCREEN_WIDTH;//640
extern int SCREEN_HEIGHT;//200

extern int FONT_SIZEX;//the size of the font image
extern int FONT_SIZEY;
extern int FONT_RESIZEX;//how many pixels it'll increase/decrease
extern int FONT_RESIZEY;
*/
SDL_Event Keyboard_event;

//The actual hardware texture
MT2D_SDL_Texture* mTexture;

void free_Texture();

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
MT2D_SDL_Window* gWindow = NULL;

//The window renderer
MT2D_SDL_Renderer* gRenderer = NULL;

//Scene sprites
MT2D_SDL_Rect gSpriteClips[ 256 ];
SDL_Surface* NEWloadedSurface;
MT2D_SDL_Texture *CharSprite[256] = { NULL };;

bool NewLoadFromFile(char *string){
	//Get rid of preexisting texture
	int mWidth;
	int mHeight;
	free_Texture();
	int TAccess = SDL_TEXTUREACCESS_STREAMING;

	//The final texture
	MT2D_SDL_Texture* newTexture = NULL;
	SDL_Surface *tmpSurface = NULL;

	//Load image at specified path
	NEWloadedSurface = IMG_Load(string);
	if( NEWloadedSurface == NULL )
	{
		//printf( "Unable to load image %s! SDL_image Error: %s\n", string, IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( NEWloadedSurface, SDL_RLEACCEL, SDL_MapRGB( NEWloadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
			//SDL_TEXTUREACCESS_STREAMING
        newTexture = MT2D_SDL_CreateTextureFromSurface( gRenderer, NEWloadedSurface );
		if( newTexture == NULL )
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
			tmpSurface = SDL_CreateRGBSurface(0, FONT_SIZEX, FONT_SIZEY,24,0,0,0,0);
			MT2D_SDL_BlitSurface(NEWloadedSurface, &gSpriteClips[i], tmpSurface,NULL);
			CharSprite[i] = MT2D_SDL_CreateTextureFromSurface(gRenderer, tmpSurface);
		}
		//Get rid of old loaded surface
		SDL_FreeSurface(NEWloadedSurface);


	}
	//Return success



	mTexture = newTexture;
	return mTexture != NULL;

}

void free_Texture(){
	//Free texture if it exists
	if( mTexture != NULL )
	{
		MT2D_SDL_DestroyTexture( mTexture );
		mTexture = NULL;
	}
}

void Clean_Render(){
	MT2D_SDL_Rect renderQuad;
	renderQuad.x = 0;
	renderQuad.y = 0;
	renderQuad.w = mode.w/2;
	renderQuad.h = mode.h/2;
	MT2D_SDL_RenderCopy(gRenderer, mTexture, &gSpriteClips[32], &renderQuad);
}


void Render_New(unsigned char BUFFER[][MAX_HOR]) {
	int posx = 0;
	int posy=0;
	MT2D_SDL_Rect renderQuad;
	MT2D_SDL_Rect Original;
	void *mPixels;
	int mPitch;
	for (posx = 0; posx < MAX_HOR; posx++) {
		for (posy = 0; posy < MAX_VER; posy++) {
			if (FRAMEBUFFER[posy][posx] != BUFFER[posy][posx]) {//avoids overdraw
				//FRAMEBUFFER[posy][posx] = BUFFER[posy][posx];
				Original.h = FONT_SIZEX;
				Original.w = FONT_SIZEY;
				Original.x = 0;
				Original.y = 0;
				if (mode.h >= mode.w) {
					//invert posx (posy is ok)
					posx = 79 - posx;
					renderQuad.y = posx*(FONT_RESIZEY)-9; //Idk why these magic numbers (-9 and +15 but I belive there's something related to the rotation code)
					renderQuad.x = posy*(FONT_RESIZEX)+15;
					renderQuad.h = FONT_RESIZEX;
					renderQuad.w = FONT_RESIZEY;
					//			SDL_RenderCopyEx(gRenderer, mTexture, &gSpriteClips[sprite], &renderQuad, 270, NULL, SDL_FLIP_NONE);
					MT2D_SDL_RenderCopyEx(gRenderer, CharSprite[BUFFER[posy][posx]], &Original, &renderQuad, 270, NULL, SDL_FLIP_NONE);
				}
				else {
					//X:Y = where the render is going to be
					renderQuad.x = posx*(FONT_SIZEX + FONT_RESIZEX) + SCREEN_MISSING_WIDTH;
					renderQuad.y = posy*(FONT_SIZEY + FONT_RESIZEY) + SCREEN_MISSING_HEIGHT;
					renderQuad.w = gSpriteClips[BUFFER[posy][posx]].w + FONT_RESIZEX;
					renderQuad.h = gSpriteClips[BUFFER[posy][posx]].h + FONT_RESIZEY;
					//				SDL_RenderCopy(gRenderer, mTexture, &gSpriteClips[BUFFER[posy][posx]], &renderQuad);
					MT2D_SDL_RenderCopyEx(gRenderer, CharSprite[BUFFER[posy][posx]], &Original, &renderQuad,0,NULL, SDL_FLIP_NONE);
				}
			}

		}
	}
}

bool fullscreen=false;
SDL_Surface *surface;


bool init()
{
	//Initialization flag
	bool success = true;
	int i = 0, j = 0;

	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		//printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
    else
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		//Get screen resolution
		if (SDL_GetCurrentDisplayMode(0, &mode) != 0) {
			return 1;
		}
		#ifndef MT2D_WINDOWED_MODE //set the screen size as the size of the screen in case of fullscreen.
            SCREEN_WIDTH = mode.w;
            SCREEN_HEIGHT = mode.h;
		#endif


		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" ) )
		{
			//printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create a window display
		#ifdef MT2D_WINDOWED_MODE
            gWindow = MT2D_SDL_CreateWindow( "MT2D SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
		#else
            gWindow = MT2D_SDL_CreateWindow( "MT2D SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
		#endif
		gRenderer = MT2D_SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
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

			if( gRenderer == NULL )
			{
				//printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				MT2D_SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0 );

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
	return success;
}

bool loadMedia()
{
	SDL_Surface *tmpSurface=0;
	SDL_Rect R;
	int ver=0;
	int hor=0;
	int dif=0;
	//Loading success flag
	bool success = true;

	success = NewLoadFromFile("../../MT2D/SDL_MT2D/msdos_fnt.png");
	//Load sprite sheet texture
	if(!success){
		success = NewLoadFromFile("./msdos_fnt.png");
	}
	if(success)
	{
		R.h = 0;
		R.w = 0;
		R.x = FONT_SIZEX;
		R.y = FONT_SIZEY;
		tmpSurface = SDL_CreateRGBSurface(0, FONT_SIZEX, FONT_SIZEY, 24, 0, 0, 0, 0);
		for(int i=0;i<256;i++){
				if(hor>256-8){
					hor=0;
					ver+=8;
				}
				gSpriteClips[ i ].x = hor;//posicao x
				gSpriteClips[ i ].y = ver;//posicao y
				gSpriteClips[ i ].w = FONT_SIZEX;// tamanho x
				gSpriteClips[ i ].h = FONT_SIZEY;// tamanho y
				hor+=8;
		}
		//Get rid of old loaded surface
		SDL_FreeSurface( NEWloadedSurface );
	}

	return success;
}

void close()
{
	free_Texture();

	//Destroy window
	MT2D_SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Event e;
void SDL_Render(){
	SDL_PollEvent( &e );
	MT2D_SDL_RenderPresent(gRenderer);
}

void SDL_Clear_Display(){
	MT2D_SDL_RenderClear( gRenderer );
}

int RES_MODE=1;
void SDL_ResizeScreen(int SCREEN_NEW_WIDTH,int SCREEN_NEW_HEIGHT){
	SCREEN_WIDTH = SCREEN_NEW_WIDTH;
	SCREEN_HEIGHT = SCREEN_NEW_HEIGHT;
	FONT_RESIZEX = (SCREEN_WIDTH/80)-FONT_SIZEX;
	FONT_RESIZEY = (SCREEN_HEIGHT/25)-FONT_SIZEY;
	MT2D_SDL_SetWindowSize(gWindow, SCREEN_WIDTH, SCREEN_HEIGHT );

}
void SDL_FreeWindow(){
	SDL_PollEvent( &e );
}



//bool key_SDL_pressed = false;
int key_SDL = 0;
bool SHIFT_ON = false;

bool SDL_SpecialKeyHit = false;
unsigned char SDL_EmulatedKeySpecial = 0;
unsigned char SDL_EmulatedKey = 0;
bool SDL_KeyboardHit = false;

int SDL_getkeyboard(){//keyboardhit is the function that gets the keycode, this function only sends the right key code to the progrma
	int key_code = 0;
	if (SDL_KeyboardHit == false) {// if not touched, lock the software till some key is touched
		while (SDL_Keyboard_touched() == false);
	}
	if (SDL_SpecialKeyHit) {
		SDL_SpecialKeyHit = false;
		key_code = SDL_EmulatedKeySpecial;
		if (SDL_EmulatedKey == 69) {//F11
			switch (RES_MODE) {
			case 0:
				SDL_ResizeScreen(640, 400);
				RES_MODE++;
				break;
			case 1:
				SDL_ResizeScreen(640, 480);
				RES_MODE++;
				break;
			case 2:
				SDL_ResizeScreen(800, 600);
				RES_MODE++;
				break;
			case 3:
				SDL_ResizeScreen(1024, 768);
				RES_MODE++;
				break;
			default:
				RES_MODE = 0;
				SDL_ResizeScreen(320, 240);

			}
			if (fullscreen)
				SDL_Render();
			SDL_Render();
		}
		else if (SDL_EmulatedKey == 68) { // F10
			if (!fullscreen) {
				fullscreen = true;
				MT2D_SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
				SDL_Render();
			}
			else {
				fullscreen = false;
				MT2D_SDL_SetWindowFullscreen(gWindow, 0);
				SDL_Render();

			}
		}
		SDL_EmulatedKeySpecial = false; //clears the special key bufffer...
	}
	else {
		key_code = SDL_EmulatedKey;
		SDL_KeyboardHit = false;
	}

	/*
	if(key_SDL){
		key_code=key_SDL;
		key_SDL=0;
        Keyboard_event.type = SDL_KEYUP;//this fixes the problem where keys were being pressed without being pressed
		while(SDL_PollEvent(&Keyboard_event));

	}
	else{
        if(Keyboard_event.type != SDL_KEYDOWN){
		while(SDL_Keyboard_touched()==0);
		}
        switch(Keyboard_event.type)
        {
        case SDL_KEYDOWN:
            if(Keyboard_event.key.keysym.sym>= 97 && Keyboard_event.key.keysym.sym <= 122)//letter
            {
                  if(Keyboard_event.key.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
                  {
                    key_code=Keyboard_event.key.keysym.sym-32;
                  }
                  else
                  {
                    key_code=Keyboard_event.key.keysym.sym;
                  }
            }
            if(Keyboard_event.key.keysym.sym == SDLK_LEFT){
                key_SDL=key_left;
                key_code=_key;

            }
            else if(Keyboard_event.key.keysym.sym == SDLK_RIGHT){
                key_SDL=key_right;
                key_code=_key;

            }
            else if(Keyboard_event.key.keysym.sym == SDLK_UP){
                key_SDL=key_up;
                key_code=_key;
            }
            else if(Keyboard_event.key.keysym.sym == SDLK_DOWN){
                key_SDL=key_down;
                key_code=_key;
            }else if(Keyboard_event.key.keysym.sym == SDLK_F11){
                switch(RES_MODE){
                    case 0:
                        SDL_ResizeScreen(640,200);
                        RES_MODE++;
                        break;
                    case 1:
                        SDL_ResizeScreen(640,400);
                        RES_MODE++;
                        break;
                    case 2:
                        SDL_ResizeScreen(800,600);
                        RES_MODE++;
                        break;
                    case 3:
                        SDL_ResizeScreen(1280,1024);
                        RES_MODE++;
                        break;
                    default:
                        RES_MODE=0;
                        SDL_ResizeScreen(320,200);

                }
                if(fullscreen)
                    SDL_Render();
                SDL_Render();
            }else if(Keyboard_event.key.keysym.sym == SDLK_F10){
                if(!fullscreen){
                    fullscreen=true;
                    SDL_SetWindowFullscreen(gWindow,SDL_WINDOW_FULLSCREEN);
                    SDL_Render();
                }else{
                    fullscreen=false;
                    SDL_SetWindowFullscreen(gWindow,0);
                    SDL_Render();

                }
            }
            if(Keyboard_event.key.keysym.sym>255) printf("tecla invalida %d\n",Keyboard_event.key.keysym.sym);
            else
                key_code = Keyboard_event.key.keysym.sym;
            printf("%d %c\n",key_code,key_code);
        break;
        default:
        break;
        }
    }
    if(!key_SDL){
        Keyboard_event.type = SDL_KEYUP;//this fixes the problem where keys were being pressed without being pressed
        while(SDL_PollEvent(&Keyboard_event));
    }
	*/
	return key_code;
}


int SDL_Keyboard_touched() {//check if a key was pressed, and if it was, get the key and store all the information so SDL_getkeyboard can handle what key code should MT2D send
	int output = 0;
	Keyboard_event;
	SDL_PollEvent(&Keyboard_event);
	if (Keyboard_event.type == SDL_KEYDOWN) {
		output = true;
		SDL_KeyboardHit = true;
		printf("tecla pra baixo,");
		//char 8 - 122 are equals, no need for conversion;
		//there's only lower case alphabet, so convert if shift was pressed..
		if (Keyboard_event.key.keysym.sym >= 97 && Keyboard_event.key.keysym.sym <= 122) {
			printf(" letras...\n");
			if (Keyboard_event.key.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
			{
				SDL_EmulatedKey = Keyboard_event.key.keysym.sym - 32;//transforma em maiuscula
			}
			else {
				SDL_EmulatedKey = Keyboard_event.key.keysym.sym;
			}
		}else if(Keyboard_event.key.keysym.sym <97){//espero ser sempre maior que zero
			printf(" caracter de valor menor a 97\n");

			SDL_EmulatedKey = Keyboard_event.key.keysym.sym;
		}
		else if (Keyboard_event.key.keysym.sym >= SDLK_F1 && Keyboard_event.key.keysym.sym <= SDLK_F12) {//converte as teclas F12
			printf("Tecla do tipo Fx\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = Keyboard_event.key.keysym.sym - 1073741823; // SDLK_F1 - 59
		}
		else if (Keyboard_event.key.keysym.sym == SDLK_UP) {
			printf(" seta cima\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = key_up;
		}
		else if (Keyboard_event.key.keysym.sym == SDLK_DOWN) {
			printf(" seta baixo\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = key_down;
		}
		else if (Keyboard_event.key.keysym.sym == SDLK_LEFT) {
			printf(" seta esquerda\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = key_left;
		}
		else if (Keyboard_event.key.keysym.sym == SDLK_RIGHT) {
			printf(" seta direita\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = key_right;
		}
		if (SDL_EmulatedKeySpecial == 224) {
			SDL_SpecialKeyHit = true;
		}
		SDL_WaitEvent(&Keyboard_event);
	}
	else {
		SDL_PushEvent(&Keyboard_event);//if it's not an keyboard event, drop it back into the event list
	}
	return output;
}

/*
int SDL_Keyboard_touched(){

	int key_code=0;
	SDL_PollEvent(&Keyboard_event);
	switch(Keyboard_event.type)
	{
		case SDL_KEYDOWN:
//			SDL_PushEvent(&event);// put the event back on event list for SDL_getkeyboard be able to read the event too
//			Delay(1);
			key_code = true;
		break;
		default:
			//SDL_PushEvent(NULL);
			break;
	}
	return key_code;
}
*/
void SDL_MT2DDelay(int Milliseconds) {
	SDL_Delay((Uint32)Milliseconds);
}


#endif


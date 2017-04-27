/*===================================================================
MT2D_Terminal_Define.h Created by: Lucas Zimerman Fraulob

	the purpose of this header is to identify if the developer wants
	to use the system terminal or an emulated terminal (Using SDL)

	#define SDL_USE, unmark it and MT2D will use the SDL2 for drawing
	and also sounding. (Still in beta)

	This is not the best place for doing that stuff, but it's what I
	have for now.

	#define SDL_stbimage, Used to replace SDL_IMAGE (only used at Universal Windows Platform apps or others that do not support SDL_IMAGE
	unmark this in case you're using SDL_IMAGE
	files required for SDL_stbimage:
	-https://github.com/DanielGibson/Snippets/blob/master/SDL_stbimage.h
	-https://github.com/nothings/stb

	#define MT2D_SDL_GPU, Used for replace SDL render calls by SDL_GPU
	-https://github.com/grimfang4/sdl-gpu
===================================================================*/


#define SDL_USE
//#define MT2D_SDL_GPU

//#define SDL_stbimage //Used to replace SDL_IMAGE (only used at Universal Windows Platform apps or others that do not support SDL_IMAGE

#if !defined(_WIN32) || !defined(__MSDOS__) || !defined(SDL_USE) // If there`s no way of using some form of hardware aceleration for displaying the screen,
// tell it`s using an ancient way of displaying stuff...
   #define MT2D_RENDERING_PRINTF //we are forced to use printf :(
#endif


/*===================================================================
SDL_MT2Dmain.h SDL_MT2Dmain.cpp created by: Lucas Zimerman Fraulob

	Here is all the spaghetti code that initializes and runs SDL
	for now, only the audio code is separated from the SDL main.
	you shouldn't can any of these functions directly, they should
	only be used by internal MT2D functions.

	On the actual implementation, we have keyboard, pc speaker and 
	terminal implemented into SDL, plus some audio mixing 
	implementation, some extras are that with the SDL code, you can
	go Fulscreen (pressing F12) and also change resolution (pressing
	F11) by fixed resolutions values.
	The PC Speaker code is working perfectly, the keyboard code is 
	almost ready (still buggy sometimes, works better on linux...)

	//Should be used SDL 2.0, you can use older versions but it'll 
	be required some changes into the code (it may be included on the
	future, but for now, I have other plans).

	<!> For enabling the SDL Support, go to the SDL_use.h and 
	add the following line:
	#define SDL_USE

	You may also need to include the "include folders" and the 
	"linker folders"

	On linux, just download the SDL2 packets with sudo apt-get instal...
	you'll need to add these linker commands for fully generate a SDL
	project:
	without audio (but including pc beeps): "-lSDL2 -lSDL2_image"
	with audio: "-lSDL2 -lSDL2_image -lSDL2_mixer"

	MT2D + SDL : tested on windows 10 and Ubuntu 13
	for more information about SDL, check these websites
	https://www.libsdl.org/
	https://wiki.libsdl.org/
===================================================================*/
#ifndef _MT2D_SDL_MAIN_H
#define _MT2D_SDL_MAIN_H
#include <MT2D/SDL/MT2D_SDL_Redefine.h>

void MT2D_SDL_Init();
void close();
void free_Texture();
void SDL_MT2DDelay(int Milliseconds);
MT2D_SDL_Texture *MT2D_SDL_Create_Texture(SDL_Surface *Surface);
#endif
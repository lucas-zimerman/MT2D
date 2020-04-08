/*===================================================================
SDL_MT2DAudio.h SDL_MT2DAudio.cpp created by:Lucas Zimerman Fraulob

	For more info about SDL + MT2D open the file SDL_MT2Dmain.h

	Here, you'll find functions for playing any kind of sounds
	(Using threads :D ).
	for now, complex audio will only be handled by SDL, but with a 
	cost of disabling PC SPEAKER SDL code.

	These functions should only be called from MT2D Audio core
	(because on future changes, there may be another ways of playing
	complex audios)
===================================================================*/
#ifndef HEADER_AUDIO
#define HEADER_AUDIO
	void SDL_Play_Music(char *Name);
	void SDL_Play_Sound(char *Name);
	void SDL_Start_Sound_System();
	void SDL_Close_Sound_System();
	void SDL_PlayMusicFromMemory(void* music, unsigned int size, bool free);
	void SDL_PlaySoundFromMemory(void* memoryBlock, unsigned int size, bool free);
#endif
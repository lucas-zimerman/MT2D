#ifndef HEADER_AUDIOCPP
#define HEADER_AUDIOCPP

#include "MT2D/MT2D_Terminal_Define.h"

#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef SDL_USE_AUDIO
	#ifdef linux
		#include <SDL2/SDL.h>
		#include <SDL2/SDL_audio.h>
		#include <SDL2/SDL_mixer.h>
	#elif defined _WIN32
		#include <SDL.h>
		#include <SDL_mixer.h>
#endif
#include <queue>
#include <cmath>

extern bool nosound;

bool SDL_AUDIO_STARTED = false;
Mix_Music *SDL_mus = 0;
int music_loaded = 0;
Mix_Chunk *SDL_wav = 0;
int sound_loaded = 0;

extern char SDL_SOUND_TYPE;

void SDL_Start_Sound_System()
{
    if(SDL_SOUND_TYPE == -1){
		SDL_SOUND_TYPE = 1;
        if(SDL_Init(SDL_INIT_AUDIO) < 0){
            exit(1);
        }else{
            Mix_OpenAudio(22050,AUDIO_S16SYS,2,640);
            SDL_PauseAudio(0);
        }
    }
}

#ifdef _WIN32
Mix_Music *mus = 0;

Mix_Chunk *wav = 0;
#endif

void SDL_Close_Sound_System()
{
	if (SDL_SOUND_TYPE == 1) {
		while (Mix_Playing(0) != 0);
		Mix_FreeChunk(SDL_wav);
		Mix_FreeMusic(SDL_mus);
		Mix_CloseAudio();
		SDL_Quit();
	}
}

void SDL_Play_Music(char *Name){
	if (SDL_SOUND_TYPE == 1) {
#ifdef _DEBUG
		printf("Starting SDL Play music (%s)  \n", Name);
#endif
		if (Name) {
#ifdef _DEBUG
			printf("music loaded: %d \n", music_loaded);
#endif
			if (!music_loaded) {
#ifdef _DEBUG
				printf("Entrou no if\n");
#endif

				SDL_mus = Mix_LoadMUS(Name);
				if (!SDL_mus) {
#ifdef _DEBUG
					printf("Mix_LoadMUS(\"%s\")> %s\n", Name, Mix_GetError());
#endif
				}
				music_loaded = 1;
				printf("som na caixa\n");
				Mix_PlayMusic(SDL_mus, 10);
			}
			else {
				if (SDL_mus) {
					Mix_FreeMusic(SDL_mus);
				}
				SDL_mus = Mix_LoadMUS(Name);
				if (!SDL_mus) {
#ifdef _DEBUG
					printf("Mix_LoadMUS(\"%s\")> %s\n", Name, Mix_GetError());
#endif
				}
				music_loaded = 1;
				Mix_PlayMusic(SDL_mus, 10);
			}
		}
		else {
#ifdef _DEBUG
			printf("No music name received...\n");
#endif
		}
	}
}

void SDL_Play_Sound(char *Name){
	if (SDL_SOUND_TYPE == 1) {
		if (!sound_loaded) {
			sound_loaded = 1;
			SDL_wav = Mix_LoadWAV(Name);
			Mix_PlayChannel(0, SDL_wav, 0);
		}
		else {
			sound_loaded = 1;
			if (SDL_wav) {
				Mix_FreeChunk(SDL_wav);
			}
			SDL_wav = Mix_LoadWAV(Name);
			Mix_PlayChannel(0, SDL_wav, 0);
		}
	}
}


#else
//#warning define "SDL_USE_AUDIO" must be declared to use the SDL audio code
#endif

#endif

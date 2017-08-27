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
#include <time.h>

extern bool nosound;

/**
	Private struct
**/
typedef struct MT2D_Audio_Buffer {
#ifdef SDL_USE_AUDIO
	bool SDL_AUDIO_STARTED;
	Mix_Music *SDL_music;
	int music_loaded;
	Mix_Chunk *SDL_Audio[8]; // we can play only max 8 audio files at the same time.
	time_t SDL_Audio_Started[8];
	int sound_loaded;
#endif
};

MT2D_Audio_Buffer SDL_ABuffer;
bool SDL_AUDIO_STARTED = false;

extern char SDL_SOUND_TYPE;

void SDL_Start_Sound_System()
{
	SDL_ABuffer.music_loaded = 0;
	for (int i = 0; i < 8; i++) {
		SDL_ABuffer.SDL_Audio[i] = 0;
		SDL_ABuffer.SDL_Audio_Started[i] = 0;
	}
	SDL_ABuffer.SDL_AUDIO_STARTED = 0;
	SDL_ABuffer.SDL_music = 0;
	SDL_ABuffer.sound_loaded = 0;

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

void SDL_Close_Sound_System()
{
	if (SDL_SOUND_TYPE == 1) {
		while (Mix_Playing(0) != 0);
		for (int i = 0; i < 8; i++) {
			if (SDL_ABuffer.SDL_Audio[0]) {
				Mix_FreeChunk(SDL_ABuffer.SDL_Audio[0]);
			}
		}
		Mix_FreeMusic(SDL_ABuffer.SDL_music);
		Mix_CloseAudio();
		SDL_Quit();
	}
}

void SDL_Clear_Audio_Buffer() {
	Mix_Chunk *MC;
	time_t NOW = clock();
	for (int i = 0; i < 8; i++) {
		if (SDL_ABuffer.SDL_Audio[i] != NULL && NOW-SDL_ABuffer.SDL_Audio_Started[i] > SDL_ABuffer.SDL_Audio[i]->alen) {
			//the sound has already played
			Mix_FreeChunk(SDL_ABuffer.SDL_Audio[i]);
			SDL_ABuffer.SDL_Audio[i] = 0;
			SDL_ABuffer.SDL_Audio_Started[i] = 0;
		}
	}
}

void SDL_Play_Music(char *Name){
	if (SDL_SOUND_TYPE == 1) {
#ifdef _DEBUG
		printf("Starting SDL Play music (%s)  \n", Name);
#endif
		if (Name) {
#ifdef _DEBUG
			printf("music loaded: %d \n", SDL_ABuffer.music_loaded);
#endif
			if (!SDL_ABuffer.music_loaded) {
#ifdef _DEBUG
				printf("Entrou no if\n");
#endif

				SDL_ABuffer.SDL_music = Mix_LoadMUS(Name);
				if (!SDL_ABuffer.SDL_music) {
#ifdef _DEBUG
					printf("Mix_LoadMUS(\"%s\")> %s\n", Name, Mix_GetError());
#endif
				}
				SDL_ABuffer.music_loaded = 1;
#ifdef _DEBUG
				printf("som na caixa\n");
#endif
				Mix_PlayMusic(SDL_ABuffer.SDL_music, 10);
			}
			else {
				if (SDL_ABuffer.SDL_music) {
					Mix_FreeMusic(SDL_ABuffer.SDL_music);
				}
				SDL_ABuffer.SDL_music = Mix_LoadMUS(Name);
				if (!SDL_ABuffer.SDL_music) {
#ifdef _DEBUG
					printf("Mix_LoadMUS(\"%s\")> %s\n", Name, Mix_GetError());
#endif
				}
				SDL_ABuffer.music_loaded = 1;
				Mix_PlayMusic(SDL_ABuffer.SDL_music, 10);
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
		bool Played = 0;
 		int i = 0;
 		SDL_Clear_Audio_Buffer();
		for (; i < 8; i++) {
			if (SDL_ABuffer.SDL_Audio[i] == 0) {
				SDL_ABuffer.SDL_Audio[i] = Mix_LoadWAV(Name);
				Mix_PlayChannel(i, SDL_ABuffer.SDL_Audio[i], 0);
				SDL_ABuffer.SDL_Audio_Started[i] = clock();
				Played = true;
				i = 8;
			}

		}
		if (Played == false) {
			/*All the channels are in use so*/
			/*first we check this audio is already being played*/
			Mix_Chunk *Tmp = Mix_LoadWAV(Name);
			for (i = 0; i < 8; i++) {
				if (SDL_ABuffer.SDL_Audio[i]->volume == Tmp->volume &&
					SDL_ABuffer.SDL_Audio[i]->alen == Tmp->alen &&
					SDL_ABuffer.SDL_Audio[i]->allocated == Tmp->allocated) {
					Mix_FreeChunk(Tmp);
					Mix_PlayChannel(i, SDL_ABuffer.SDL_Audio[i], 0);
					SDL_ABuffer.SDL_Audio_Started[i] = clock();
					i = 8;
					Played = true;
				}
			}
			if (Played == false) {
				/*this file isnt being played, so lets replace the sound that were being played that is near to finish*/
         				int IdChannel = 0;
				time_t NOW = clock();
				int ShortestTime = SDL_ABuffer.SDL_Audio[0]->alen - (NOW - SDL_ABuffer.SDL_Audio_Started[0]);
				for (i = 1; i < 8; i++) {
					if (SDL_ABuffer.SDL_Audio[i]->alen - (NOW - SDL_ABuffer.SDL_Audio_Started[i]) < ShortestTime)
					{
						IdChannel = i;
						ShortestTime = SDL_ABuffer.SDL_Audio[i]->alen - (NOW - SDL_ABuffer.SDL_Audio_Started[i]);
					}
				}
				Mix_FreeChunk(SDL_ABuffer.SDL_Audio[IdChannel]);
				SDL_ABuffer.SDL_Audio[IdChannel] = Tmp;
				Mix_PlayChannel(IdChannel, SDL_ABuffer.SDL_Audio[IdChannel], 0);
				SDL_ABuffer.SDL_Audio_Started[IdChannel] = clock();
			}
		}
	}
}


#else
//#warning define "SDL_USE_AUDIO" must be declared to use the SDL audio code
#endif

#endif

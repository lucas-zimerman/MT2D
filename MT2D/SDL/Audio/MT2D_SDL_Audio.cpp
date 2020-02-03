#ifndef HEADER_AUDIOCPP
#define HEADER_AUDIOCPP

#ifndef __ANDROID__
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D_Debug.h>
#else
#include "../../MT2D_Terminal_Define.h"
#endif

#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef SDL_USE_AUDIO
	#if defined(linux) || defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
		#include <SDL2/SDL.h>
		#include <SDL2/SDL_audio.h>
		#include <SDL2/SDL_mixer.h>
    #elif defined(__ANDROID__)
	#include <SDL.h>
	#include <SDL_audio.h>
	#include <SDL_mixer.h>
	#elif defined _WIN32
		#include <SDL.h>
		#include <SDL_mixer.h>
#endif
//#include <queue>
//#include <cmath>
#include <time.h>
#include <MT2D\File\MT2D_File.h>

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
	MT2D_Ide_Printf("Start Sound System");
	SDL_ABuffer.music_loaded = 0;
	for (int i = 0; i < 8; i++) {
		SDL_ABuffer.SDL_Audio[i] = 0;
		SDL_ABuffer.SDL_Audio_Started[i] = 0;
	}
	SDL_ABuffer.SDL_AUDIO_STARTED = 0;
	SDL_ABuffer.SDL_music = 0;
	SDL_ABuffer.sound_loaded = 0;
	if(SDL_SOUND_TYPE == (char)-1){
		SDL_SOUND_TYPE = 1;
        if(SDL_Init(SDL_INIT_AUDIO) < 0){
			MT2D_Ide_Printf(SDL_GetError());
			exit(1);
        }else{	
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) < 0) {
				MT2D_Ide_Printf(SDL_GetError());
			}
			if (Mix_Init(MIX_INIT_OGG) < 0) {
				MT2D_Ide_Printf(SDL_GetError());
			}
            //SDL_PauseAudio(0);
        }
    }
	MT2D_Ide_Printf("Start Sound System OK");
}

void SDL_Close_Sound_System()
{
	MT2D_Ide_Printf("Close Sound System");

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
	MT2D_Ide_Printf("Close Sound System OK");
}

void SDL_Clear_Audio_Buffer() {
	MT2D_Ide_Printf("Clear Audio Buffer");

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
	MT2D_Ide_Printf("Clear Audio Buffer OK");
}

void SDL_Play_Music(char *Name){
	MT2D_Ide_Printf(Name);
	MT2D_Ide_Printf("Music Loading");
	if (SDL_SOUND_TYPE == 1) {
		MT2D_Ide_Printf("Starting SDL Play music");
		if (Name) {
			if (!SDL_ABuffer.music_loaded) {
#ifdef __ANDROID__
                SDL_RWops *file = SDL_RWFromFile(Name,"rb");
				SDL_ABuffer.SDL_music = Mix_LoadMUS_RW(file,1);
			//	SDL_RWclose(file);
#else
				SDL_RWops* rw = SDL_RWFromFile(Name, "rb");
				if (NULL == rw) {
					MT2D_Ide_Printf(SDL_GetError());
					return;
				}

				SDL_ABuffer.SDL_music = Mix_LoadMUS_RW(rw,1);
//				SDL_ABuffer.SDL_music = Mix_LoadMUS(Name);
#endif
				if (!SDL_ABuffer.SDL_music) {
					MT2D_Ide_Printf("Mix_LoadMUS...");
					MT2D_Ide_Printf(Name);
					MT2D_Ide_Printf(Mix_GetError());
				}
				SDL_ABuffer.music_loaded = 1;
				MT2D_Ide_Printf("Starting Music");
				Mix_PlayMusic(SDL_ABuffer.SDL_music, 10);
			}
			else {
				if (SDL_ABuffer.SDL_music) {
					Mix_FreeMusic(SDL_ABuffer.SDL_music);
				}
#ifdef __ANDROID__
                SDL_RWops *file = SDL_RWFromFile(Name,"rb");
                SDL_ABuffer.SDL_music = Mix_LoadMUS_RW(file,1);
                //	SDL_RWclose(file);
#else
				MT2D_FILE* file = MT2D_FILE_OPEN(Name, "rb");
				MT2D_FILE_SEEK(file, 0, SEEK_END);
				int size = MT2D_FILE_TELL(file);
				char* buff = (char*)malloc(size * sizeof(char));
				MT2D_FILE_SEEK(file, 0, SEEK_SET);
				MT2D_FILE_READ(file, buff, size, 1);
				SDL_RWops* a = SDL_RWFromMem(buff, size);
				SDL_ABuffer.SDL_music = Mix_LoadMUS_RW(a,1);
                //SDL_ABuffer.SDL_music = Mix_LoadMUS(Name);
#endif
                if (!SDL_ABuffer.SDL_music) {
					MT2D_Ide_Printf("Mix_LoadMUS...");
					MT2D_Ide_Printf(Name);
					MT2D_Ide_Printf(Mix_GetError());
				}
				SDL_ABuffer.music_loaded = 1;
				Mix_PlayMusic(SDL_ABuffer.SDL_music, 10);
			}
		}
		else {
			MT2D_Ide_Printf("No music name received");
		}
	}
}

void SDL_Play_Sound(char *Name){
	MT2D_Ide_Printf("Loading sound...");
	MT2D_Ide_Printf(Name);
	if (SDL_SOUND_TYPE == 1) {
		bool Played = 0;
 		int i = 0;
 		SDL_Clear_Audio_Buffer();
		for (; i < 8; i++) {
			if (SDL_ABuffer.SDL_Audio[i] == 0) {
				MT2D_Ide_Printf("Found channel to load");
				SDL_ABuffer.SDL_Audio[i] = Mix_LoadWAV(Name);
				MT2D_Ide_Printf("Wav Loaded");
				Mix_PlayChannel(i, SDL_ABuffer.SDL_Audio[i], 0);
				SDL_ABuffer.SDL_Audio_Started[i] = clock();
				Played = true;
				i = 8;
			}

		}
		if (Played == false) {
			/*All the channels are in use so*/
			/*first we check this audio is already being played*/
#ifdef __ANDROID__
			SDL_RWops *file = SDL_RWFromFile(Name,"rb");
			Mix_Chunk *Tmp = Mix_LoadWAV_RW(file,1);
			//SDL_RWclose(file);
#else
			Mix_Chunk *Tmp = Mix_LoadWAV(Name);
#endif

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
	MT2D_Ide_Printf("audio played");
}


#else
//#warning define "SDL_USE_AUDIO" must be declared to use the SDL audio code
#endif

#endif

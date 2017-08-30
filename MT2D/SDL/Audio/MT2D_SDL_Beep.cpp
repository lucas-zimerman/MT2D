#ifndef _SDLBEEP
#define _SDLBEEP

#ifndef __ANDROID__
#include <MT2D/MT2D_Terminal_Define.h>
#else
#include "../../MT2D_Terminal_Define.h"
#endif

#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef SDL_USE_AUDIO
#if defined(linux) && !defined(__ANDROID__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#elif defined(__ANDROID__)
#include <SDL.h>
#include <SDL_audio.h>
#elif defined _WIN32
#include <SDL.h>
#include <SDL_audio.h>
#endif

#include <queue>
#include <cmath>


const int AMPLITUDE = 28000;
const int FREQUENCY = 44100;

extern char SDL_SOUND_TYPE;


struct BeepObject
{
	double freq;
	int samplesLeft;
};


class Beeper
{
private:
	double v;
	std::queue<BeepObject> beeps;
	bool Beep_Started ;//= false;
public:
	Beeper();
	~Beeper();
	void beep(double freq, int duration);
	void generateSamples(Sint16 *stream, int length);
	void wait();
};

void audio_callback(void*, Uint8*, int);

SDL_AudioSpec desiredSpec;
SDL_AudioSpec obtainedSpec;

Beeper::Beeper()
{
	desiredSpec.freq = FREQUENCY;
	desiredSpec.format = AUDIO_S16SYS;
	desiredSpec.channels = 1;
	desiredSpec.samples = 2048;
	desiredSpec.callback = audio_callback;
	desiredSpec.userdata = this;

	// you might want to look for errors here
//	SDL_OpenAudio(&desiredSpec, &obtainedSpec);
//	SDL_CloseAudio();
	// start play audio
//	SDL_PauseAudio(0);
}

Beeper::~Beeper()
{
	SDL_CloseAudio();
}

void Beeper::generateSamples(Sint16 *stream, int length)
{
	int i = 0;
	while (i < length) {

		if (beeps.empty()) {
			while (i < length) {
				stream[i] = 0;
				i++;
			}
			return;
		}
		BeepObject& bo = beeps.front();

		int samplesToDo = std::min(i + bo.samplesLeft, length);
		bo.samplesLeft -= samplesToDo - i;

		while (i < samplesToDo) {
			stream[i] = AMPLITUDE * (Sint16)std::sin(v * 2 * M_PI / FREQUENCY);
			i++;
			v += bo.freq;
		}

		if (bo.samplesLeft == 0) {
			beeps.pop();
		}
	}
}

void Beeper::beep(double freq, int duration)
{
	SDL_Init(SDL_INIT_AUDIO);
	if (SDL_SOUND_TYPE == -1) {
		SDL_SOUND_TYPE = 0;
		SDL_OpenAudio(&desiredSpec, &obtainedSpec);
		SDL_PauseAudio(0);
	}
	if (SDL_SOUND_TYPE == 0) {
		BeepObject bo;
		bo.freq = freq;
		bo.samplesLeft = duration * FREQUENCY / 1000;

		SDL_LockAudio();
		beeps.push(bo);
		SDL_UnlockAudio();
	}
}

void Beeper::wait()
{
	int size;
	do {
		SDL_Delay(20);
		SDL_LockAudio();
		size = beeps.size();
		SDL_UnlockAudio();
	} while (size > 0);

}

void audio_callback(void *_beeper, Uint8 *_stream, int _length)
{
	Sint16 *stream = (Sint16*)_stream;
	int length = _length / 2;
	Beeper* beeper = (Beeper*)_beeper;

	beeper->generateSamples(stream, length);
}


Beeper PC_SPEAKER_SDL;


void SDL_Beep(int frequency, int TimeMS) {
	PC_SPEAKER_SDL.beep(frequency, TimeMS);
	PC_SPEAKER_SDL.wait();
}

#else
//#e define "SDL_USE_AUDIO" must be declared to use the emulated PC SPEAKER function
#endif

#endif
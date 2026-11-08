#ifndef _SDLBEEP
#define _SDLBEEP

#include <MT2D/MT2D_Terminal_Define.h>

#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef SDL_USE_AUDIO
#if defined(linux) || defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#elif defined(__ANDROID__)
#include <SDL.h>
#include <SDL_audio.h>
#elif defined _WIN32
#include <SDL.h>
#include <SDL_audio.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <MT2D/ObjectCore/MT2D_CDecl.h> /* bool */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define AMPLITUDE 28000
#define FREQUENCY 44100

extern char SDL_SOUND_TYPE;

typedef struct BeepObject
{
	double freq;
	int samplesLeft;
} BeepObject;

/*
	The PC speaker emulation (it was the C++ class "Beeper" with a std::queue).
	SDL_Beep waits until the queue is empty, so a small ring buffer is enough.
*/
#define BEEP_QUEUE_SIZE 64

static BeepObject beeps[BEEP_QUEUE_SIZE];
static int beepsHead = 0;  /* index of the first beep */
static int beepsCount = 0; /* how many beeps are queued */
static double beeperV = 0;
static bool beeperStarted = false;

static SDL_AudioSpec desiredSpec;
static SDL_AudioSpec obtainedSpec;

static void audio_callback(void*, Uint8*, int);

static void Beeper_Close(void)
{
	SDL_CloseAudio();
}

static void Beeper_Start(void)
{
	if (beeperStarted) {
		return;
	}
	beeperStarted = true;
	desiredSpec.freq = FREQUENCY;
	desiredSpec.format = AUDIO_S16SYS;
	desiredSpec.channels = 1;
	desiredSpec.samples = 2048;
	desiredSpec.callback = audio_callback;
	desiredSpec.userdata = NULL;
	atexit(Beeper_Close);
}

static void Beeper_GenerateSamples(Sint16 *stream, int length)
{
	int i = 0;
	while (i < length) {
		BeepObject *bo;
		int samplesToDo;

		if (beepsCount == 0) {
			while (i < length) {
				stream[i] = 0;
				i++;
			}
			return;
		}
		bo = &beeps[beepsHead];

		samplesToDo = i + bo->samplesLeft;
		if (samplesToDo > length) {
			samplesToDo = length;
		}
		bo->samplesLeft -= samplesToDo - i;

		while (i < samplesToDo) {
			stream[i] = (Sint16)(AMPLITUDE * sin(beeperV * 2 * M_PI / FREQUENCY));
			i++;
			beeperV += bo->freq;
		}

		if (bo->samplesLeft == 0) {
			beepsHead = (beepsHead + 1) % BEEP_QUEUE_SIZE;
			beepsCount--;
		}
	}
}

static void Beeper_Beep(double freq, int duration)
{
	Beeper_Start();
	SDL_Init(SDL_INIT_AUDIO);
	if (SDL_SOUND_TYPE == -1) {
		SDL_SOUND_TYPE = 0;
		SDL_OpenAudio(&desiredSpec, &obtainedSpec);
		SDL_PauseAudio(0);
	}
	if (SDL_SOUND_TYPE == 0) {
		SDL_LockAudio();
		if (beepsCount < BEEP_QUEUE_SIZE) {
			BeepObject *bo = &beeps[(beepsHead + beepsCount) % BEEP_QUEUE_SIZE];
			bo->freq = freq;
			bo->samplesLeft = duration * FREQUENCY / 1000;
			beepsCount++;
		}
		SDL_UnlockAudio();
	}
}

static void Beeper_Wait(void)
{
	int size;
	do {
		SDL_Delay(20);
		SDL_LockAudio();
		size = beepsCount;
		SDL_UnlockAudio();
	} while (size > 0);
}

static void audio_callback(void *_beeper, Uint8 *_stream, int _length)
{
	Sint16 *stream = (Sint16*)_stream;
	int length = _length / 2;
	(void)_beeper;

	Beeper_GenerateSamples(stream, length);
}

void SDL_Beep(int frequency, int TimeMS) {
	Beeper_Beep(frequency, TimeMS);
#ifndef __EMSCRIPTEN__
	Beeper_Wait();
#else
	/* A browser tab cannot block: the audio callback only runs when the page returns to the browser,
	   so waiting here would freeze the page forever. The beep stays in the queue and plays by itself. */
	(void)Beeper_Wait;
#endif
}

#else
//#e define "SDL_USE_AUDIO" must be declared to use the emulated PC SPEAKER function
//without it the beep is silent (but the project still links)
void SDL_Beep(int frequency, int TimeMS) { (void)frequency; (void)TimeMS; }
#endif

#endif
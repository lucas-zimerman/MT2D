/*===================================================================
	MT2D_Mixer.h, software sound mixer (platform independent, plain C).

	Mixes a few mono voices into one output buffer. A voice plays a block of samples that is
	already in memory, either 8 bit unsigned (the WAV way) or 16 bit signed (native byte order),
	so both kinds can play at the same time. All samples must already be at the output rate:
	there is no resampling.

	The mixer does not touch the hardware. A port calls MT2D_Mixer_Render8 or MT2D_Mixer_Render16
	to fill its output (the Sound Blaster DMA half buffer on MS-DOS, a test buffer in the tests).
	Render never allocates and never does I/O, so it can run inside an interrupt handler.

	Threading: Play/Stop are main loop side, Render is timer/IRQ side. They share the voice
	table without a lock, so a port that renders from an interrupt should disable interrupts
	around Play and Stop (a voice is made active last, so a half written voice is never mixed).
===================================================================*/
#ifndef MT2D_MIXER_H
#define MT2D_MIXER_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

#ifndef MT2D_MIXER_VOICES
#define MT2D_MIXER_VOICES 3 /* at most 16, each active voice costs time inside the interrupt */
#endif
#if MT2D_MIXER_VOICES < 1 || MT2D_MIXER_VOICES > 16
#error MT2D_MIXER_VOICES must be between 1 and 16
#endif

#define MT2D_MIXER_U8  0 /* 8 bit unsigned, 128 is silence */
#define MT2D_MIXER_S16 1 /* 16 bit signed, native byte order */

#define MT2D_MIXER_LOOP 1 /* flag: restart at the end instead of stopping */

#define MT2D_MIXER_VOL_MAX 256 /* unity gain, volumes go from 0 to this value */

typedef struct MT2D_MixerVoice {
	const void *data;          /* not owned, must outlive the voice */
	unsigned long length;      /* samples (not bytes) */
	unsigned long pos;         /* next sample to play */
	unsigned long serial;      /* start order, used to steal the oldest voice */
	int volume;                /* 0..MT2D_MIXER_VOL_MAX */
	unsigned char format;      /* MT2D_MIXER_U8 / MT2D_MIXER_S16 */
	unsigned char loop;
	volatile unsigned char active;
} MT2D_MixerVoice;

typedef struct MT2D_Mixer {
	MT2D_MixerVoice voice[MT2D_MIXER_VOICES];
	unsigned long counter;     /* last serial handed out */
	int master;                /* 0..MT2D_MIXER_VOL_MAX, applied to the sum of the voices */
} MT2D_Mixer;

MT2D_C_BEGIN

/* Stops every voice and sets the master volume to unity. */
void MT2D_Mixer_Init(MT2D_Mixer *m);
/*
 * Starts a voice. data holds `samples` samples of the given format, flags is 0 or MT2D_MIXER_LOOP,
 * volume is 0..MT2D_MIXER_VOL_MAX (clamped).
 * Which voice plays it: the voice already playing the same sound (same data, length and format) restarts
 * from the beginning, so a sound fired again and again never fills the mixer; otherwise a free voice;
 * otherwise the voice with the least left to play (a looping voice counts as never ending).
 * Returns a handle for Stop/IsPlaying (>= 0) or -1 if the arguments are invalid.
 * A handle goes stale as soon as its voice is replaced: using it afterwards does nothing.
 */
int MT2D_Mixer_Play(MT2D_Mixer *m, const void *data, unsigned long samples, int format, int flags, int volume);
void MT2D_Mixer_Stop(MT2D_Mixer *m, int handle);
void MT2D_Mixer_StopAll(MT2D_Mixer *m);
/* 1 while the voice behind the handle is still playing. */
int MT2D_Mixer_IsPlaying(const MT2D_Mixer *m, int handle);
/* Number of voices playing now. */
int MT2D_Mixer_Active(const MT2D_Mixer *m);

/* Mixes n samples into out. Silence is 0 for 16 bit and 128 for 8 bit. The sum is clamped, never wraps. */
void MT2D_Mixer_Render16(MT2D_Mixer *m, short *out, int n);
void MT2D_Mixer_Render8(MT2D_Mixer *m, unsigned char *out, int n);
/* Defined at the end of MT2D_Mixer.c: lets an interrupt based port lock [MT2D_Mixer_Init, MT2D_Mixer_Code_End) in memory. */
void MT2D_Mixer_Code_End(void);

MT2D_C_END

#endif

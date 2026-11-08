/*===================================================================
	M2Sound.h, sound effects for MS-DOS (DJGPP) on the Sound Blaster (8 bit mono, DMA).

	The voices are mixed by MT2D/Audio/MT2D_Mixer.c. Here we only have:
	-the card setup (BLASTER variable, DSP reset, IRQ and DMA),
	-an auto-init DMA buffer made of two halves: every time the card finishes a half, the IRQ
	 mixes the next one (so a sound starts to be heard after at most 2 halves, about 90 ms),
	-the sound sources: a WAV in a container file, a loose WAV file or a block of memory.

	Sounds are .wav files, PCM mono, 8 or 16 bit. A rate that is not MT2D_DOS_SOUND_RATE is
	converted when the file is loaded. The cards need DSP 2.00 or newer (Sound Blaster 2.0 and up).
	Loaded sounds stay cached (MT2D_DOS_SOUND_CACHE files) so playing one again does not touch the disk.
	Defines: MT2D_DOS_SOUND_RATE (default 22050), MT2D_DOS_SOUND_HALF (samples, default 1024, multiple of 4),
	MT2D_DOS_SOUND_CACHE (default 16), MT2D_DOS_SOUND_MAX (biggest file, default 524288 bytes).
===================================================================*/
#ifndef M2SOUND_H
#define M2SOUND_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

#ifndef MT2D_DOS_SOUND_RATE
#define MT2D_DOS_SOUND_RATE 22050
#endif
#ifndef MT2D_DOS_SOUND_HALF
#define MT2D_DOS_SOUND_HALF 1024
#endif
#ifndef MT2D_DOS_SOUND_CACHE
#define MT2D_DOS_SOUND_CACHE 16
#endif
#ifndef MT2D_DOS_SOUND_MAX
#define MT2D_DOS_SOUND_MAX 524288UL
#endif

MT2D_C_BEGIN

/* Finds and starts the card. Returns 1 if there is sound support, 0 otherwise (everything else becomes a no-op). */
int MT2D_Dos_Sound_Init(void);
/* Stops the card and gives the IRQ back. Also done by atexit. */
void MT2D_Dos_Sound_Close(void);
/*
 * What MT2D_Play_Audio calls: plays "d/<name>.wav" once. The name is cut to 8 characters and its
 * extension replaced (coin.ogg -> d/coin.wav). A container file with that path wins over a loose file.
 * Returns a handle (>= 0) or -1 (no card, file not found or not playable).
 */
int MT2D_Dos_Sound_Play(const char *name);
/* Plays a WAV image that is in memory. If free_after is not 0 the block is freed once the sound is not needed. */
int MT2D_Dos_Sound_PlayMemory(void *wav, unsigned size, int free_after);
void MT2D_Dos_Sound_Stop(int handle);
void MT2D_Dos_Sound_StopAll(void);
/* Number of sounds playing now. */
int MT2D_Dos_Sound_Active(void);

MT2D_C_END

#endif

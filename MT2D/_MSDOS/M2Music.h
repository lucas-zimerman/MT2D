/*===================================================================
	M2Music.h, IMF music playback for MS-DOS (DJGPP) on the OPL2 chip.

	The song is decoded by MT2D/Audio/MT2D_Music.c. Here we only have:
	-the OPL access (M2Opl.c),
	-a timer interrupt (IRQ 0, reprogrammed to the song rate, the DOS clock keeps its speed),
	-the file sources: a container file, a loose file or a block of memory.

	The songs are not encrypted. Call MT2D_Dos_Music_Update once per frame so a streamed song
	gets refilled (it is a no-op for songs that fit the buffer).
	Buffer size: define MT2D_DOS_MUSIC_BUF (bytes, default 32768) to change it.
===================================================================*/
#ifndef M2MUSIC_H
#define M2MUSIC_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/Audio/MT2D_Music.h>

#ifndef MT2D_DOS_MUSIC_BUF
#define MT2D_DOS_MUSIC_BUF 32768
#endif

MT2D_C_BEGIN

/* Detects the OPL chip. Returns 1 if there is music support, 0 otherwise (everything else becomes a no-op). */
int MT2D_Dos_Music_Init(void);
/* Stops the song and gives the timer back to DOS. Also done by atexit. */
void MT2D_Dos_Music_Close(void);
/* Ticks per second of the next songs (default MT2D_MUSIC_DEFAULT_RATE, some songs use 560). */
void MT2D_Dos_Music_SetRate(int hz);
/*
 * Plays a song: first a container file called name, then a loose file.
 * flags: MT2D_MUSIC_LOOP / STREAM / PRELOAD. Returns MT2D_MUSIC_OK or a MT2D_MUSIC_ERR_* code.
 */
int MT2D_Dos_Music_Play(const char *name, int flags);
/*
 * What MT2D_Play_Music calls: loops the song "d/<name>.imf". The name is cut to 8 characters and its extension
 * replaced (ode.ogg -> d/ode.imf), a name that already ends with .imf is used as it is.
 */
int MT2D_Dos_Music_PlayFile(const char *name);
/* Plays a song from memory without copying it. If free_after is not 0 the block is freed when the song is replaced or stopped. */
int MT2D_Dos_Music_PlayMemory(void *data, unsigned size, int free_after, int flags);
void MT2D_Dos_Music_Stop(void);
int MT2D_Dos_Music_IsPlaying(void);
/* 1 while the current song is read from disk piece by piece, 0 if it is all in memory */
int MT2D_Dos_Music_IsStreaming(void);
/* Main loop: refills the stream buffer. */
void MT2D_Dos_Music_Update(void);

MT2D_C_END

#endif

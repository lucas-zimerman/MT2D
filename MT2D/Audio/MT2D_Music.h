/*===================================================================
	MT2D_Music.h, IMF music decoder (platform independent, plain C).

	IMF is a stream of 4 byte events: OPL register, value, delay (16 bit, little endian).
	The delay is the number of ticks to wait before the next event.
	Type 1 files start with a 16 bit data length, type 0 files (first word is 0) are only events.

	The decoder does not touch the hardware: every event is sent to a write callback (the OPL
	backend on MS-DOS, a recorder in the tests). A timer calls MT2D_Music_Tick at m->rate Hz.

	Two ways of feeding it, both behind the same player:
	-whole : the song fits the buffer, it is read once on open and never touches the disk again.
	-stream: the buffer is a ring, MT2D_Music_Update (main loop, never the timer) refills it.
	Garbage data is not detected, it just plays garbage, but it can't hang or overrun the buffer.
===================================================================*/
#ifndef MT2D_MUSIC_H
#define MT2D_MUSIC_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

#define MT2D_MUSIC_LOOP    1 /* restart at the end */
#define MT2D_MUSIC_STREAM  2 /* force stream mode (even if the song fits) */
#define MT2D_MUSIC_PRELOAD 4 /* force whole mode, Open fails if the song doesn't fit */

#define MT2D_MUSIC_DEFAULT_RATE 700 /* ticks per second, 560 for some songs */
#define MT2D_MUSIC_MAX_EVENTS   256 /* events per tick, avoids locking on zero delay garbage */

#define MT2D_MUSIC_OK         0
#define MT2D_MUSIC_ERR_ARGS  -1
#define MT2D_MUSIC_ERR_EMPTY -2 /* nothing to play */
#define MT2D_MUSIC_ERR_FIT   -3 /* PRELOAD but the song is bigger than the buffer */
#define MT2D_MUSIC_ERR_READ  -4

/* Reads up to n bytes of the song starting at pos, returns the bytes read (<= 0 on failure). */
typedef int (*MT2D_MusicReadFn)(void *ctx, long pos, unsigned char *dst, int n);
/* Receives every OPL register write. */
typedef void (*MT2D_MusicWriteFn)(unsigned char reg, unsigned char val, void *ctx);

typedef struct MT2D_Music {
	MT2D_MusicReadFn read;
	void *read_ctx;
	MT2D_MusicWriteFn write;
	void *write_ctx;
	unsigned char *buf;       /* whole song, or ring buffer */
	unsigned long buf_size;
	long data_start;          /* offset of the first event in the song */
	unsigned long data_len;   /* bytes of events */
	/* produced is written only by Update and consumed only by Tick, so no locking is needed */
	volatile unsigned long produced; /* bytes of events loaded so far */
	volatile unsigned long consumed; /* bytes of events played so far */
	volatile unsigned long wait;     /* ticks until the next event */
	volatile unsigned char playing;
	volatile unsigned char stalled;  /* stream mode: waiting for Update to rewind (loop) */
	unsigned char loop;
	unsigned char streaming;
	unsigned short rate;      /* ticks per second the timer must use */
} MT2D_Music;

MT2D_C_BEGIN

/*
 * Prepare m for a song of `size` bytes read through `read`. buf/buf_size is the storage (not freed by us).
 * The song is loaded whole when its events fit buf_size, otherwise it is streamed (buf_size >= 8).
 * It does not start playing, see MT2D_Music_Play.
 */
int MT2D_Music_Open(MT2D_Music *m, MT2D_MusicReadFn read, void *read_ctx, long size,
					unsigned char *buf, unsigned long buf_size,
					MT2D_MusicWriteFn write, void *write_ctx, int flags);
/* Zero copy: plays a song already in memory (e.g. a memory container file). data must outlive the player. */
int MT2D_Music_OpenMemory(MT2D_Music *m, const unsigned char *data, long size,
						  MT2D_MusicWriteFn write, void *write_ctx, int flags);
/* Starts (or restarts) from the beginning. */
void MT2D_Music_Play(MT2D_Music *m);
void MT2D_Music_Stop(MT2D_Music *m);
/* Timer side, call at m->rate Hz. Never does I/O. Returns 0 once the song ended. */
int MT2D_Music_Tick(MT2D_Music *m);
/* Main loop side, refills the stream buffer. Does nothing in whole mode. */
void MT2D_Music_Update(MT2D_Music *m);
/* Defined right after MT2D_Music_Tick: lets an interrupt based port lock [MT2D_Music_Tick, MT2D_Music_Tick_End) in memory. */
void MT2D_Music_Tick_End(void);

/* Read callbacks for MT2D_Music_Open */
typedef struct MT2D_MusicMem { const unsigned char *data; long size; } MT2D_MusicMem;
int MT2D_Music_ReadMemory(void *ctx, long pos, unsigned char *dst, int n);    /* ctx: MT2D_MusicMem* */
int MT2D_Music_ReadContainer(void *ctx, long pos, unsigned char *dst, int n); /* ctx: int* container file id */
int MT2D_Music_ReadFile(void *ctx, long pos, unsigned char *dst, int n);      /* ctx: MT2D_FILE* opened "rb" */

MT2D_C_END

#endif

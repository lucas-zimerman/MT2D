/*===================================================================
	MT2D_Wav.h, WAV reader for the software mixer (platform independent, plain C).

	Reads the header of a .wav file that is already in memory and tells where the samples are.
	Only what the mixer plays is accepted: PCM, mono, 8 bit unsigned or 16 bit signed.
	The samples are not copied (zero copy), MT2D_Wav_Resample converts them to another rate
	(nearest sample, no filter) so every sound can match the output rate of the mixer.
===================================================================*/
#ifndef MT2D_WAV_H
#define MT2D_WAV_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/Audio/MT2D_Mixer.h>

#define MT2D_WAV_OK           0
#define MT2D_WAV_ERR_ARGS    -1
#define MT2D_WAV_ERR_FORMAT  -2 /* not a RIFF/WAVE file, or the fmt/data chunk is missing or cut */
#define MT2D_WAV_ERR_UNSUPPORTED -3 /* not PCM mono 8/16 bit, or rate is 0 / above 65535 */

typedef struct MT2D_Wav {
	const unsigned char *data; /* first sample, inside the file image */
	unsigned long samples;     /* number of samples */
	unsigned long rate;        /* samples per second */
	int format;                /* MT2D_MIXER_U8 / MT2D_MIXER_S16 (little endian in the file) */
} MT2D_Wav;

MT2D_C_BEGIN

int MT2D_Wav_Parse(const unsigned char *file, unsigned long size, MT2D_Wav *out);
/* Samples a sound of `samples` samples at rate `from` has once converted to rate `to`. */
unsigned long MT2D_Wav_ResampledLength(unsigned long samples, unsigned long from, unsigned long to);
/*
 * Converts src (`samples` samples of `format`, bytes in the order of the file) to rate `to`.
 * dst must hold MT2D_Wav_ResampledLength(...) samples. 16 bit output is written in native byte
 * order, so it is also what the mixer wants. With from == to it only copies (and fixes the byte order).
 */
void MT2D_Wav_Resample(const unsigned char *src, unsigned long samples, int format, unsigned long from,
					   unsigned long to, void *dst);

MT2D_C_END

#endif

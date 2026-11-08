#include <stddef.h>
#include <MT2D/Audio/MT2D_Wav.h>

static unsigned long wav_u16(const unsigned char *p)
{
	return (unsigned long)p[0] | ((unsigned long)p[1] << 8);
}

static unsigned long wav_u32(const unsigned char *p)
{
	return (unsigned long)p[0] | ((unsigned long)p[1] << 8) | ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}

static int wav_tag(const unsigned char *p, const char *tag)
{
	return p[0] == (unsigned char)tag[0] && p[1] == (unsigned char)tag[1] &&
		   p[2] == (unsigned char)tag[2] && p[3] == (unsigned char)tag[3];
}

int MT2D_Wav_Parse(const unsigned char *file, unsigned long size, MT2D_Wav *out)
{
	unsigned long pos = 12;
	int have_fmt = 0;
	unsigned long rate = 0, bits = 0;
	if (file == NULL || out == NULL) {
		return MT2D_WAV_ERR_ARGS;
	}
	if (size < 12 || !wav_tag(file, "RIFF") || !wav_tag(file + 8, "WAVE")) {
		return MT2D_WAV_ERR_FORMAT;
	}
	while (size - pos >= 8) { /* pos <= size always holds here */
		unsigned long len = wav_u32(file + pos + 4);
		const unsigned char *body = file + pos + 8;
		unsigned long room = size - pos - 8;
		if (wav_tag(file + pos, "fmt ")) {
			if (len < 16 || room < 16) {
				return MT2D_WAV_ERR_FORMAT;
			}
			if (wav_u16(body) != 1) { /* PCM only */
				return MT2D_WAV_ERR_UNSUPPORTED;
			}
			if (wav_u16(body + 2) != 1) { /* mono only */
				return MT2D_WAV_ERR_UNSUPPORTED;
			}
			rate = wav_u32(body + 4);
			bits = wav_u16(body + 14);
			if (rate == 0 || rate > 65535UL || (bits != 8 && bits != 16)) {
				return MT2D_WAV_ERR_UNSUPPORTED;
			}
			have_fmt = 1;
		} else if (wav_tag(file + pos, "data")) {
			if (!have_fmt) {
				return MT2D_WAV_ERR_FORMAT;
			}
			if (len > room) {
				len = room; /* cut file (or streaming header), play what is there */
			}
			out->data = body;
			out->rate = rate;
			out->format = (bits == 8) ? MT2D_MIXER_U8 : MT2D_MIXER_S16;
			out->samples = len / (bits / 8);
			return out->samples > 0 ? MT2D_WAV_OK : MT2D_WAV_ERR_FORMAT;
		}
		if (len > room) {
			break;
		}
		pos += 8 + len + (len & 1); /* chunks are padded to an even size */
		if (pos > size) {
			break;
		}
	}
	return MT2D_WAV_ERR_FORMAT;
}

unsigned long MT2D_Wav_ResampledLength(unsigned long samples, unsigned long from, unsigned long to)
{
	unsigned long long n;
	if (from == 0 || to == 0) {
		return 0;
	}
	n = (unsigned long long)samples * to / from;
	return n == 0 ? 1 : (unsigned long)n; /* never turn a sound into nothing */
}

void MT2D_Wav_Resample(const unsigned char *src, unsigned long samples, int format, unsigned long from,
					   unsigned long to, void *dst)
{
	unsigned long n, i;
	if (src == NULL || dst == NULL || samples == 0 || from == 0 || to == 0) {
		return;
	}
	n = MT2D_Wav_ResampledLength(samples, from, to);
	for (i = 0; i < n; i++) {
		unsigned long at = (unsigned long)((unsigned long long)i * from / to);
		if (at >= samples) {
			at = samples - 1;
		}
		if (format == MT2D_MIXER_U8) {
			((unsigned char *)dst)[i] = src[at];
		} else {
			((short *)dst)[i] = (short)(unsigned short)wav_u16(src + at * 2);
		}
	}
}

#include <stddef.h>
#include <MT2D/Audio/MT2D_Mixer.h>

#define MIXER_CHUNK 64 /* samples mixed per pass, keeps the stack use small for an interrupt handler */

void MT2D_Mixer_Init(MT2D_Mixer *m)
{
	int i;
	if (m == NULL) {
		return;
	}
	for (i = 0; i < MT2D_MIXER_VOICES; i++) {
		m->voice[i].active = 0;
		m->voice[i].data = NULL;
		m->voice[i].length = 0;
		m->voice[i].pos = 0;
		m->voice[i].serial = 0;
		m->voice[i].volume = 0;
		m->voice[i].format = MT2D_MIXER_U8;
		m->voice[i].loop = 0;
	}
	m->counter = 0;
	m->master = MT2D_MIXER_VOL_MAX;
}

static int mixer_clamp_volume(int v)
{
	if (v < 0) {
		return 0;
	}
	if (v > MT2D_MIXER_VOL_MAX) {
		return MT2D_MIXER_VOL_MAX;
	}
	return v;
}

/* handle = (serial low 16 bits << 4) | voice index */
static MT2D_MixerVoice *mixer_lookup(const MT2D_Mixer *m, int handle)
{
	MT2D_MixerVoice *v;
	if (m == NULL || handle < 0) {
		return NULL;
	}
	if ((handle & 15) >= MT2D_MIXER_VOICES) {
		return NULL;
	}
	v = (MT2D_MixerVoice *)&m->voice[handle & 15];
	if (!v->active) {
		return NULL;
	}
	if (((v->serial & 0xFFFFUL) << 4) != ((unsigned long)handle & ~15UL)) {
		return NULL;
	}
	return v;
}

/* Remaining samples of a playing voice, a looping one never ends. */
static unsigned long mixer_left(const MT2D_MixerVoice *v)
{
	return v->loop ? 0xFFFFFFFFUL : v->length - v->pos;
}

/*
 * Which voice a new sound uses: the one already playing the same sound (it restarts), else a free one,
 * else the one with the least left to play (the oldest of them if it is a tie).
 */
static int mixer_pick(const MT2D_Mixer *m, const void *data, unsigned long samples, int format)
{
	int i, pick = -1, free_voice = -1;
	for (i = 0; i < MT2D_MIXER_VOICES; i++) {
		const MT2D_MixerVoice *v = &m->voice[i];
		if (!v->active) {
			if (free_voice < 0) {
				free_voice = i;
			}
			continue;
		}
		if (v->data == data && v->length == samples && v->format == format) {
			return i;
		}
		if (pick < 0 || mixer_left(v) < mixer_left(&m->voice[pick]) ||
			(mixer_left(v) == mixer_left(&m->voice[pick]) && v->serial < m->voice[pick].serial)) {
			pick = i;
		}
	}
	return free_voice >= 0 ? free_voice : pick;
}

int MT2D_Mixer_Play(MT2D_Mixer *m, const void *data, unsigned long samples, int format, int flags, int volume)
{
	int pick;
	MT2D_MixerVoice *v;
	if (m == NULL || data == NULL || samples == 0 ||
		(format != MT2D_MIXER_U8 && format != MT2D_MIXER_S16)) {
		return -1;
	}
	pick = mixer_pick(m, data, samples, format);
	v = &m->voice[pick];
	v->active = 0; /* hide it from Render while it is rewritten */
	m->counter++;
	v->data = data;
	v->length = samples;
	v->pos = 0;
	v->serial = m->counter;
	v->volume = mixer_clamp_volume(volume);
	v->format = (unsigned char)format;
	v->loop = (flags & MT2D_MIXER_LOOP) ? 1 : 0;
	v->active = 1;
	return (int)((m->counter & 0xFFFFUL) << 4) | pick;
}

void MT2D_Mixer_Stop(MT2D_Mixer *m, int handle)
{
	MT2D_MixerVoice *v = mixer_lookup(m, handle);
	if (v != NULL) {
		v->active = 0;
	}
}

void MT2D_Mixer_StopAll(MT2D_Mixer *m)
{
	int i;
	if (m == NULL) {
		return;
	}
	for (i = 0; i < MT2D_MIXER_VOICES; i++) {
		m->voice[i].active = 0;
	}
}

int MT2D_Mixer_IsPlaying(const MT2D_Mixer *m, int handle)
{
	return mixer_lookup(m, handle) != NULL;
}

int MT2D_Mixer_Active(const MT2D_Mixer *m)
{
	int i, n = 0;
	if (m == NULL) {
		return 0;
	}
	for (i = 0; i < MT2D_MIXER_VOICES; i++) {
		if (m->voice[i].active) {
			n++;
		}
	}
	return n;
}

/* Adds up to n samples of one voice into acc (acc holds sample16 * 256), advances or ends the voice. */
static void mixer_voice(MT2D_MixerVoice *vo, long *acc, int n)
{
	while (vo->active && n > 0) {
		unsigned long avail = vo->length - vo->pos;
		int take = n;
		int i;
		if ((unsigned long)take > avail) {
			take = (int)avail;
		}
		if (vo->format == MT2D_MIXER_U8) {
			const unsigned char *s = (const unsigned char *)vo->data + vo->pos;
			long gain = (long)vo->volume << 8; /* (s - 128) is already 8 bits short of a 16 bit sample */
			for (i = 0; i < take; i++) {
				acc[i] += ((long)s[i] - 128L) * gain;
			}
		} else {
			const short *s = (const short *)vo->data + vo->pos;
			long gain = (long)vo->volume;
			for (i = 0; i < take; i++) {
				acc[i] += (long)s[i] * gain;
			}
		}
		acc += take;
		n -= take;
		vo->pos += (unsigned long)take;
		if (vo->pos >= vo->length) {
			if (vo->loop) {
				vo->pos = 0; /* length > 0, so this always makes progress */
			} else {
				vo->active = 0;
			}
		}
	}
}

/* Mixes one chunk and returns the clamped 16 bit result through out. */
static void mixer_chunk(MT2D_Mixer *m, short *out, int n)
{
	long acc[MIXER_CHUNK];
	int i, v;
	for (i = 0; i < n; i++) {
		acc[i] = 0;
	}
	for (v = 0; v < MT2D_MIXER_VOICES; v++) {
		mixer_voice(&m->voice[v], acc, n);
	}
	for (i = 0; i < n; i++) {
		long s = ((acc[i] >> 8) * (long)m->master) >> 8;
		if (s > 32767L) {
			s = 32767L;
		} else if (s < -32768L) {
			s = -32768L;
		}
		out[i] = (short)s;
	}
}

void MT2D_Mixer_Render16(MT2D_Mixer *m, short *out, int n)
{
	if (m == NULL || out == NULL) {
		return;
	}
	while (n > 0) {
		int c = n > MIXER_CHUNK ? MIXER_CHUNK : n;
		mixer_chunk(m, out, c);
		out += c;
		n -= c;
	}
}

void MT2D_Mixer_Render8(MT2D_Mixer *m, unsigned char *out, int n)
{
	short tmp[MIXER_CHUNK];
	int i;
	if (m == NULL || out == NULL) {
		return;
	}
	while (n > 0) {
		int c = n > MIXER_CHUNK ? MIXER_CHUNK : n;
		mixer_chunk(m, tmp, c);
		for (i = 0; i < c; i++) {
			out[i] = (unsigned char)((tmp[i] >> 8) + 128);
		}
		out += c;
		n -= c;
	}
}

void MT2D_Mixer_Code_End(void) {}

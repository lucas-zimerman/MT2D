#include <stddef.h>
#include <MT2D/Audio/MT2D_Music.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/File/MT2D_File.h>

static void music_fill(MT2D_Music *m) {
	unsigned long free_bytes = m->buf_size - (m->produced - m->consumed);
	while (m->produced < m->data_len && free_bytes > 0) {
		unsigned long idx = m->produced % m->buf_size;
		unsigned long n = free_bytes;
		int got;
		if (n > m->buf_size - idx) n = m->buf_size - idx;
		if (n > m->data_len - m->produced) n = m->data_len - m->produced;
		got = m->read(m->read_ctx, m->data_start + (long)m->produced, m->buf + idx, (int)n);
		if (got <= 0) {
			m->data_len = m->produced; /* the file ended early, play what we have */
			break;
		}
		m->produced += (unsigned long)got;
		free_bytes -= (unsigned long)got;
	}
}

/* Reads the optional length header, sets data_start/data_len. */
static int music_header(MT2D_Music *m, long size) {
	unsigned char h[2];
	unsigned long len;
	if (size < 4) {
		return MT2D_MUSIC_ERR_EMPTY;
	}
	if (m->read(m->read_ctx, 0, h, 2) != 2) {
		return MT2D_MUSIC_ERR_READ;
	}
	len = (unsigned long)h[0] | ((unsigned long)h[1] << 8);
	if (len == 0) { /* type 0: only events */
		m->data_start = 0;
		m->data_len = (unsigned long)size;
	} else { /* type 1: the first word is the length */
		m->data_start = 2;
		m->data_len = (unsigned long)(size - 2);
		if (len < m->data_len) m->data_len = len;
	}
	return MT2D_MUSIC_OK;
}

static void music_init(MT2D_Music *m, MT2D_MusicWriteFn write, void *write_ctx, int flags) {
	m->write = write;
	m->write_ctx = write_ctx;
	m->produced = 0;
	m->consumed = 0;
	m->wait = 0;
	m->playing = 0;
	m->stalled = 0;
	m->loop = (flags & MT2D_MUSIC_LOOP) ? 1 : 0;
	m->streaming = 0;
	m->rate = MT2D_MUSIC_DEFAULT_RATE;
}

int MT2D_Music_Open(MT2D_Music *m, MT2D_MusicReadFn read, void *read_ctx, long size,
					unsigned char *buf, unsigned long buf_size,
					MT2D_MusicWriteFn write, void *write_ctx, int flags) {
	int err;
	if (!m || !read || !buf || !write || size < 0) {
		return MT2D_MUSIC_ERR_ARGS;
	}
	music_init(m, write, write_ctx, flags);
	m->read = read;
	m->read_ctx = read_ctx;
	m->buf = buf;
	m->buf_size = buf_size;
	err = music_header(m, size);
	if (err != MT2D_MUSIC_OK) {
		return err;
	}
	if ((flags & MT2D_MUSIC_STREAM) || m->data_len > buf_size) {
		if (flags & MT2D_MUSIC_PRELOAD) {
			return MT2D_MUSIC_ERR_FIT;
		}
		if (buf_size < 8) {
			return MT2D_MUSIC_ERR_ARGS;
		}
		m->streaming = 1;
	}
	if (!m->streaming && m->buf_size > m->data_len) {
		m->buf_size = m->data_len; /* so the index never wraps before the end */
	}
	music_fill(m);
	if (m->data_len < 4) {
		return MT2D_MUSIC_ERR_EMPTY;
	}
	return MT2D_MUSIC_OK;
}

int MT2D_Music_OpenMemory(MT2D_Music *m, const unsigned char *data, long size,
						  MT2D_MusicWriteFn write, void *write_ctx, int flags) {
	MT2D_MusicMem mem;
	int err;
	if (!m || !data || !write || size < 0) {
		return MT2D_MUSIC_ERR_ARGS;
	}
	mem.data = data;
	mem.size = size;
	music_init(m, write, write_ctx, flags);
	m->read = MT2D_Music_ReadMemory;
	m->read_ctx = &mem; /* only used by the header read below */
	err = music_header(m, size);
	m->read = NULL;
	m->read_ctx = NULL;
	if (err != MT2D_MUSIC_OK) {
		return err;
	}
	m->buf = (unsigned char *)data + m->data_start;
	m->buf_size = m->data_len;
	m->produced = m->data_len;
	if (m->data_len < 4) {
		return MT2D_MUSIC_ERR_EMPTY;
	}
	return MT2D_MUSIC_OK;
}

void MT2D_Music_Play(MT2D_Music *m) {
	if (!m || m->data_len < 4) return;
	m->playing = 0;
	m->wait = 0;
	m->stalled = 0;
	if (m->streaming) {
		m->consumed = 0;
		m->produced = 0;
		music_fill(m);
	} else {
		m->consumed = 0;
	}
	m->playing = 1;
}

void MT2D_Music_Stop(MT2D_Music *m) {
	if (m) m->playing = 0;
}

int MT2D_Music_Tick(MT2D_Music *m) {
	unsigned events = 0;
	if (!m->playing) return 0;
	if (m->stalled) return 1;
	while (m->wait == 0 && events < MT2D_MUSIC_MAX_EVENTS) {
		unsigned long c = m->consumed;
		unsigned char e[4];
		unsigned long i;
		if (c + 4 > m->data_len) { /* end of the song */
			if (!m->loop || m->data_len < 4) {
				m->playing = 0;
				return 0;
			}
			if (m->streaming) {
				m->stalled = 1; /* Update rewinds, it's the only one allowed to do I/O */
				return 1;
			}
			m->consumed = 0;
			events++;
			continue;
		}
		if (m->produced - c < 4) { /* Update is late, wait for it */
			return 1;
		}
		for (i = 0; i < 4; i++) {
			e[i] = m->buf[(c + i) % m->buf_size];
		}
		m->consumed = c + 4;
		m->write(e[0], e[1], m->write_ctx);
		m->wait = (unsigned long)e[2] | ((unsigned long)e[3] << 8);
		events++;
	}
	if (m->wait) m->wait--;
	return 1;
}

void MT2D_Music_Tick_End(void) {}

void MT2D_Music_Update(MT2D_Music *m) {
	if (!m->streaming || !m->playing) return;
	if (m->stalled) {
		m->consumed = 0;
		m->produced = 0;
		m->wait = 0;
		music_fill(m);
		m->stalled = 0;
		return;
	}
	if (m->buf_size - (m->produced - m->consumed) >= m->buf_size / 2) {
		music_fill(m);
	}
}

int MT2D_Music_ReadMemory(void *ctx, long pos, unsigned char *dst, int n) {
	const MT2D_MusicMem *mem = (const MT2D_MusicMem *)ctx;
	long i;
	if (!mem || pos < 0 || n < 0 || pos >= mem->size) return 0;
	if (n > mem->size - pos) n = (int)(mem->size - pos);
	for (i = 0; i < n; i++) dst[i] = mem->data[pos + i];
	return n;
}

int MT2D_Music_ReadContainer(void *ctx, long pos, unsigned char *dst, int n) {
	return MT2D_Container_Read(*(int *)ctx, pos, dst, n);
}

int MT2D_Music_ReadFile(void *ctx, long pos, unsigned char *dst, int n) {
	MT2D_FILE *f = (MT2D_FILE *)ctx;
	if (!f || n < 0) return 0;
	if (MT2D_FILE_SEEK(f, pos, SEEK_SET) != 0) return 0;
	return (int)MT2D_FILE_READ(f, dst, 1, n);
}

#if defined(__DJGPP__)
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <MT2D/_MSDOS/M2Sound.h>
#include <MT2D/Audio/MT2D_Mixer.h>
#include <MT2D/Audio/MT2D_Wav.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/File/MT2D_File.h>

#define DMA_TOTAL (MT2D_DOS_SOUND_HALF * 2)

/* Everything the interrupt touches is in this struct, static and locked in memory. */
typedef struct SoundState {
	MT2D_Mixer mixer;
	unsigned char mix[MT2D_DOS_SOUND_HALF];
	unsigned long dma_addr;     /* linear address of the DMA buffer (DOS memory, below 1 MB) */
	unsigned short base;        /* Sound Blaster port */
	volatile unsigned char next; /* half the next interrupt refills */
	unsigned char irq;
} SoundState;

static SoundState g_snd;
static int g_dma = 0;
static int g_dos_sel = 0;
static int g_ready = 0;
static int g_atexit = 0;
static unsigned char g_old_mask_m = 0;
static unsigned char g_old_mask_s = 0;
static _go32_dpmi_seginfo g_old_isr;
static _go32_dpmi_seginfo g_new_isr;

/* ---- sound cache ---- */
typedef struct SoundEntry {
	char name[16];           /* "d/coin.wav", empty for a sound given from memory */
	void *block;             /* malloc block to free, or NULL */
	const void *samples;     /* NULL: the slot is free */
	unsigned long count;
	int format;
	int anon;                /* from memory: freed as soon as it stops playing */
} SoundEntry;
static SoundEntry g_cache[MT2D_DOS_SOUND_CACHE];

/* ---- interrupt ---- */

/*
 * Sound Blaster interrupt: a half of the DMA buffer finished playing, refill it.
 * Keep it, and the functions it calls, inside the locked ranges (see lock_memory).
 */
static void sound_isr(void) {
	unsigned long a;
	int i;
	inportb(g_snd.base + 0x0E); /* acknowledge the 8 bit DMA interrupt */
	if (g_snd.irq >= 8) {
		outportb(0xA0, 0x20);
	}
	outportb(0x20, 0x20);
	MT2D_Mixer_Render8(&g_snd.mixer, g_snd.mix, MT2D_DOS_SOUND_HALF);
	a = g_snd.dma_addr + (unsigned long)g_snd.next * MT2D_DOS_SOUND_HALF;
	_farsetsel(_dos_ds);
	for (i = 0; i < MT2D_DOS_SOUND_HALF; i++) {
		_farnspokeb(a + (unsigned long)i, g_snd.mix[i]);
	}
	g_snd.next ^= 1;
}
static void sound_isr_end(void) {}

static void lock_memory(void) {
	_go32_dpmi_lock_code((void *)sound_isr, (unsigned long)sound_isr_end - (unsigned long)sound_isr);
	_go32_dpmi_lock_code((void *)MT2D_Mixer_Init, (unsigned long)MT2D_Mixer_Code_End - (unsigned long)MT2D_Mixer_Init);
	_go32_dpmi_lock_data(&g_snd, sizeof(g_snd));
}

/* ---- the card ---- */

static int dsp_write(unsigned char v) {
	long t = 65535L;
	while (t-- > 0) {
		if (!(inportb(g_snd.base + 0x0C) & 0x80)) {
			outportb(g_snd.base + 0x0C, v);
			return 1;
		}
	}
	return 0;
}

static int dsp_read(void) {
	long t = 65535L;
	while (t-- > 0) {
		if (inportb(g_snd.base + 0x0E) & 0x80) {
			return inportb(g_snd.base + 0x0A);
		}
	}
	return -1;
}

static int dsp_reset(void) {
	int tries;
	outportb(g_snd.base + 0x06, 1);
	delay(3);
	outportb(g_snd.base + 0x06, 0);
	for (tries = 0; tries < 20; tries++) {
		if (dsp_read() == 0xAA) {
			return 1;
		}
		delay(1);
	}
	return 0;
}

static int digit(char c, int hex) {
	if (c >= '0' && c <= '9') return c - '0';
	if (hex && c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (hex && c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

/* BLASTER=A220 I5 D1 T3 (H5 is the 16 bit DMA, not used here). Returns 1 if base, irq and an 8 bit DMA were found. */
static int blaster_parse(void) {
	const char *e = getenv("BLASTER");
	unsigned long base = 0;
	int irq = -1, dma = -1;
	if (!e) return 0;
	while (*e) {
		char t;
		long v = 0;
		int hex, d;
		while (*e == ' ' || *e == '\t') e++;
		if (!*e) break;
		t = *e++;
		if (t >= 'a' && t <= 'z') t = (char)(t - 32);
		hex = (t == 'A');
		while (*e && *e != ' ' && *e != '\t') {
			d = digit(*e++, hex);
			if (d >= 0) v = v * (hex ? 16 : 10) + d;
		}
		if (t == 'A') base = (unsigned long)v;
		else if (t == 'I') irq = (int)v;
		else if (t == 'D') dma = (int)v;
	}
	if (irq == 2) irq = 9; /* IRQ 2 is the cascade, the card really uses 9 */
	if (base < 0x210 || base > 0x280 || irq < 3 || irq > 15 || (dma != 0 && dma != 1 && dma != 3)) {
		return 0;
	}
	g_snd.base = (unsigned short)base;
	g_snd.irq = (unsigned char)irq;
	g_dma = dma;
	return 1;
}

/* DMA buffer in DOS memory that does not cross a 64 KB page. */
static int dma_alloc(void) {
	int seg, sel;
	unsigned long addr, i;
	seg = __dpmi_allocate_dos_memory((DMA_TOTAL * 2 + 15) / 16, &sel);
	if (seg == -1) return 0;
	addr = (unsigned long)seg * 16UL;
	if ((addr & 0xFFFFUL) + DMA_TOTAL > 0x10000UL) {
		addr = (addr + 0xFFFFUL) & ~0xFFFFUL; /* the spare half of the allocation covers this */
	}
	g_dos_sel = sel;
	g_snd.dma_addr = addr;
	_farsetsel(_dos_ds);
	for (i = 0; i < DMA_TOTAL; i++) {
		_farnspokeb(addr + i, 128); /* silence */
	}
	return 1;
}

static void dma_program(void) {
	static const unsigned char page_port[4] = {0x87, 0x83, 0x81, 0x82};
	unsigned long a = g_snd.dma_addr;
	outportb(0x0A, (unsigned char)(4 | g_dma));            /* mask the channel */
	outportb(0x0C, 0);                                     /* reset the flip flop */
	outportb(0x0B, (unsigned char)(0x58 | g_dma));         /* single, auto-init, memory -> card */
	outportb((unsigned short)(g_dma * 2), (unsigned char)(a & 0xFF));
	outportb((unsigned short)(g_dma * 2), (unsigned char)((a >> 8) & 0xFF));
	outportb(page_port[g_dma], (unsigned char)((a >> 16) & 0xFF));
	outportb((unsigned short)(g_dma * 2 + 1), (unsigned char)((DMA_TOTAL - 1) & 0xFF));
	outportb((unsigned short)(g_dma * 2 + 1), (unsigned char)(((DMA_TOTAL - 1) >> 8) & 0xFF));
	outportb(0x0A, (unsigned char)g_dma);                  /* unmask */
}

static void pic_irq(int irq, int on, unsigned char *old_m, unsigned char *old_s) {
	unsigned char m = inportb(0x21);
	unsigned char s = inportb(0xA1);
	if (on) {
		*old_m = m;
		*old_s = s;
		if (irq >= 8) {
			s = (unsigned char)(s & ~(1 << (irq - 8)));
			m = (unsigned char)(m & ~4); /* the cascade */
		} else {
			m = (unsigned char)(m & ~(1 << irq));
		}
	} else {
		if (irq >= 8) {
			s = (unsigned char)((s & ~(1 << (irq - 8))) | (*old_s & (1 << (irq - 8))));
			m = (unsigned char)((m & ~4) | (*old_m & 4));
		} else {
			m = (unsigned char)((m & ~(1 << irq)) | (*old_m & (1 << irq)));
		}
	}
	outportb(0x21, m);
	outportb(0xA1, s);
}

static int irq_vector(int irq) {
	return irq < 8 ? 8 + irq : 0x70 + irq - 8;
}

int MT2D_Dos_Sound_Init(void) {
	unsigned char hi, lo;
	int vi;
	if (g_ready) return 1;
	if (!g_atexit) {
		atexit(MT2D_Dos_Sound_Close);
		g_atexit = 1;
	}
	if (!blaster_parse() || !dsp_reset()) return 0;
	dsp_write(0xE1); /* DSP version */
	vi = dsp_read();
	{
		int minor = dsp_read();
		if (vi < 2 || minor < 0) return 0; /* the auto-init DMA mode came with the DSP 2.00 */
	}
	if (!dma_alloc()) return 0;
	MT2D_Mixer_Init(&g_snd.mixer);
	memset(g_snd.mix, 128, sizeof(g_snd.mix));
	g_snd.next = 0;
	lock_memory();

	_go32_dpmi_get_protected_mode_interrupt_vector(irq_vector(g_snd.irq), &g_old_isr);
	g_new_isr.pm_selector = _go32_my_cs();
	g_new_isr.pm_offset = (unsigned long)sound_isr;
	_go32_dpmi_allocate_iret_wrapper(&g_new_isr);
	disable();
	_go32_dpmi_set_protected_mode_interrupt_vector(irq_vector(g_snd.irq), &g_new_isr);
	pic_irq(g_snd.irq, 1, &g_old_mask_m, &g_old_mask_s);
	enable();

	dma_program();
	lo = (unsigned char)(256 - 1000000L / MT2D_DOS_SOUND_RATE);
	hi = (unsigned char)((MT2D_DOS_SOUND_HALF - 1) >> 8);
	dsp_write(0xD1); /* speaker on */
	dsp_write(0x40); /* time constant */
	dsp_write(lo);
	dsp_write(0x48); /* block size: one interrupt per half */
	dsp_write((unsigned char)((MT2D_DOS_SOUND_HALF - 1) & 0xFF));
	dsp_write(hi);
	dsp_write(0x1C); /* start the 8 bit auto-init DMA playback */
	g_ready = 1;
	return 1;
}

static void cache_free(SoundEntry *e) {
	if (e->block) free(e->block);
	memset(e, 0, sizeof(*e));
}

void MT2D_Dos_Sound_Close(void) {
	int i;
	if (!g_ready) return;
	g_ready = 0;
	dsp_write(0xD0); /* halt */
	dsp_write(0xDA); /* leave auto-init */
	dsp_write(0xD3); /* speaker off */
	outportb(0x0A, (unsigned char)(4 | g_dma));
	disable();
	pic_irq(g_snd.irq, 0, &g_old_mask_m, &g_old_mask_s);
	_go32_dpmi_set_protected_mode_interrupt_vector(irq_vector(g_snd.irq), &g_old_isr);
	enable();
	_go32_dpmi_free_iret_wrapper(&g_new_isr);
	dsp_reset();
	__dpmi_free_dos_memory(g_dos_sel);
	MT2D_Mixer_StopAll(&g_snd.mixer);
	for (i = 0; i < MT2D_DOS_SOUND_CACHE; i++) {
		cache_free(&g_cache[i]);
	}
}

/* ---- sounds ---- */

static int in_use(const SoundEntry *e) {
	int i;
	for (i = 0; i < MT2D_MIXER_VOICES; i++) {
		if (g_snd.mixer.voice[i].active && g_snd.mixer.voice[i].data == e->samples) return 1;
	}
	return 0;
}

/* A free slot: first the anonymous sounds that stopped, then any cached sound nobody plays. */
static SoundEntry *slot_get(void) {
	int i;
	for (i = 0; i < MT2D_DOS_SOUND_CACHE; i++) {
		if (g_cache[i].samples && g_cache[i].anon && !in_use(&g_cache[i])) cache_free(&g_cache[i]);
	}
	for (i = 0; i < MT2D_DOS_SOUND_CACHE; i++) {
		if (!g_cache[i].samples) return &g_cache[i];
	}
	for (i = 0; i < MT2D_DOS_SOUND_CACHE; i++) {
		if (!in_use(&g_cache[i])) {
			cache_free(&g_cache[i]);
			return &g_cache[i];
		}
	}
	return NULL;
}

/*
 * Fills e from a WAV image. owner (may be NULL) is the malloc block holding the image: kept when the
 * samples are played straight from it, freed when they had to be converted. Returns 1 on success.
 */
static int entry_load(SoundEntry *e, unsigned char *file, unsigned long size, void *owner) {
	MT2D_Wav w;
	unsigned long bytes;
	if (MT2D_Wav_Parse(file, size, &w) != MT2D_WAV_OK) return 0;
	if (w.rate == MT2D_DOS_SOUND_RATE && (w.format == MT2D_MIXER_U8 || (((unsigned long)w.data & 1) == 0))) {
		e->block = owner;
		e->samples = w.data;
		e->count = w.samples;
	} else {
		unsigned long n = MT2D_Wav_ResampledLength(w.samples, w.rate, MT2D_DOS_SOUND_RATE);
		void *buf = malloc(n * (w.format == MT2D_MIXER_S16 ? 2 : 1));
		if (!buf) return 0;
		MT2D_Wav_Resample(w.data, w.samples, w.format, w.rate, MT2D_DOS_SOUND_RATE, buf);
		if (owner) free(owner);
		e->block = buf;
		e->samples = buf;
		e->count = n;
	}
	e->format = w.format;
	bytes = e->count * (e->format == MT2D_MIXER_S16 ? 2 : 1);
	_go32_dpmi_lock_data((void *)e->samples, bytes); /* the interrupt reads it */
	return 1;
}

static int voice_start(const SoundEntry *e) {
	int h;
	disable();
	h = MT2D_Mixer_Play(&g_snd.mixer, e->samples, e->count, e->format, 0, MT2D_MIXER_VOL_MAX);
	enable();
	return h;
}

/* Reads a whole container file or loose file into a new malloc block. */
static unsigned char *read_all(const char *path, unsigned long *size) {
	unsigned char *buf = NULL;
	int id = MT2D_Container_Get_FileId((char *)path);
	if (id >= 0) {
		long len = MT2D_Container_Get_FileLength(id);
		if (len <= 0 || (unsigned long)len > MT2D_DOS_SOUND_MAX) return NULL;
		buf = (unsigned char *)malloc((size_t)len);
		if (!buf) return NULL;
		if (MT2D_Container_Read(id, 0L, buf, (int)len) != (int)len) {
			free(buf);
			return NULL;
		}
		*size = (unsigned long)len;
	} else {
		long len;
		MT2D_FILE *f = MT2D_FILE_OPEN(path, "rb");
		if (!f) return NULL;
		MT2D_FILE_SEEK(f, 0L, SEEK_END);
		len = MT2D_FILE_TELL(f);
		MT2D_FILE_SEEK(f, 0L, SEEK_SET);
		if (len <= 0 || (unsigned long)len > MT2D_DOS_SOUND_MAX) {
			MT2D_FILE_CLOSE(f);
			return NULL;
		}
		buf = (unsigned char *)malloc((size_t)len);
		if (buf && (long)MT2D_FILE_READ(f, buf, 1, (int)len) != len) {
			free(buf);
			buf = NULL;
		}
		MT2D_FILE_CLOSE(f);
		if (!buf) return NULL;
		*size = (unsigned long)len;
	}
	return buf;
}

/* "dir/coin.ogg" -> "d/coin.wav" (stem cut to 8 characters), like the images and the songs. */
static void sound_path(const char *name, char *path) {
	const char *base = name;
	const char *p;
	int n = 0, stem = 0;
	for (p = name; *p; p++) {
		if (*p == '/' || *p == '\\') base = p + 1;
	}
	path[n++] = 'd';
	path[n++] = '/';
	while (base[stem] && base[stem] != '.' && stem < 8) {
		path[n++] = base[stem++];
	}
	strcpy(path + n, ".wav");
}

int MT2D_Dos_Sound_Play(const char *name) {
	char path[16];
	SoundEntry *e = NULL;
	int i;
	if (!g_ready || !name) return -1;
	sound_path(name, path);
	for (i = 0; i < MT2D_DOS_SOUND_CACHE; i++) {
		if (g_cache[i].samples && strcmp(g_cache[i].name, path) == 0) {
			e = &g_cache[i];
			break;
		}
	}
	if (!e) {
		unsigned long size = 0;
		unsigned char *file;
		e = slot_get();
		if (!e) return -1;
		file = read_all(path, &size);
		if (!file) return -1;
		if (!entry_load(e, file, size, file)) {
			/* entry_load frees the file only after a successful conversion */
			free(file);
			memset(e, 0, sizeof(*e));
			return -1;
		}
		strcpy(e->name, path);
	}
	return voice_start(e);
}

int MT2D_Dos_Sound_PlayMemory(void *wav, unsigned size, int free_after) {
	SoundEntry *e;
	int h;
	if (!g_ready || !wav) return -1;
	e = slot_get();
	if (!e) return -1;
	if (!entry_load(e, (unsigned char *)wav, size, free_after ? wav : NULL)) {
		memset(e, 0, sizeof(*e));
		return -1; /* the block stays the caller's */
	}
	e->anon = 1;
	h = voice_start(e);
	if (h < 0) cache_free(e);
	return h;
}

void MT2D_Dos_Sound_Stop(int handle) {
	if (!g_ready) return;
	disable();
	MT2D_Mixer_Stop(&g_snd.mixer, handle);
	enable();
}

void MT2D_Dos_Sound_StopAll(void) {
	if (!g_ready) return;
	disable();
	MT2D_Mixer_StopAll(&g_snd.mixer);
	enable();
}

int MT2D_Dos_Sound_Active(void) {
	return g_ready ? MT2D_Mixer_Active(&g_snd.mixer) : 0;
}
#endif

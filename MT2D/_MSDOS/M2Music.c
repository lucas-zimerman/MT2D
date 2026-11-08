#if defined(__DJGPP__)
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/exceptn.h>
#include <MT2D/_MSDOS/M2Music.h>
#include <MT2D/_MSDOS/M2Opl.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/File/MT2D_File.h>

#define PIT_CLOCK 1193182UL
#define PIT_FULL  65536UL /* the divisor DOS uses: 18.2 ticks per second */

/* Everything the interrupt touches is static and locked in memory. */
static MT2D_Music g_music;
static unsigned char g_buf[MT2D_DOS_MUSIC_BUF];
static volatile unsigned long g_div = PIT_FULL;
static volatile unsigned long g_acc = 0;
static struct { unsigned long offset; unsigned short selector; } g_old_far;

static _go32_dpmi_seginfo g_old_isr;
static _go32_dpmi_seginfo g_new_isr;
static int g_opl = 0;
static int g_isr_on = 0;
static int g_atexit = 0;
static int g_rate = MT2D_MUSIC_DEFAULT_RATE;
static int g_container_id = -1;
static MT2D_FILE *g_file = NULL;
static void *g_free_mem = NULL;

/*
 * Timer interrupt: plays the song at the song rate. DOS and the BIOS expect 18.2 ticks per
 * second, so the old handler is only called when that much time went by (g_acc counts PIT
 * ticks modulo 65536). Otherwise we acknowledge the interrupt ourselves.
 * Keep it, and the functions it calls, inside the locked ranges (see lock_memory).
 */
static void music_isr(void) {
	MT2D_Music_Tick(&g_music);
	g_acc += g_div;
	if (g_acc >= PIT_FULL) {
		g_acc -= PIT_FULL;
		__asm__ __volatile__("pushfl\n\tlcall *%0" : : "m"(g_old_far) : "memory", "cc");
	} else {
		outportb(0x20, 0x20);
	}
}
static void music_isr_end(void) {}

static void lock_memory(void) {
	_go32_dpmi_lock_code((void *)music_isr, (unsigned long)music_isr_end - (unsigned long)music_isr);
	_go32_dpmi_lock_code((void *)MT2D_Music_Tick, (unsigned long)MT2D_Music_Tick_End - (unsigned long)MT2D_Music_Tick);
	_go32_dpmi_lock_code((void *)MT2D_Dos_Opl_Write, (unsigned long)MT2D_Dos_Opl_End - (unsigned long)MT2D_Dos_Opl_Write);
	_go32_dpmi_lock_data(&g_music, sizeof(g_music));
	_go32_dpmi_lock_data(g_buf, sizeof(g_buf));
	_go32_dpmi_lock_data((void *)&g_div, sizeof(g_div));
	_go32_dpmi_lock_data((void *)&g_acc, sizeof(g_acc));
	_go32_dpmi_lock_data(&g_old_far, sizeof(g_old_far));
}

static void pit_set(unsigned long divisor) {
	outportb(0x43, 0x36); /* channel 0, lobyte/hibyte, rate generator */
	outportb(0x40, (unsigned char)(divisor & 0xFF));
	outportb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

static void isr_install(void) {
	unsigned long div = PIT_CLOCK / (unsigned long)g_rate;
	if (div < 256) div = 256;       /* no more than ~4.6 kHz */
	if (div > 0xFFFF) div = 0xFFFF;
	if (g_isr_on) {
		disable();
		g_div = div;
		pit_set(div);
		enable();
		return;
	}
	lock_memory();
	_go32_dpmi_get_protected_mode_interrupt_vector(8, &g_old_isr);
	g_old_far.offset = g_old_isr.pm_offset;
	g_old_far.selector = g_old_isr.pm_selector;
	g_new_isr.pm_selector = _go32_my_cs();
	g_new_isr.pm_offset = (unsigned long)music_isr;
	_go32_dpmi_allocate_iret_wrapper(&g_new_isr);
	disable();
	g_div = div;
	g_acc = 0;
	_go32_dpmi_set_protected_mode_interrupt_vector(8, &g_new_isr);
	pit_set(div);
	enable();
	g_isr_on = 1;
}

static void isr_remove(void) {
	if (!g_isr_on) return;
	disable();
	pit_set(PIT_FULL & 0xFFFF); /* 0 means 65536 */
	_go32_dpmi_set_protected_mode_interrupt_vector(8, &g_old_isr);
	enable();
	_go32_dpmi_free_iret_wrapper(&g_new_isr);
	g_isr_on = 0;
}

static void release_sources(void) {
	if (g_file) {
		MT2D_FILE_CLOSE(g_file);
		g_file = NULL;
	}
	if (g_free_mem) {
		free(g_free_mem);
		g_free_mem = NULL;
	}
	g_container_id = -1;
}

int MT2D_Dos_Music_Init(void) {
	if (!g_atexit) {
		atexit(MT2D_Dos_Music_Close);
		g_atexit = 1;
	}
	if (!g_opl) {
		g_opl = MT2D_Dos_Opl_Detect();
		if (g_opl) {
			MT2D_Dos_Opl_Reset();
		}
	}
	return g_opl;
}

void MT2D_Dos_Music_Stop(void) {
	if (!g_opl) return;
	MT2D_Music_Stop(&g_music);
	isr_remove();
	MT2D_Dos_Opl_Silence();
	release_sources();
}

void MT2D_Dos_Music_Close(void) {
	MT2D_Dos_Music_Stop();
}

void MT2D_Dos_Music_SetRate(int hz) {
	if (hz < 50) hz = 50;
	if (hz > 4000) hz = 4000;
	g_rate = hz;
}

static int start(int err) {
	if (err != MT2D_MUSIC_OK) {
		release_sources();
		return err;
	}
	if (g_music.streaming == 0 && g_file) {
		MT2D_FILE_CLOSE(g_file); /* the whole song is in memory already */
		g_file = NULL;
	}
	g_music.rate = (unsigned short)g_rate;
	MT2D_Music_Play(&g_music);
	isr_install();
	return MT2D_MUSIC_OK;
}

int MT2D_Dos_Music_Play(const char *name, int flags) {
	int id;
	if (!g_opl || !name) return MT2D_MUSIC_ERR_ARGS;
	MT2D_Dos_Music_Stop();
	id = MT2D_Container_Get_FileId((char *)name);
	if (id >= 0) {
		g_container_id = id;
		return start(MT2D_Music_Open(&g_music, MT2D_Music_ReadContainer, &g_container_id,
									 MT2D_Container_Get_FileLength(id), g_buf, sizeof(g_buf),
									 MT2D_Dos_Opl_Write, NULL, flags));
	} else {
		long size;
		g_file = MT2D_FILE_OPEN(name, "rb");
		if (!g_file) return MT2D_MUSIC_ERR_READ;
		MT2D_FILE_SEEK(g_file, 0L, SEEK_END);
		size = MT2D_FILE_TELL(g_file);
		return start(MT2D_Music_Open(&g_music, MT2D_Music_ReadFile, g_file, size, g_buf, sizeof(g_buf),
									 MT2D_Dos_Opl_Write, NULL, flags));
	}
}

int MT2D_Dos_Music_PlayFile(const char *name) {
	char path[32];
	const char *base = name;
	const char *p;
	int stem = 0, n = 0, len;
	if (!name) return MT2D_MUSIC_ERR_ARGS;
	for (p = name; *p; p++) {
		if (*p == '/' || *p == '\\') base = p + 1;
	}
	len = (int)strlen(base);
	if (len > 4 && (base[len - 4] == '.') && (base[len - 3] | 32) == 'i' && (base[len - 2] | 32) == 'm' && (base[len - 1] | 32) == 'f') {
		return MT2D_Dos_Music_Play(name, MT2D_MUSIC_LOOP); /* already an imf: use it as it is */
	}
	path[n++] = 'd';
	path[n++] = '/';
	while (base[stem] && base[stem] != '.' && stem < 8) {
		path[n++] = base[stem++];
	}
	strcpy(path + n, ".imf");
	return MT2D_Dos_Music_Play(path, MT2D_MUSIC_LOOP);
}

int MT2D_Dos_Music_PlayMemory(void *data, unsigned size, int free_after, int flags) {
	int err;
	if (!g_opl || !data) return MT2D_MUSIC_ERR_ARGS;
	MT2D_Dos_Music_Stop();
	if (free_after) g_free_mem = data;
	err = MT2D_Music_OpenMemory(&g_music, (const unsigned char *)data, (long)size, MT2D_Dos_Opl_Write, NULL, flags);
	if (err == MT2D_MUSIC_OK) {
		lock_memory();
		_go32_dpmi_lock_data(data, size); /* the interrupt reads it */
	}
	return start(err);
}

int MT2D_Dos_Music_IsPlaying(void) {
	return g_opl && g_music.playing;
}

int MT2D_Dos_Music_IsStreaming(void) {
	return g_opl && g_music.playing && g_music.streaming;
}

void MT2D_Dos_Music_Update(void) {
	if (g_opl && g_isr_on) {
		MT2D_Music_Update(&g_music);
	}
}
#endif

/* Benchmark of the MS-DOS VGA renderer (mode 13h, MSDOS_USE_GRAPHICS), see bench/README.md.
 *
 * It draws scripted scenes through the public renderer calls (Dos_Clear_ImgBuffer, Dos_Add_ImagetoBuffer,
 * MT2D_Dos_Render_VGA_ASCII) and prints:
 *   - the time of one frame of each scene (uclock, best of 3 runs of the scene),
 *   - a checksum of the 320x200 screen after the scene, so two builds can be checked to draw the same picture,
 *   - the heap usage (peak bytes alive and number of malloc/realloc/free calls) measured with the linker's --wrap.
 * Builds: bench/build_vga.sh. Written in C, only the renderer itself stays DJGPP style C++.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/nearptr.h>
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/_MSDOS/Render/DosImg.h>
#include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>
#include <MT2D/_MSDOS/Render/dVgaRen.h>
#include <MT2D/_MSDOS/Render/font.h>

/* ---- heap accounting: malloc/realloc calls are counted with the linker's --wrap (free is not wrapped, libc frees blocks
   it allocated itself). The heap size is the sbrk high-water mark: DJGPP's malloc grows the heap with sbrk, so it also
   shows the fragmentation of the many small blocks. ---- */
static unsigned long g_malloc = 0, g_realloc = 0;
void *__real_malloc(size_t n);
void *__real_realloc(void *p, size_t n);
void *__wrap_malloc(size_t n) { g_malloc++; return __real_malloc(n); }
void *__wrap_realloc(void *p, size_t n) { g_realloc++; return __real_realloc(p, n); }

/* ---- scene helpers ---- */
extern unsigned char VGA_ASCII_COLORS[2];
typedef struct { const unsigned char *pix; int w, h, x, y; } LogSprite;
static LogSprite slog[64];
static int slog_n = 0;
static void ADD(Sprite *sp, int x, int y) {
	if (slog_n < 64) { slog[slog_n].pix = sp->Image->Pixels; slog[slog_n].w = sp->SizeX; slog[slog_n].h = sp->SizeY; slog[slog_n].x = x; slog[slog_n].y = y; slog_n++; }
	Dos_Add_ImagetoBuffer(sp, x, y);
}

#define FRAMES 120
#define REPS 3
static unsigned char win[25][80];
static unsigned char pixbuf[8][64 * 64];
static MT2D_Internal_Image imgs[8];
static Sprite spr[8];
static unsigned char *screen = 0;
static unsigned long rng = 12345;

static unsigned long rnd(void) { rng = rng * 1103515245UL + 12345UL; return (rng >> 16) & 0x7fff; }
static double now_ms(void) { return (double)uclock() * 1000.0 / UCLOCKS_PER_SEC; }

static unsigned long checksum(void) {
	unsigned long h = 2166136261UL;
	int i;
	for (i = 0; i < 64000; i++) { h ^= screen[i]; h *= 16777619UL; }
	return h;
}

static void make_sprites(void) {
	int s, x, y;
	for (s = 0; s < 8; s++) {
		for (y = 0; y < 64; y++) for (x = 0; x < 64; x++) {
			/* about a quarter of the pixels are transparent (index 0) */
			pixbuf[s][y * 64 + x] = ((x ^ y) & 3) == 0 ? 0 : (unsigned char)(32 + ((x + y + s * 9) & 63));
		}
		imgs[s].Pixels = pixbuf[s];
		spr[s].Image = &imgs[s]; spr[s].SizeX = 32; spr[s].SizeY = 32; spr[s].ScaleX = 1; spr[s].ScaleY = 1;
		spr[s].type = 1; spr[s].refCount = 1;
	}
}

static void fill_text(int seed) {
	int x, y;
	for (y = 0; y < 25; y++) for (x = 0; x < 80; x++) win[y][x] = (unsigned char)(33 + ((x * 7 + y * 13 + seed) % 90));
}

/* one frame as the engine does it: clear the window and the sprites, add the sprites, render */
static void frame_begin(void) {
	memset(win, ' ', sizeof(win));
	slog_n = 0;
	Dos_Clear_ImgBuffer();
}

/* Independent reference: paints the text with the font, then the sprites in the order they were added, pixel by pixel.
   Returns the number of screen pixels that differ from what the renderer drew. */
static unsigned char refbuf[64000];
static long verify(void) {
	int tx, ty, i, r, c, k, x, y;
	long bad = 0;
	for (ty = 0; ty < 25; ty++) for (tx = 0; tx < 80; tx++) {
		unsigned char ch = win[ty][tx];
		unsigned off = (ch & 31) + ((ch >> 5) << 7);
		for (i = 0; i < 4; i++) {
			unsigned char b = VGAFONT[off + 32 * i];
			for (r = 0; r < 2; r++) for (c = 0; c < 4; c++) {
				int bit = r == 0 ? 7 - c : 3 - c;
				refbuf[(ty * 8 + i * 2 + r) * 320 + tx * 4 + c] = VGA_ASCII_COLORS[(b >> bit) & 1];
			}
		}
	}
	for (k = 0; k < slog_n; k++) {
		for (y = 0; y < slog[k].h; y++) for (x = 0; x < slog[k].w; x++) {
			int sx = slog[k].x + x, sy = slog[k].y + y;
			unsigned char v = slog[k].pix[y * slog[k].w + x];
			if (v && sx >= 0 && sx < 320 && sy >= 0 && sy < 200) refbuf[sy * 320 + sx] = v;
		}
	}
	for (i = 0; i < 64000; i++) if (refbuf[i] != screen[i]) bad++;
	return bad;
}

typedef void (*SceneFn)(int f);
static void sc_idle(int f)   { (void)f; frame_begin(); fill_text(0); }
static void sc_churn(int f)  { int i; frame_begin(); fill_text(0); for (i = 0; i < 500; i++) win[rnd() % 25][rnd() % 80] = (unsigned char)(33 + rnd() % 90); (void)f; }
/* sprites at positions that are multiples of the tile (4x8) and 32x32 big */
static void sc_aligned(int f) {
	int i;
	frame_begin(); fill_text(0);
	for (i = 0; i < 8; i++) ADD(&spr[i], ((i * 36 + f * 4) % 280) & ~3, ((i * 16 + f * 8) % 160) & ~7);
}
/* any pixel position, odd sizes */
static void sc_free(int f) {
	int i;
	frame_begin(); fill_text(0);
	for (i = 0; i < 8; i++) { spr[i].SizeX = 29; spr[i].SizeY = 21; ADD(&spr[i], (i * 37 + f * 3) % 280 + 1, (i * 17 + f) % 170 + 3); }
	for (i = 0; i < 8; i++) { spr[i].SizeX = 32; spr[i].SizeY = 32; }
}
/* many small sprites */
static void sc_many(int f) {
	int i;
	frame_begin(); fill_text(0);
	for (i = 0; i < 48; i++) {
		Sprite *s = &spr[i & 7];
		s->SizeX = 16; s->SizeY = 16;
		ADD(s, ((i * 23 + f * 2) % 300) & ~3, ((i * 11 + f) % 180) & ~7);
	}
	for (i = 0; i < 8; i++) { spr[i].SizeX = 32; spr[i].SizeY = 32; }
}

static void run_scene(const char *name, SceneFn fn, int warm) {
	double best = 1e18;
	unsigned long m0 = g_malloc + g_realloc, sum = 0;
	int r, f;
	for (r = 0; r < REPS; r++) {
		double t0, t1;
		rng = 12345;
		for (f = 0; f < warm; f++) { fn(f); MT2D_Dos_Render_VGA_ASCII(win); } /* warm up: the first frame always draws everything */
		t0 = now_ms();
		for (f = 0; f < FRAMES; f++) { fn(warm + f); MT2D_Dos_Render_VGA_ASCII(win); }
		t1 = now_ms();
		if (t1 - t0 < best) best = t1 - t0;
	}
	sum = checksum();
	printf("RESULT %-14s ms/frame=%8.3f  sum=%08lX  badpixels=%ld  alloc/frame=%.1f\n", name, best / FRAMES, sum, verify(),
	       (double)((g_malloc + g_realloc) - m0) / (double)(REPS * (FRAMES + warm)));
	fflush(stdout);
	frame_begin(); /* leave no sprites behind for the next scene */
	MT2D_Dos_Render_VGA_ASCII(win);
}

int main(void) {
	unsigned long heap0 = (unsigned long)sbrk(0); /* before MT2D_Dos_VGA_Init, so the double buffer is counted */
	printf("START\n"); fflush(stdout);
	if (!MT2D_Dos_VGA_Init()) { printf("VGA init failed\n"); return 1; }
	screen = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
	make_sprites();
	fill_text(0);
	printf("INIT OK\n"); fflush(stdout);
	MT2D_Dos_Render_VGA_ASCII(win);
	printf("FIRST FRAME OK\n"); fflush(stdout);
	{
		run_scene("idle", sc_idle, 2);
		run_scene("text_churn", sc_churn, 2);
		run_scene("sprites_al", sc_aligned, 2);
		run_scene("sprites_free", sc_free, 2);
		run_scene("sprites_x48", sc_many, 2);
		MT2D_Dos_VGA_Close();
		printf("RESULT %-14s heap_growth_bytes=%lu  malloc=%lu realloc=%lu\n", "HEAP", (unsigned long)sbrk(0) - heap0,
		       g_malloc, g_realloc);
	}
	printf("DONE\n");
	return 0;
}

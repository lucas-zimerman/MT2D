/* MT2D Sixel encoder, see MT2D_Linux_Sixel.h. Portable C, no terminal access. */
#include <stddef.h>
#include <string.h>
#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "MT2D_Linux_Sixel.h"
#include "MT2D_Linux_Font.h"

/* The global palette: all black until MT2D_Init sets the VGA default through the MT2D_SET_PALETTE setting. */
static unsigned char Global_Palette[MT2D_SIXEL_COLORS][3];

/* 6 bit VGA component to 8 bits */
#define VGA6(v) ((int)(((v) << 2) | ((v) >> 4)))

/* The VGA default palette is made of 16 EGA colors, 16 grays and 9 groups of 24 colors (a ramp around the hue circle,
   each group has a brightness and a minimum level), see the ramp tables below. */
void MT2D_Sixel_Default_Palette(int rgb[MT2D_SIXEL_COLORS]) {
	static const unsigned char ega[16][3] = {
		{0, 0, 0}, {0, 0, 42}, {0, 42, 0}, {0, 42, 42}, {42, 0, 0}, {42, 0, 42}, {42, 21, 0}, {42, 42, 42},
		{21, 21, 21}, {21, 21, 63}, {21, 63, 21}, {21, 63, 63}, {63, 21, 21}, {63, 21, 63}, {63, 63, 21}, {63, 63, 63}
	};
	static const unsigned char gray[16] = {0, 5, 8, 11, 14, 17, 20, 24, 28, 32, 36, 40, 45, 50, 56, 63};
	static const unsigned char group[9][2] = { /* { highest, lowest } level of each group */
		{63, 0}, {63, 31}, {63, 45}, {28, 0}, {28, 14}, {28, 20}, {16, 0}, {16, 8}, {16, 11}
	};
	/* one turn around the hue circle in 24 steps: the level (0..4 of the ramp) of red, green and blue */
	static const unsigned char hue[24][3] = {
		{0, 0, 4}, {1, 0, 4}, {2, 0, 4}, {3, 0, 4}, {4, 0, 4}, {4, 0, 3}, {4, 0, 2}, {4, 0, 1},
		{4, 0, 0}, {4, 1, 0}, {4, 2, 0}, {4, 3, 0}, {4, 4, 0}, {3, 4, 0}, {2, 4, 0}, {1, 4, 0},
		{0, 4, 0}, {0, 4, 1}, {0, 4, 2}, {0, 4, 3}, {0, 4, 4}, {0, 3, 4}, {0, 2, 4}, {0, 1, 4}
	};
	int i, g, h, k, ramp[5], d, q;
	for (i = 0; i < 16; i++) rgb[i] = (VGA6(ega[i][0]) << 16) | (VGA6(ega[i][1]) << 8) | VGA6(ega[i][2]);
	for (i = 0; i < 16; i++) rgb[16 + i] = (VGA6(gray[i]) << 16) | (VGA6(gray[i]) << 8) | VGA6(gray[i]);
	for (g = 0; g < 9; g++) {
		for (k = 0; k < 5; k++) { /* lowest + (highest - lowest) * k / 4, a half rounds down */
			d = (group[g][0] - group[g][1]) * k;
			q = d / 4;
			if (d % 4 > 2) q++;
			ramp[k] = group[g][1] + q;
		}
		for (h = 0; h < 24; h++) {
			rgb[32 + g * 24 + h] = (VGA6(ramp[hue[h][0]]) << 16) | (VGA6(ramp[hue[h][1]]) << 8) | VGA6(ramp[hue[h][2]]);
		}
	}
	for (i = 248; i < MT2D_SIXEL_COLORS; i++) rgb[i] = 0;
}

void MT2D_Sixel_Palette_Set_Rgb(int first, const int *rgb, int count) {
	int i;
	for (i = 0; i < count && first + i < MT2D_SIXEL_COLORS; i++) {
		if (first + i < 0) continue;
		Global_Palette[first + i][0] = (unsigned char)((rgb[i] >> 16) & 0xFF); /* only the low 24 bits are used */
		Global_Palette[first + i][1] = (unsigned char)((rgb[i] >> 8) & 0xFF);
		Global_Palette[first + i][2] = (unsigned char)(rgb[i] & 0xFF);
	}
}

void MT2D_Sixel_Palette_Set_Bgra(const unsigned char *bgra, int count) {
	int i;
	for (i = 0; i < count && i < MT2D_SIXEL_COLORS; i++) {
		Global_Palette[i][0] = bgra[i * 4 + 2];
		Global_Palette[i][1] = bgra[i * 4 + 1];
		Global_Palette[i][2] = bgra[i * 4];
	}
}

const unsigned char *MT2D_Sixel_Palette_Entry(int index) {
	if (index < 0 || index >= MT2D_SIXEL_COLORS) return NULL;
	return Global_Palette[index];
}

void MT2D_Sixel_Draw_Text(MT2D_Sixel_Canvas *canvas, unsigned char *pixels, const unsigned char *cells, int cols, int rows,
	int bg, int fg) {
	int cx, cy, row, bit;
	unsigned char *dst;
	canvas->Pixels = pixels;
	canvas->Width = cols * MT2D_SIXEL_CELL_W;
	canvas->Height = rows * MT2D_SIXEL_CELL_H;
	memcpy(canvas->Palette, Global_Palette, sizeof(canvas->Palette));
	for (cy = 0; cy < rows; cy++) {
		for (row = 0; row < MT2D_SIXEL_CELL_H; row++) {
			dst = pixels + (size_t)(cy * MT2D_SIXEL_CELL_H + row) * canvas->Width;
			for (cx = 0; cx < cols; cx++) {
				unsigned char bits = (unsigned char)(MT2D_Linux_Font_8x16[cells[cy * cols + cx] * 4 + row / 4] >> (24 - 8 * (row % 4)));
				for (bit = 0; bit < MT2D_SIXEL_CELL_W; bit++) {
					*dst++ = (unsigned char)(((bits >> (7 - bit)) & 1) ? fg : bg);
				}
			}
		}
	}
}

void MT2D_Sixel_Draw_Image(MT2D_Sixel_Canvas *canvas, const unsigned char *src, int srcW, int srcH, int x, int y, int w, int h) {
	int dx, dy, dx0, dx1, dy0, dy1;
	if (!src || srcW < 1 || srcH < 1 || w < 1 || h < 1) return;
	dx0 = x < 0 ? 0 : x;
	dy0 = y < 0 ? 0 : y;
	dx1 = x + w > canvas->Width ? canvas->Width : x + w;
	dy1 = y + h > canvas->Height ? canvas->Height : y + h;
	for (dy = dy0; dy < dy1; dy++) {
		const unsigned char *line = src + (size_t)((long)(dy - y) * srcH / h) * srcW;
		unsigned char *dst = canvas->Pixels + (size_t)dy * canvas->Width;
		for (dx = dx0; dx < dx1; dx++) {
			unsigned char v = line[(long)(dx - x) * srcW / w];
			if (v) dst[dx] = v; /* index 0 is transparent */
		}
	}
}

/* Output cursor: every write is checked, "full" is set when the buffer is too small. */
typedef struct Sixel_Out {
	char *p;
	char *end;
	int full;
} Sixel_Out;

static void Put_Char(Sixel_Out *o, char c) {
	if (o->p >= o->end) o->full = 1;
	else *o->p++ = c;
}

static void Put_Text(Sixel_Out *o, const char *s) {
	while (*s) Put_Char(o, *s++);
}

static void Put_Number(Sixel_Out *o, unsigned int n) {
	char tmp[12];
	int len = 0;
	do {
		tmp[len++] = (char)('0' + (n % 10));
		n /= 10;
	} while (n);
	while (len) Put_Char(o, tmp[--len]);
}

/* A run of "count" equal sixel chars, repeating (!n) when that is shorter. */
static void Put_Run(Sixel_Out *o, char c, unsigned int count) {
	if (count > 3) {
		Put_Char(o, '!');
		Put_Number(o, count);
		Put_Char(o, c);
	} else {
		while (count--) Put_Char(o, c);
	}
}

/* One pass of a band: the pixels of one color. The empty ('?') runs at the end are dropped. */
static void Encode_Color(Sixel_Out *o, const MT2D_Sixel_Canvas *canvas, int band, int color) {
	int x, k, y, bits;
	unsigned int run = 0, pending = 0; /* pending: '?' chars not written yet */
	char c, prev = 0;
	Put_Char(o, '#');
	Put_Number(o, (unsigned int)color);
	for (x = 0; x < canvas->Width; x++) {
		bits = 0;
		for (k = 0; k < 6; k++) {
			y = band * 6 + k;
			if (y < canvas->Height && canvas->Pixels[(size_t)y * canvas->Width + x] == color) bits |= 1 << k;
		}
		c = (char)(63 + bits);
		if (run && c == prev) {
			run++;
			continue;
		}
		if (run) {
			if (prev == '?') pending += run;
			else {
				Put_Run(o, '?', pending);
				pending = 0;
				Put_Run(o, prev, run);
			}
		}
		prev = c;
		run = 1;
	}
	if (run && prev != '?') {
		Put_Run(o, '?', pending);
		Put_Run(o, prev, run);
	}
}

unsigned int MT2D_Sixel_Encode(const MT2D_Sixel_Canvas *canvas, char *out, unsigned int outSize) {
	Sixel_Out o;
	int i, band, bands, k, x, first;
	unsigned char present[MT2D_SIXEL_COLORS], used[MT2D_SIXEL_COLORS];
	if (!out || !canvas || !canvas->Pixels || canvas->Width < 1 || canvas->Height < 1 || outSize < 2) return 0;
	o.p = out;
	o.end = out + outSize - 1; /* room for the '\0' */
	o.full = 0;
	bands = (canvas->Height + 5) / 6;
	/* ESC P 0;0;0 q : every pixel is painted by the pass of its color, so a frame replaces the previous one without
	   being cleared. "1;1;W;H = square pixels and the image size. */
	Put_Text(&o, "\033P0;0;0q\"1;1;");
	Put_Number(&o, (unsigned int)canvas->Width);
	Put_Char(&o, ';');
	Put_Number(&o, (unsigned int)canvas->Height);
	/* the palette colors the canvas uses, RGB in percent */
	for (i = 0; i < MT2D_SIXEL_COLORS; i++) used[i] = 0;
	for (k = 0; k < canvas->Width * canvas->Height; k++) used[canvas->Pixels[k]] = 1;
	for (i = 0; i < MT2D_SIXEL_COLORS; i++) {
		if (!used[i]) continue;
		Put_Char(&o, '#');
		Put_Number(&o, (unsigned int)i);
		Put_Text(&o, ";2;");
		Put_Number(&o, (unsigned int)canvas->Palette[i][0] * 100 / 255);
		Put_Char(&o, ';');
		Put_Number(&o, (unsigned int)canvas->Palette[i][1] * 100 / 255);
		Put_Char(&o, ';');
		Put_Number(&o, (unsigned int)canvas->Palette[i][2] * 100 / 255);
	}
	for (band = 0; band < bands; band++) {
		for (i = 0; i < MT2D_SIXEL_COLORS; i++) present[i] = 0;
		for (k = 0; k < 6 && band * 6 + k < canvas->Height; k++) {
			const unsigned char *line = canvas->Pixels + (size_t)(band * 6 + k) * canvas->Width;
			for (x = 0; x < canvas->Width; x++) present[line[x]] = 1;
		}
		first = 1;
		for (i = 0; i < MT2D_SIXEL_COLORS; i++) {
			if (!present[i]) continue;
			if (!first) Put_Char(&o, '$'); /* back to the left of the band for the next color */
			first = 0;
			Encode_Color(&o, canvas, band, i);
		}
		Put_Char(&o, '-'); /* next band */
		if (o.full) return 0;
	}
	Put_Text(&o, "\033\\");
	if (o.full) return 0;
	*o.p = '\0';
	return (unsigned int)(o.p - out);
}

int MT2D_Sixel_Reply_Supports_Sixel(const char *reply) {
	const char *p;
	int value, digits;
	if (!reply) return 0;
	while (*reply && !(reply[0] == 27 && reply[1] == '[' && reply[2] == '?')) reply++;
	if (!*reply) return 0;
	p = reply + 3;
	for (;;) {
		value = 0;
		digits = 0;
		while (*p >= '0' && *p <= '9') {
			if (value < 10000) value = value * 10 + (*p - '0');
			digits++;
			p++;
		}
		if (digits && value == 4 && (*p == ';' || *p == 'c')) return 1;
		if (*p == ';') p++;
		else return 0; /* 'c' (end of the list), or a reply that is cut or not a device attributes one */
	}
}

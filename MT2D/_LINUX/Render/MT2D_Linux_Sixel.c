/* MT2D Sixel encoder, see MT2D_Linux_Sixel.h. Portable C, no terminal access. */
#include <stddef.h>
#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "MT2D_Linux_Sixel.h"
#include "MT2D_Linux_Font.h"

void MT2D_Sixel_Draw_Text(MT2D_Sixel_Canvas *canvas, unsigned char *pixels, const unsigned char *cells, int cols, int rows) {
	int cx, cy, row, bit;
	unsigned char *dst;
	canvas->Pixels = pixels;
	canvas->Width = cols * MT2D_SIXEL_CELL_W;
	canvas->Height = rows * MT2D_SIXEL_CELL_H;
	canvas->Palette[0][0] = canvas->Palette[0][1] = canvas->Palette[0][2] = 0;
	canvas->Palette[1][0] = canvas->Palette[1][1] = canvas->Palette[1][2] = 217; /* light gray */
	canvas->Colors = 2;
	for (cy = 0; cy < rows; cy++) {
		for (row = 0; row < MT2D_SIXEL_CELL_H; row++) {
			dst = pixels + (size_t)(cy * MT2D_SIXEL_CELL_H + row) * canvas->Width;
			for (cx = 0; cx < cols; cx++) {
				unsigned char bits = (unsigned char)(MT2D_Linux_Font_8x16[cells[cy * cols + cx] * 4 + row / 4] >> (24 - 8 * (row % 4)));
				for (bit = 0; bit < MT2D_SIXEL_CELL_W; bit++) {
					*dst++ = (unsigned char)((bits >> (7 - bit)) & 1);
				}
			}
		}
	}
}

int MT2D_Sixel_Add_Color(MT2D_Sixel_Canvas *canvas, int r, int g, int b) {
	int i, best = 0;
	long d, bestd = -1;
	for (i = 0; i < canvas->Colors; i++) {
		long dr = r - canvas->Palette[i][0], dg = g - canvas->Palette[i][1], db = b - canvas->Palette[i][2];
		d = dr * dr + dg * dg + db * db;
		if (d == 0) return i;
		if (bestd < 0 || d < bestd) {
			bestd = d;
			best = i;
		}
	}
	if (canvas->Colors >= MT2D_SIXEL_COLORS) return best;
	canvas->Palette[canvas->Colors][0] = (unsigned char)r;
	canvas->Palette[canvas->Colors][1] = (unsigned char)g;
	canvas->Palette[canvas->Colors][2] = (unsigned char)b;
	return canvas->Colors++;
}

void MT2D_Sixel_Draw_Image(MT2D_Sixel_Canvas *canvas, const unsigned char *src, const unsigned char (*palette)[3], int colors,
	int srcW, int srcH, int x, int y, int w, int h) {
	int map[MT2D_SIXEL_COLORS]; /* palette index of the canvas for each color of the image, -1 = not added yet */
	int i, dx, dy, dx0, dx1, dy0, dy1;
	if (!src || srcW < 1 || srcH < 1 || w < 1 || h < 1) return;
	dx0 = x < 0 ? 0 : x;
	dy0 = y < 0 ? 0 : y;
	dx1 = x + w > canvas->Width ? canvas->Width : x + w;
	dy1 = y + h > canvas->Height ? canvas->Height : y + h;
	if (dx0 >= dx1 || dy0 >= dy1) return; /* nothing of it is on the canvas */
	if (colors > MT2D_SIXEL_COLORS) colors = MT2D_SIXEL_COLORS;
	for (i = 0; i < colors; i++) map[i] = -1; /* only the colors that are drawn enter the palette */
	for (dy = dy0; dy < dy1; dy++) {
		const unsigned char *line = src + (size_t)((long)(dy - y) * srcH / h) * srcW;
		unsigned char *dst = canvas->Pixels + (size_t)dy * canvas->Width;
		for (dx = dx0; dx < dx1; dx++) {
			unsigned char v = line[(long)(dx - x) * srcW / w];
			if (v && v < colors) {
				if (map[v] < 0) map[v] = MT2D_Sixel_Add_Color(canvas, palette[v][0], palette[v][1], palette[v][2]);
				dst[dx] = (unsigned char)map[v];
			}
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
	unsigned char present[MT2D_SIXEL_COLORS];
	if (!out || !canvas || !canvas->Pixels || canvas->Width < 1 || canvas->Height < 1 || outSize < 2) return 0;
	o.p = out;
	o.end = out + outSize - 1; /* room for the '\0' */
	o.full = 0;
	bands = (canvas->Height + 5) / 6;
	/* ESC P 0;0;0 q : P2 = 0 paints the pixels that are not set with the background (color 0), so a frame
	   replaces the previous one without being cleared. "1;1;W;H = square pixels and the image size. */
	Put_Text(&o, "\033P0;0;0q\"1;1;");
	Put_Number(&o, (unsigned int)canvas->Width);
	Put_Char(&o, ';');
	Put_Number(&o, (unsigned int)canvas->Height);
	for (i = 0; i < canvas->Colors; i++) { /* palette, RGB in percent */
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
		for (i = 1; i < canvas->Colors; i++) { /* color 0 is the background */
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

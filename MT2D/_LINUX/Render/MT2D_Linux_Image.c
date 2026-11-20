/* MT2D image sprites of the terminal graphics mode, see MT2D_Linux_Image.h. */
#include <stdlib.h>
#include <string.h>
#include "MT2D_Linux_Image.h"

const char *MT2D_Linux_Image_Path(const char *file, char *out, size_t outSize) {
	const char *base = file, *p, *dot;
	size_t n, i;
	for (p = file; *p; p++) {
		if (*p == '/' || *p == '\\') base = p + 1;
	}
	dot = strrchr(base, '.');
	if (!dot || strcmp(dot, ".png") != 0 || outSize < 16) return file;
	n = (size_t)(dot - base);
	if (n > 8) n = 8;
	strcpy(out, "d/");
	for (i = 0; i < n; i++) {
		char c = base[i];
		out[2 + i] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
	}
	strcpy(out + 2 + n, ".bmp");
	return out;
}

static unsigned long Le32(const unsigned char *b) {
	return (unsigned long)b[0] | ((unsigned long)b[1] << 8) | ((unsigned long)b[2] << 16) | ((unsigned long)b[3] << 24);
}

static unsigned int Le16(const unsigned char *b) {
	return (unsigned int)b[0] | ((unsigned int)b[1] << 8);
}

int MT2D_Linux_Load_BMP(FILE *file, Sprite *S) {
	unsigned char head[54];
	unsigned char pal[1024];
	unsigned long offset, dibSize, padding, colors;
	long width, height, row;
	MT2D_Internal_Image *image;
	int i;
	if (fread(head, 1, sizeof(head), file) != sizeof(head)) return 0;
	if (head[0] != 'B' || head[1] != 'M') return 0;
	offset = Le32(head + 10);
	dibSize = Le32(head + 14);
	width = (long)Le32(head + 18);
	height = (long)Le32(head + 22);
	if (dibSize < 40 || Le16(head + 28) != 8 || Le32(head + 30) != 0) return 0; /* 8 bpp, BI_RGB only */
	if (width <= 0 || width > 32767 || height <= 0 || height > 32767) return 0;
	colors = Le32(head + 46);
	if (colors == 0 || colors > 256) colors = 256;
	/* the palette is stored as B,G,R,0 right after the headers */
	if (fseek(file, 14L + (long)dibSize, SEEK_SET) != 0) return 0;
	memset(pal, 0, sizeof(pal));
	if (fread(pal, 4, (size_t)colors, file) != (size_t)colors) return 0;
	image = (MT2D_Internal_Image *)malloc(sizeof(MT2D_Internal_Image));
	if (!image) return 0;
	image->Pixels = (unsigned char *)malloc((size_t)width * (size_t)height);
	if (!image->Pixels) {
		free(image);
		return 0;
	}
	image->Colors = (int)colors;
	memset(image->Palette, 0, sizeof(image->Palette));
	for (i = 0; i < (int)colors; i++) {
		image->Palette[i][0] = pal[i * 4 + 2];
		image->Palette[i][1] = pal[i * 4 + 1];
		image->Palette[i][2] = pal[i * 4];
	}
	/* rows are stored bottom up and padded to 4 bytes */
	padding = (4UL - ((unsigned long)width % 4UL)) % 4UL;
	if (fseek(file, (long)offset, SEEK_SET) != 0) {
		MT2D_Linux_Image_Free(image);
		return 0;
	}
	for (row = height - 1; row >= 0; row--) {
		if (fread(image->Pixels + (size_t)row * (size_t)width, 1, (size_t)width, file) != (size_t)width ||
			(padding && fseek(file, (long)padding, SEEK_CUR) != 0)) {
			MT2D_Linux_Image_Free(image);
			return 0;
		}
	}
	S->Image = image;
	S->SizeX = (int)width;
	S->SizeY = (int)height;
	S->type = 1;
	return 1;
}

void MT2D_Linux_Image_Free(MT2D_Internal_Image *image) {
	if (image) {
		free(image->Pixels);
		free(image);
	}
}

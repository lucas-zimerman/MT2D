/**********************************************************
    MT2D_DosVGA_Render, created by Lucas Zimerman Fraulob

    this code render sprites and also ascii in a default
    256 color 320x200 display, the font used is hardcoded
    inside the font.h file.

    The screen is a grid of 80x25 tiles of 4x8 pixels, one
    tile per text cell. A tile is only redrawn when its char
    or the stack of sprites over it changed. A redrawn tile
    is built (glyph + sprites) in a 32 byte local buffer and
    written once to the video memory, as 8 dwords, so there
    is no flicker and no need of a 64KB double buffer (the
    MSDOS_USE_DOUBLE_BUFFER define no longer works, it does nothing).

    Memory: about 18KB of static data plus the sprite list
    and the tile node pool (4 bytes for each tile touched
    by a sprite, in 4KB chunks), both grow when needed and
    are kept between frames (no malloc/free per frame).

    Define MSDOS_VGA_WAIT_RETRACE to wait the vertical
    retrace before the first write of a frame.

    NOTE: This code has a hard coded limit to 32,767 sprites
    being rendered on screen.
***********************************************************/
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/_MSDOS/Render/dVgaRen.h> /* gives the functions C linkage, as the callers expect */
#ifdef __DJGPP__
#include <pc.h> /* outportb, inportb */
#else
#include <conio.h>
#define outportb outp
#define inportb inp
#endif
#include <MT2D/_MSDOS/Render/DosImg.h>
#include <MT2D/_MSDOS/Render/font.h>

#define TILES_X 80
#define TILES_Y 25
#define MAX_SPRITES 32767

extern unsigned char *VGA;
extern const unsigned char VGAFONT[];
extern unsigned char VGA_ASCII_COLORS[2]; /*store the index of the colors to be used by a char*/

/* What the renderer keeps of a sprite when it is added: the sprite itself can change or be freed before the render. */
typedef struct {
	const unsigned char *Pix; /* SizeX*SizeY palette indexes, row major, 0 is transparent */
	int X, Y, W, H;
} VgaSprite;

/* A node of the list of sprites over a tile. The newest sprite is the head of the list, so it is drawn first.
   A node is one dword: the index in Sprites (15 bits) and, above it, the node index + 1 of the next (older) sprite on the
   tile (17 bits, 0 = end of list). Nodes are kept in chunks of 4KB that are never moved or copied. */
#define NODE_SPRITE_BITS 15
#define NODE_SPRITE_MASK 0x7FFFUL
#define NODE_CHUNK_BITS 10
#define NODE_CHUNK_SIZE (1UL << NODE_CHUNK_BITS)
#define NODE_CHUNKS 128
#define NODE_MAX 131071UL /* the next index + 1 must fit in 17 bits */
#define NODE_AT(i) (NodeChunks[(i) >> NODE_CHUNK_BITS][(i) & (NODE_CHUNK_SIZE - 1)])

static VgaSprite *Sprites = 0;
static unsigned long SpritesCnt = 0, SpritesCap = 0;
static unsigned long *NodeChunks[NODE_CHUNKS];
static unsigned long NodesCnt = 0;
static long TileHead[TILES_Y][TILES_X];                  /* node index + 1 of the top sprite, 0 = no sprite (zeroed at start) */
static unsigned long TilePrevSig[TILES_Y][TILES_X];      /* signature of the sprite stack that is on screen */
static unsigned char VGA_ASCII_Backup[TILES_Y][TILES_X]; /* char that is on screen */
static unsigned long NibbleColor[16];                    /* 4 font bits -> 4 pixels (one dword) with the current colors */
static unsigned char CachedColors[2];
static int NibbleValid = 0;
static int ForceRedraw = 1;                              /* draw every tile in the next render */
static int HadSprites = 0;                               /* there were sprites in the previous render */

void MT2D_Dos_Set_Palette(const unsigned char *bgra, int count){
	int i;
	outportb(0x3C8, 0);/*first DAC index to write, it auto increments*/
	for(i = 0; i < count; i++){
		outportb(0x3C9, bgra[i*4 + 2] >> 2);/*R, the DAC only has 6 bits*/
		outportb(0x3C9, bgra[i*4 + 1] >> 2);/*G*/
		outportb(0x3C9, bgra[i*4 + 0] >> 2);/*B*/
	}
}

void MT2D_Dos_Render_Invalidate(void){
	ForceRedraw = 1;
}

void MT2D_Dos_Render_Free(void){
	int i;
	free(Sprites); Sprites = 0; SpritesCnt = 0; SpritesCap = 0;
	for (i = 0; i < NODE_CHUNKS; i++) { free(NodeChunks[i]); NodeChunks[i] = 0; }
	NodesCnt = 0;
	memset(TileHead, 0, sizeof(TileHead));
	HadSprites = 0;
	ForceRedraw = 1;
}

/*Forgets the sprites of the last frame, the sprite memory is kept to be reused.*/
void Dos_Clear_ImgBuffer(void){
	if (SpritesCnt > 0) {
		memset(TileHead, 0, sizeof(TileHead));
		SpritesCnt = 0;
		NodesCnt = 0;
	}
}

static int GrowSprites(void){
	unsigned long cap = (SpritesCap ? SpritesCap * 2 : 64);
	VgaSprite *p = (VgaSprite*)realloc(Sprites, cap * sizeof(VgaSprite));
	if (!p) return 0;
	Sprites = p;
	SpritesCap = cap;
	return 1;
}

/*Adds a node at the head of the tile list, returns 0 when there is no memory (the sprite is then missing on that tile).*/
static int AddNode(long *head, unsigned long sprite){
	unsigned long chunk = NodesCnt >> NODE_CHUNK_BITS;
	if (NodesCnt >= NODE_MAX) return 0;
	if (!NodeChunks[chunk]) {
		NodeChunks[chunk] = (unsigned long*)malloc(NODE_CHUNK_SIZE * sizeof(unsigned long));
		if (!NodeChunks[chunk]) return 0;
	}
	NODE_AT(NodesCnt) = ((unsigned long)*head << NODE_SPRITE_BITS) | sprite;
	NodesCnt++;
	*head = (long)NodesCnt; /*index + 1*/
	return 1;
}

/*Store a copy of the data needed to draw the sprite and mark the tiles that it touches.*/
void Dos_Add_ImagetoBuffer(Sprite *IMG,int X, int Y) {
	VgaSprite *s;
	int x0, x1, y0, y1, tx, ty, tx0, tx1, ty0, ty1;
	unsigned long id;
	if (!IMG || !IMG->Image || !IMG->Image->Pixels) return;
	if (IMG->SizeX <= 0 || IMG->SizeY <= 0 || X >= 320 || Y >= 200 || X + IMG->SizeX <= 0 || Y + IMG->SizeY <= 0) {
		return; /*nothing of it is on the screen*/
	}
	if (SpritesCnt >= MAX_SPRITES || (SpritesCnt == SpritesCap && !GrowSprites())) return;
	id = SpritesCnt++;
	s = &Sprites[id];
	s->Pix = IMG->Image->Pixels;
	s->X = X; s->Y = Y; s->W = IMG->SizeX; s->H = IMG->SizeY;

	/*screen area of the sprite, then the first and last tile that it touches (this snaps to the tile grid)*/
	x0 = (X < 0 ? 0 : X);
	x1 = (X + s->W > 320 ? 320 : X + s->W);
	y0 = (Y < 0 ? 0 : Y);
	y1 = (Y + s->H > 200 ? 200 : Y + s->H);
	tx0 = x0 >> 2; tx1 = (x1 - 1) >> 2;
	ty0 = y0 >> 3; ty1 = (y1 - 1) >> 3;
	for (ty = ty0; ty <= ty1; ty++) {
		for (tx = tx0; tx <= tx1; tx++) {
			if (!AddNode(&TileHead[ty][tx], id)) return;
		}
	}
}

/*Number that changes when any sprite, its position or its size changes in the stack of the tile (0 is never returned for a used tile).*/
static unsigned long TileSignature(long head){
	unsigned long sig = 1, h;
	while (head) {
		unsigned long n = NODE_AT(head - 1);
		const VgaSprite *s = &Sprites[n & NODE_SPRITE_MASK];
		h = (unsigned long)s->Pix;
		h = h * 31 + (unsigned long)s->X;
		h = h * 31 + (unsigned long)s->Y;
		h = h * 31 + (unsigned long)s->W;
		h = h * 31 + (unsigned long)s->H;
		sig = (sig ^ h) * 16777619UL;
		head = (long)(n >> NODE_SPRITE_BITS);
	}
	return sig ? sig : 1;
}

/*Draws the sprites of the tile over Tile (8 rows of 4 pixels), from the top sprite to the bottom one.*/
static void ComposeTile(unsigned char *Tile, int tx, int ty, long head){
	unsigned long done = 0;/*one bit for each pixel of the tile that already has a sprite on it*/
	int px = tx << 2, py = ty << 3;
	int c0, c1, r0, r1, r, c;
	while (head && done != 0xFFFFFFFFUL) {
		unsigned long n = NODE_AT(head - 1);
		const VgaSprite *s = &Sprites[n & NODE_SPRITE_MASK];
		const unsigned char *src;
		head = (long)(n >> NODE_SPRITE_BITS);
		c0 = (s->X > px ? s->X - px : 0);
		c1 = s->X + s->W - px; if (c1 > 4) c1 = 4;
		r0 = (s->Y > py ? s->Y - py : 0);
		r1 = s->Y + s->H - py; if (r1 > 8) r1 = 8;
		if (c0 >= c1 || r0 >= r1) continue;
		src = s->Pix + (py + r0 - s->Y) * s->W + (px + c0 - s->X);
		for (r = r0; r < r1; r++, src += s->W) {
			for (c = c0; c < c1; c++) {
				unsigned long bit = 1UL << (r * 4 + c);
				if (!(done & bit) && src[c - c0]) {
					Tile[r * 4 + c] = src[c - c0];
					done |= bit;
				}
			}
		}
	}
}

static void BuildNibbleTable(void){
	int n;
	for (n = 0; n < 16; n++) {
		/*bit 3 is the pixel on the left, the first byte of the dword is the pixel on the left*/
		NibbleColor[n] = (unsigned long)VGA_ASCII_COLORS[(n >> 3) & 1]
		               | ((unsigned long)VGA_ASCII_COLORS[(n >> 2) & 1] << 8)
		               | ((unsigned long)VGA_ASCII_COLORS[(n >> 1) & 1] << 16)
		               | ((unsigned long)VGA_ASCII_COLORS[n & 1] << 24);
	}
	CachedColors[0] = VGA_ASCII_COLORS[0];
	CachedColors[1] = VGA_ASCII_COLORS[1];
	NibbleValid = 1;
}

/*
   32 chars for each line are stored in VGAFONT, each char has a bit representing a pixel if its on or off.
   so the ascii char is stored in a format like [4][1] where a single char stores two lines of chars (
   because in our case a char has the size of 8x4 instead of 8x8)
*/
void MT2D_Dos_Render_VGA_ASCII(unsigned char BUFFER[][80]){
	unsigned long Tile32[8];
	unsigned char *Tile = (unsigned char*)Tile32;
	unsigned long sig;
	unsigned char *dst;
	long head;
	int tx, ty, i, r;
	unsigned short Offset;
	unsigned char ch, b;
	int active = (SpritesCnt > 0 || HadSprites);
	#ifdef MSDOS_VGA_WAIT_RETRACE
	int waited = 0;
	#endif

	if (!NibbleValid || CachedColors[0] != VGA_ASCII_COLORS[0] || CachedColors[1] != VGA_ASCII_COLORS[1]) {
		BuildNibbleTable();
		ForceRedraw = 1;
	}
	for (ty = 0; ty < TILES_Y; ty++) {
		/*without sprites a whole line that did not change is skipped with a single compare*/
		if (!ForceRedraw && !active && memcmp(VGA_ASCII_Backup[ty], BUFFER[ty], TILES_X) == 0) continue;
		for (tx = 0; tx < TILES_X; tx++) {
			head = TileHead[ty][tx];
			sig = (head ? TileSignature(head) : 0);
			ch = BUFFER[ty][tx];
			if (!ForceRedraw && ch == VGA_ASCII_Backup[ty][tx] && sig == TilePrevSig[ty][tx]) continue;
			VGA_ASCII_Backup[ty][tx] = ch;
			TilePrevSig[ty][tx] = sig;
			/*the font stores 2 lines of 4 pixels in each byte (high nibble = first line)*/
			Offset = (ch & 31) + ((ch >> 5) << 7);
			for (i = 0; i < 8; i += 2) {
				b = VGAFONT[Offset];
				Tile32[i] = NibbleColor[b >> 4];
				Tile32[i + 1] = NibbleColor[b & 15];
				Offset += 32;
			}
			if (head) ComposeTile(Tile, tx, ty, head);
			#ifdef MSDOS_VGA_WAIT_RETRACE
			if (!waited) {
				while (inportb(0x3DA) & 8) {}   /*leave the current retrace*/
				while (!(inportb(0x3DA) & 8)) {}/*wait the next one*/
				waited = 1;
			}
			#endif
			dst = VGA + ty * (8 * 320) + tx * 4;
			for (r = 0; r < 8; r++, dst += 320) {
				*(unsigned long*)dst = Tile32[r];
			}
		}
	}
	ForceRedraw = 0;
	HadSprites = (SpritesCnt > 0);
}

#endif

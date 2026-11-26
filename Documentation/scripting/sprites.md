# Sprites

Header: `MT2D/ObjectCore/Sprites.h` (implementation `Sprites.c`, `MT2D_Text_Image.*`, editing: `SprRaw.*`). Works in text mode.

```c
struct Sprite {
    MT2D_Text_Image *Text;       /* type 0, NULL otherwise */
    MT2D_Internal_Image *Image;  /* type 1, NULL otherwise */
    int SizeX, SizeY;    /* size of the source */
    int ScaleX, ScaleY;  /* size drawn on the 320x240 screen (image sprites only) */
    char type;           /* 0 = ASCII image, 1 = image */
    unsigned int refCount;
};
```

## Two kinds

| | ASCII sprite (`type 0`) | Image sprite (`type 1`) |
| --- | --- | --- |
| Backends | **all** (text, SDL, DOS VGA, Linux Sixel) | SDL, DOS VGA, Linux Sixel only ([../gui/limitations.md](../gui/limitations.md)) |
| Load | `Load_Sprite(file)`, `Sprite_Create_Ascii_From_Cells(w, h, cells)`, `MT2D_Sprite_Create_From_Raw(raw, false, ...)` | `Load_Sprite_Image(file, sx, sy)`, `Load_Sprite_Image_From_Container(...)`, `MT2D_Sprite_Create_From_Raw(raw, true, sx, sy)` |
| Position unit | text cells: `x` = column, `y` = row | pixels on the 320x240 virtual screen |
| Drawn | immediately into the window (`WINDOW1` for `NULL`) | queued, shown by the next `MT2D_Draw_Window` |
| Scale | ignored | `ScaleX/ScaleY` |

## Functions

| Function | Notes |
| --- | --- |
| `Sprite *Load_Sprite(char *file)` | ASCII sprite from a file, `NULL` when the file cannot be opened. Format below |
| `Sprite *Sprite_Create_Ascii_From_Cells(int w, int h, const char *cells)` | `w*h` bytes, row major, **`0` = transparent**. `NULL` if width, height or opaque cell count is over 65535, or no memory |
| `char Sprite_Get_Char(const Sprite *s, int x, int y)` | `0` for transparent, out of range, `NULL` or image sprites |
| `Sprite *Load_Sprite_Image(char *file, int ScaleX, int ScaleY)` | `NULL` when the file is missing. The examples pass `0, 0` and then set `s->ScaleX = s->SizeX` |
| `bool Sprite_Render_on_Window(Sprite *s, unsigned char (*window)[MAX_HOR], int x, int y)` | see below |
| `void Sprite_Delete(Sprite *s)` | decrements `refCount`, frees at 0 |

`Sprite_Render_on_Window`:
* ASCII: copied into `window` (`WINDOW1` for `NULL`) right now, clipped to 80x25, transparent cells skipped, returns `true`.
* Image: only for the main screen (`NULL` or `WINDOW1`), any other window returns `false`. SDL queues it and still returns **`false`**; DOS VGA queues it and returns `true`; targets
  without image support draw nothing and return `false`. Do not use the return value as an error check.
* `NULL` sprite returns `false`.

Draw order is call order; a later sprite or `insert_string_on_display` overwrites an earlier one in the buffer.
`MT2D_Clear_Main_Window()` first, every frame.

## ASCII sprite file (`Load_Sprite`)

Plain bytes (CP850), one row per line. The loader treats byte **13** as the end of a line and skips the byte
after it, so write lines ending in **CR LF**. A file with only LF (`\n`) has no line break for the loader: the whole file
becomes one row with byte 10 cells.

* The sprite width is the longest line (plus a column or two of transparent padding, the counting loop is off by one);
  shorter lines are padded with transparent cells.
* **A space (32) is an opaque character**. Only a missing cell (past the end of a short line) is transparent.
  To make holes, leave the end of the row empty or build the sprite with `Sprite_Create_Ascii_From_Cells` and `0` bytes.
* Characters above 127 are CP850 box and shade characters (see `examples/allchars`).
* The height is the number of CR bytes plus one.
* Read from the loader code, not run: its `while (!EOF)` loop reads one byte past the end of the file, so one extra opaque
  cell with the value 255 (a blank in CP850) is stored after the last character, in the extra last row when the file ends with CR LF.
  It overwrites one cell of whatever is under the sprite. If that matters, build the sprite with `Sprite_Create_Ascii_From_Cells`.

## Reference counting and editing

* `refCount` starts at 0. `MT2D_ObjectState_ADD` increments it for the sprite it stores; `Sprite_Delete` (also called
  when a state is deleted) decrements it. A sprite shared by several frames is freed once, after the last one.
* ASCII sprites are compact read-only slices, there is no `Data` array anymore ([`../../MIGRATION.md`](../../MIGRATION.md)).
  Edit them (and image sprites) with an `MT2D_SpriteRaw`, below.
* `ObjectCore_AddGlobalSprite(path, sprite)` makes the script loader share a sprite for a `Show "path"` ([../Object-Scripts.md](../Object-Scripts.md)).

## Editing sprites (`MT2D_SpriteRaw`)

Header: `MT2D/ObjectCore/SprRaw.h` (included by `Sprites.h`). A raw sprite is uncompressed content you edit directly. It
can not be rendered: make a sprite from it, or write it into one.

```c
typedef struct { unsigned char R, G, B, A; } MT2D_RGBA;   /* 4 bytes, A: 0 = transparent, 255 = opaque */
typedef struct {
    int Width, Height;
    bool IsRgba;                                           /* false: 1 byte cells, true: MT2D_RGBA cells */
    union { unsigned char *Cells; MT2D_RGBA *Rgba; } Data; /* row by row: cell (x,y) is Data.Cells[y * Width + x] */
} MT2D_SpriteRaw;
```

The cells have the layout of a C array `cells[Height][Width]`, so a raw can also point to your own array
(`r.Data.Cells = &block[0][0]`, only free the raws the engine made). `0` (or `A = 0`) is transparent.

Which raw a sprite takes:

| Sprite | Cells |
| --- | --- |
| ASCII (every build) | 1 byte: characters |
| Image, MS-DOS VGA / Linux Sixel | 1 byte: palette indexes (the palette of the screen) |
| Image, SDL | RGBA (semi transparency works) |

A raw of the other format is refused (`NULL` / `0`).

| Function | Notes |
| --- | --- |
| `MT2D_SpriteRaw *MT2D_SpriteRaw_Create(int w, int h, bool isRgba)` | all cells 0, one allocation. `NULL` for sizes <= 0 or > 32767 |
| `MT2D_SpriteRaw *MT2D_SpriteRaw_Clip(const MT2D_SpriteRaw *raw, int x, int y, int w, int h)` | new raw with that area, **shrunk to the part inside `raw`** (check its `Width`/`Height`), `NULL` when it misses |
| `void MT2D_SpriteRaw_Free(MT2D_SpriteRaw *raw)` | `NULL` is accepted |
| `Sprite *MT2D_Sprite_Create_From_Raw(const MT2D_SpriteRaw *raw, bool image, int sx, int sy)` | copies `raw`. A blank sprite is made from a new raw. `sx`/`sy` <= 0 use the size of `raw`. `NULL` for `image` on a build without image sprites |
| `int MT2D_Sprite_Alter_Block(Sprite *s, int x, int y, const MT2D_SpriteRaw *raw)` | writes `raw` at `(x,y)`, clipped to the sprite. Every cell is replaced (0 erases). Returns the cells written. A pixel is a 1x1 raw |
| `MT2D_SpriteRaw *MT2D_Sprite_To_SpriteRaw(const Sprite *s)` | a copy of the content, in the format the sprite takes. **Heavy call, see below** |

```c
MT2D_SpriteRaw *r = MT2D_Sprite_To_SpriteRaw(s);     /* or MT2D_SpriteRaw_Create(w, h, isRgba) */
r->Data.Cells[1 * r->Width + 2] = 12;                /* (x=2, y=1) */
MT2D_Sprite_Alter_Block(s, 0, 0, r);
MT2D_SpriteRaw_Free(r);

/* a tile of a sheet */
MT2D_SpriteRaw *all  = MT2D_Sprite_To_SpriteRaw(sheet);
MT2D_SpriteRaw *tile = MT2D_SpriteRaw_Clip(all, 16, 0, 16, 16);
Sprite *t = MT2D_Sprite_Create_From_Raw(tile, true, 16, 16);
```

How it works on each backend:

* ASCII: the compressed image is expanded, edited and compressed again, `O(width * height)` per `Alter_Block` call:
  write one big block, not many 1x1 ones. If the new content can not be compressed (more than 65535 opaque cells,
  no memory) the sprite is left as it was and `0` is returned.
* MS-DOS VGA / Linux Sixel: a copy into the pixels of the image.
* SDL: `SDL_UpdateTexture` on the texture and its rotated copy (`GPU_UpdateImageBytes` with `MT2D_SDL_GPU`, RGBA images
  only), the textures are kept so sprites already queued for `MT2D_Draw_Window` show the new content.
  `MT2D_Sprite_To_SpriteRaw` draws the sprite 1:1 into a temporary render target and reads it back with
  `SDL_RenderReadPixels`: it waits for the GPU, **do not call it every frame**. Keep the raw and edit it instead.
  `NULL` when the renderer has no render target support.

## Tips

* Parallax / scrolling with images: draw the same sprite at `x` and `x + 320` (`examples/graphics`).
* Always check the `Load_*` result; running from the wrong folder is the usual failure.
* Sample assets: cyan = transparent in the SDL PNGs, palette index 0 = transparent in the DOS VGA BMPs.

# Sprites

Header: `MT2D/ObjectCore/Sprites.h` (implementation `Sprites.cpp`, `MT2D_Text_Image.*`). Works in text mode.

```c
struct Sprite {
    union { MT2D_Text_Image *Text; MT2D_Internal_Image *Image; };  /* member chosen by `type` */
    int SizeX, SizeY;    /* size of the source */
    int ScaleX, ScaleY;  /* size drawn on the 320x240 screen (image sprites only) */
    char type;           /* 0 = ASCII image, 1 = image */
    unsigned int refCount;
};
```

## Two kinds

| | ASCII sprite (`type 0`) | Image sprite (`type 1`) |
| --- | --- | --- |
| Backends | **all** (text, SDL, DOS VGA) | SDL, DOS VGA only ([../gui/limitations.md](../gui/limitations.md)) |
| Load | `Load_Sprite(file)`, `Sprite_Create_Ascii_From_Cells(w, h, cells)` | `Load_Sprite_Image(file, sx, sy)`, `Load_Sprite_Image_From_Container(...)` |
| Position unit | text cells: `x` = column, `y` = row | pixels on the 320x240 virtual screen |
| Drawn | immediately into `WINDOW1` / `WINDOW2` | queued, shown by the next `MT2D_Draw_Window` |
| Scale | ignored | `ScaleX/ScaleY` |

## Functions

| Function | Notes |
| --- | --- |
| `Sprite *Load_Sprite(char *file)` | ASCII sprite from a file, `NULL` when the file cannot be opened. Format below |
| `Sprite *Sprite_Create_Ascii_From_Cells(int w, int h, const char *cells)` | `w*h` bytes, row major, **`0` = transparent**. `NULL` if width, height or opaque cell count is over 65535, or no memory |
| `char Sprite_Get_Char(const Sprite *s, int x, int y)` | `0` for transparent, out of range, `NULL` or image sprites |
| `Sprite *Load_Sprite_Image(char *file, int ScaleX, int ScaleY)` | `NULL` when the file is missing. The examples pass `0, 0` and then set `s->ScaleX = s->SizeX` |
| `bool Sprite_Render_on_Window(Sprite *s, int which, int x, int y)` | see below |
| `void Sprite_Delete(Sprite *s)` | decrements `refCount`, frees at 0 |

`Sprite_Render_on_Window`:
* ASCII: copied into `WINDOW1` (or `WINDOW2`) right now, clipped to 80x25, transparent cells skipped, returns `true`.
* Image: SDL queues it and still returns **`false`**; DOS VGA queues it (only for `DISPLAY_WINDOW1`) and returns `true`; targets
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
* ASCII sprites are compact read-only slices: you cannot write a cell. To edit, read cells with `Sprite_Get_Char`, build a
  new sprite and `Sprite_Delete` the old one ([`../../MIGRATION.md`](../../MIGRATION.md)). There is no `Data` array anymore.
* `ObjectCore_AddGlobalSprite(path, sprite)` makes the script loader share a sprite for a `Show "path"` ([../Object-Scripts.md](../Object-Scripts.md)).

## Tips

* Parallax / scrolling with images: draw the same sprite at `x` and `x + 320` (`examples/graphics`).
* Always check the `Load_*` result; running from the wrong folder is the usual failure.
* Sample assets: cyan = transparent in the SDL PNGs, palette index 0 = transparent in the DOS VGA BMPs.

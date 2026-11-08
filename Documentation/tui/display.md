# Display (text screen)

Headers: `MT2D/MT2D.h`, `MT2D/MT2D_Display.h`, `MT2D/MT2D_Layout.h`, `MT2D/Cursor/Cursor.h`,
`MT2D/Tools/Video/MT2D_Generic_Line.h`.

## The buffers

```c
extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR]; /* MAX_VER 25, MAX_HOR 80: [row][column] */
extern unsigned char WINDOW2[MAX_VER + 1][MAX_HOR];
#define DISPLAY_WINDOW1 1
#define DISPLAY_WINDOW2 0
```

* One byte per cell, a CP850 (MS-DOS) character. There is no colour or attribute plane.
* `WINDOW1` is your main screen. `WINDOW2` is scratch space: menus and popups copy `WINDOW1` into it,
  draw on top and show `WINDOW2`, so `WINDOW1` stays intact underneath.
* Nothing is visible until `MT2D_Draw_Window(which)`.
* Coordinates are always **row first, then column** in `insert_*`, and **x first** in `MT2D_Create_Window_Layout`.

## Functions

| Function | Notes |
| --- | --- |
| `MT2D_Init()` | call once before anything else (macro for the backend init) |
| `MT2D_Clear_Main_Window()` | clears `WINDOW1` (on image backends it also clears the images of the last frame) |
| `MT2D_Draw_Window(which)` | shows `WINDOW1` (`DISPLAY_WINDOW1`) or `WINDOW2` |
| `insert_string_on_display(char *s, row, col, which)` | copies `s` into the buffer. Only the column is checked (`col > MAX_HOR` stops it): keep `col + strlen(s) <= 80` and `row <= 24` yourself |
| `insert_number_on_display(int n, row, col, which)` | writes the digits (and `-`) with no terminator and does not clear leftover cells: clear or overwrite the width yourself |
| `transfer_window1_to_window2()` / `transfer_window2_to_window1()` | copy rows 0..23 (the last row, 24, is not copied) |
| `MT2D_Create_Window_Layout(x, y, w, h, shadow, title, option, background, which)` | draws a double line box (CP850 201/187/200/188/205/186) from (x,y) to (x+w, y+h) inclusive, fills the inside with `background`. `shadow`: `176` shade on the right and bottom. `title`: separator line at row y+2. `option`: separator line at row y+h-2 |
| `MT2D_Line_Draw_Vertical(pos, ch)` | fills **row** `pos` of `WINDOW1` with `ch` (it looks horizontal on screen) |
| `MT2D_Line_Draw_Horizontal(pos, ch)` | fills **column** `pos` of `WINDOW1` with `ch` (a vertical line on screen) |
| `MT2D_Cursor_goto_topscreen()`, `MT2D_Cursor_Set_Position(line, column)` | move the hardware cursor; they do nothing with SDL |

Typical frame:

```c
MT2D_Clear_Main_Window();
MT2D_Create_Window_Layout(10, 5, 40, 8, true, true, false, ' ', DISPLAY_WINDOW1);
insert_string_on_display((char *)"Score", 6, 12, DISPLAY_WINDOW1);
insert_number_on_display(score, 6, 20, DISPLAY_WINDOW1);
MT2D_Draw_Window(DISPLAY_WINDOW1);
```

ASCII sprites (any backend) are copied into these same buffers and image sprites are drawn on top of them, see
[../scripting/sprites.md](../scripting/sprites.md).
Debug text: `MT2D_Ide_Printf(text)` (`MT2D_Debug.h`) has a Windows and an SDL implementation and compiles to nothing
elsewhere unless `MT2D_DEBUG_ENABLED` (which is an `#error` for platforms without code).

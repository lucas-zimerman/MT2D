# Menus

Header: `MT2D/Tools/UserInterface/MT2D_Menu.h`. Example: `examples/menu`.

```c
int MT2D_Short_Menu(char *title, int title_position, int x, int y, bool shadow,
                    char background, char cursor,
                    char *opt1, char *opt2, char *opt3, char *opt4);

int MT2D_Menu(char *title, int title_position, int x, int y, int style, bool shadow,
              char background, char cursor,
              char *opt1, ..., char *opt18, int total_option);
```

* Returns the chosen option, **1..n** (never 0 or -1 in normal use).
* `MT2D_Short_Menu`: up to 4 options. It counts the non-NULL ones, so pass them in order and use `NULL`
  for the unused tail.
* `MT2D_Menu`: up to 18 options, always pass all 18 `char *` (use `0` for unused) plus `total_option`.
  `total_option <= 0` opens an error box (`DEV MENU ERROR`) and either exits the program or returns -1.
* `x` = column (`horizontal_position`), `y` = row (`vertical_position`). `y` is moved up if the menu would not
  fit. `title_position` = column offset of the title inside the box. The box is 55 columns wide and
  `5 + total_option` rows tall.
* Input: up/down arrows (wraps around), `Enter`, or the digit of the option (`1`-`9` only, entries 10+ need arrows).
  **There is no cancel**: `ESC` does nothing. Add a "Back" entry.
* The marker is always `>`; pass any character for `cursor` and `0` for `style`.
* It blocks until a choice is made. It draws on `WINDOW2` (a copy of `WINDOW1`), so `WINDOW1` is not
  modified, but the menu stays on the screen until you redraw `WINDOW1`.

```c
int choice = MT2D_Short_Menu((char *)"MAIN MENU", 20, 10, 5, true, ' ', '>',
                             (char *)"Start", (char *)"Options", (char *)"Exit", NULL);
MT2D_Draw_Window(DISPLAY_WINDOW1);   /* remove the menu from the screen */
```

For a two option question use `MT2D_MessageBox_With_Result`, see [io.md](io.md).

# Input / output helpers

Popups, input boxes and system calls. All popups copy `WINDOW1` to `WINDOW2`, draw on `WINDOW2` and **block**
until finished. `WINDOW1` is not changed, but the popup stays on screen until you call
`MT2D_Draw_Window(DISPLAY_WINDOW1)`. Examples: `examples/inputpopup`, `examples/playpcspeaker`.

## Message boxes

| Header | Call | Behaviour |
| --- | --- | --- |
| `MessageBox/MT2D_MessageBox.h` | `void MT2D_MessageBox(char *message)` | box at (3,9), 73x4, text wraps (about 3 lines of 69 columns). Beeps (`printf("\a")`). The arrow keys **move the box**, any other key closes it |
| `MessageBox/MT2D_MessageBox_With_Result.h` | `int MT2D_MessageBox_With_Result(char *title, char *message, char *option_1, char *option_2)` | box at (9,6), 50x8. Waits for `1` or `2` and returns that number. Option labels are cut at about 21 columns. No other key closes it |

## Input boxes

| Header | Call | Returns |
| --- | --- | --- |
| `InputBox/MT2D_InputBox_String.h` | `char *MT2D_InputBox_String(char *prompt)` | the text, `malloc`ed: **you must `free()` it** |
| `InputBox/MT2D_InputBox_Password.h` | `char *MT2D_InputBox_Password(char *prompt)` | same, but the box shows `*` |
| `InputBox/MT2D_InputBox_Integer.h` | `int MT2D_InputBox_Integer(char *prompt)` | the number (digits and a leading `-` only, parsed with `atoi`, so very long input overflows) |

* The prompt is wrapped in a 73x6 box at (3,9); the visible input is the last 70 characters.
* `Enter` finishes, `Backspace` erases the last character, arrows are ignored.
* `ESC` returns an **empty string** `""` and `0` for the integer box.
* Erasing the only typed character and pressing `Enter` returns `" "` (one space).
* They poll with `MT2D_System_Delay(1)` so they do not use 100% CPU.

```c
char *name = MT2D_InputBox_String((char *)"Your name?");
if (name && name[0] != '\0') { /* use it */ }
free(name);
int age = MT2D_InputBox_Integer((char *)"Your age?");
```

## System calls (`MT2D/MT2D_System_Calls.h`)

| Call | Behaviour |
| --- | --- |
| `MT2D_System_Delay(ms)` | sleeps the calling thread for `ms` milliseconds. Use it to pace a game loop (e.g. 30 ms) |
| `MT2D_System_Beep(frequency, ms)` | PC speaker beep (emulated on modern OSes, sound card with SDL), see [../general/audio.md](../general/audio.md) |

Files: [../general/files.md](../general/files.md). Packed assets: [../general/container.md](../general/container.md). Keyboard: [../general/keyboard.md](../general/keyboard.md).

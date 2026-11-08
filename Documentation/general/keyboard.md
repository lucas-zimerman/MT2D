# Keyboard

Header: `MT2D/MT2D_Keyboard.h` (defines the `arrow_*` / `*_pressed` constants too). Same API in text mode and with SDL.
For keys that stay held or several keys at once use the [joystick](joystick.md) API instead.

| Call | Behaviour |
| --- | --- |
| `MT2D_Keyboard_touched()` | non-blocking, 1 if a key is waiting, else 0. Use it in game loops |
| `MT2D_Keyboard_keytouched()` | **blocks** until a key is pressed and returns its code |

With SDL these two also pump the SDL event queue, so call them (or `MT2D_Joystick_Update`) every frame or
the window stops answering.

## Key codes

Same codes as MS-DOS / Windows `getch`.

| Constant | Value |
| --- | --- |
| `back_space_pressed` | 8 |
| `tab_pressed` | 9 |
| `enter_pressed` | 13 |
| `escape_pressed` | 27 |
| `arrow_key_pressed` | 224 (prefix, see below) |
| `arrow_up_pressed` / `arrow_down_pressed` / `arrow_left_pressed` / `arrow_right_pressed` | 72 / 80 / 75 / 77 |

Printable keys return their ASCII value (letters are upper case when shift or caps lock is on).

**Special keys take two reads.** The first `MT2D_Keyboard_keytouched()` returns `224`, the second returns
the key (the arrow constants above; on SDL F1..F12 return `224` then 59..70):

```c
int key = MT2D_Keyboard_keytouched();
if (key == arrow_key_pressed) {
    key = MT2D_Keyboard_keytouched();   /* always consume the second code */
    if (key == arrow_up_pressed) { /* ... */ }
} else if (key == escape_pressed) { /* ... */ }
```

Read the second code even when you ignore it, otherwise it shows up as a bogus key on the next read.
Loop to drain input: `while (MT2D_Keyboard_touched()) { key = MT2D_Keyboard_keytouched(); ... }`.

## Notes

* One key event per call: there is no "key released" and no way to ask whether a key is currently down.
  Auto repeat decides how often a held key arrives. Use [joystick.md](joystick.md) for held keys.
* SDL: the SDL backend turns SDL keysyms into these codes (letters 97..122 lowercase unless shift/caps,
  keysyms below 97 pass through, arrows and F1..F12 become the two read form). Other keys leave the previous
  code in the emulation variables, so do not rely on keys outside that list.
* Do not mix with `MT2D_Joystick_*` in the same loop: both consume the same queue.

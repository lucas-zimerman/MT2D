# MT2D

**Monochrome Text 2D Engine** is a small C/C++ framework for text based programs and games. The screen is an 80x25 grid of
characters that is drawn on a real terminal, or on an emulated one (an SDL window or the MS-DOS VGA mode) that can also draw image
sprites. The same program runs on many systems, from MS-DOS to Android.

It ships with message boxes, input boxes, menus, keyboard and joystick input, delays, a PC speaker beep, audio, a file container,
and **ObjectCore**: objects with sprites, states and a small script language (Cscript) for games and complex menus.

**Documentation: [Documentation/README.md](Documentation/README.md)**

## Platforms

✅ supported, ❌ not supported.

| Platform | Graphics | Terminal | Keyboard | Joystick | Audio | PC speaker | Storage |
| --- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| Windows | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Linux | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Windows Phone | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Android | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| macOS | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| iOS | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| MS-DOS | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Web (Emscripten) | ✅ | ❌ | ✅ | ✅ | ❌ | ❌ | ✅ |

* Graphics, joystick (real controller), and audio files come from SDL2. MS-DOS has its own VGA mode (experimental),
  keyboard based joystick, and OPL2 / Sound Blaster audio.
* The PC speaker is emulated where there is no real one.

Details for each platform: [Documentation/general/platforms.md](Documentation/general/platforms.md).

## Showcase

Programs made with MT2D:

* Super Shibe 3000: https://www.youtube.com/watch?v=3MYdq7QXjFs
* Project Asteroid: https://www.youtube.com/watch?v=atRMo71Xgzk
* Cscript Test Demo: https://www.youtube.com/watch?v=mg7kxA6tLhQ
* PathFind Demo: https://www.youtube.com/watch?v=wm-WmNFX8Mg
* HexStartup++: https://www.youtube.com/watch?v=8WkwBKS80oQ
* 3D Maze: https://www.youtube.com/watch?v=JqT4374zego
* Pong (ms-dos): https://www.youtube.com/watch?v=ALux2S34D-s

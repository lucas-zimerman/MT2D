# Audio

Header: `MT2D/Audio/MT2D_Audio_core.h`. The names below are macros, the backend is chosen at compile time.

| Call | Meaning |
| --- | --- |
| `MT2D_Audio_Init()` | start the audio system |
| `MT2D_Audio_Close()` | stop it |
| `MT2D_Play_Audio(name)` | play a sound effect once (many at the same time) |
| `MT2D_Play_Music(name)` | play a song in a loop |
| `MT2D_PlayAudioFromMemory(audio, size, free)`, `MT2D_PlayMusicFromMemory(music, size, free)`, `MT2D_PlaySoundFromMemory(...)` | same from a memory block |

| Build | Backend |
| --- | --- |
| `SDL_USE_AUDIO` | SDL2_mixer (`.wav`, `.ogg`, ...) |
| DJGPP (MS-DOS) | music: IMF song on the OPL2 chip; effects: PCM mono `.wav` through the Sound Blaster. `MT2D_Audio_Init()` is non-zero when a chip/card was found |
| anything else | every call expands to nothing (a `#pragma message` warns). The program builds and stays silent |

## MS-DOS naming rules

`MT2D_Play_Music("song.ogg")` loops `d/song.imf` and `MT2D_Play_Audio("coin.ogg")` plays `d/coin.wav`: the
name is cut to 8 characters, the extension replaced and the `d/` folder added (a name that already ends
in `.imf` / `.wav` is used as is). Up to 3 effects at once, mixed in software at 22050 Hz 8 bit; songs up to
32 KB are loaded whole, bigger ones are streamed. Details: [`../examples/README.md`](../../examples/README.md#audio).

## PC speaker

`MT2D_System_Beep(frequency, milliseconds)` (header `MT2D_System_Calls.h`, see [tui/io.md](../tui/io.md)).
On modern Windows it is emulated, on Linux it tries the console speaker and then plays a generated tone with
`paplay` / `pw-play` / `aplay`. With SDL it goes to the sound card.

## SDL caveat

`SDL_SOUND_TYPE` (`MT2D/SDL/MT2D_SDL_Defines.h`): the SDL backend uses either the beeper or audio files,
whichever is requested first, not both in one run.

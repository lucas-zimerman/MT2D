/*===================================================================
        Audio_core.h Audio_core.h created by: Lucas Zimerman Fraulob
        
        Here's where you're going to play complex music files 
        (.wav,.mp3,...), you have two options of playing sounds.
        1) Music playing: the sound is always played into a loop.
        2) Audio playing: the sound is only played once.
        
        MT2D Start Audio System
                Description: Start the audio system
        MT2D Close Audio System
                Description: Closes the audio system
        MT2D Play Audio
                Description: Plays a sound file from a given addres
        MT2D Play Music
                Description: Plays a sound file in a loop.
        
===================================================================*/
#include <MT2D/MT2D_Terminal_Define.h>
#if defined(SDL_USE_AUDIO)
#include "../SDL/Audio/MT2D_SDL_Audio.h"
#define MT2D_Play_Audio SDL_Play_Sound
#define MT2D_PlayAudioFromMemory SDL_PlaySoundFromMemory
#define MT2D_Play_Music SDL_Play_Music
#define MT2D_PlayMusicFromMemory SDL_PlayMusicFromMemory
#define MT2D_Audio_Init() SDL_Start_Sound_System()
#define MT2D_Audio_Close SDL_Close_Sound_System
#elif defined(__DJGPP__)
/* MS-DOS: IMF songs on the OPL2 chip (Adlib / Sound Blaster), see _MSDOS/M2Music.h.
   MT2D_Play_Music("ode.ogg") plays d/ode.imf, like the images (png -> d/xxx.bmp). */
#include <MT2D/_MSDOS/M2Music.h>
#define MT2D_Audio_Init() MT2D_Dos_Music_Init()
#define MT2D_Audio_Close() MT2D_Dos_Music_Close()
#define MT2D_Play_Audio(name)
#define MT2D_Play_Music(name) MT2D_Dos_Music_PlayFile(name)
#define MT2D_PlayAudioFromMemory(audio, size, free)
#define MT2D_PlayMusicFromMemory(music, size, free) MT2D_Dos_Music_PlayMemory((music), (size), (free), MT2D_MUSIC_LOOP)
#define MT2D_PlaySoundFromMemory(memoryBlock, size, free)
#else
#ifndef MT2D_AUIDO_WARN
#define MT2D_AUIDO_WARN
//avoid the spam of this message
#pragma message ("Audio code not found, you can compile the project but you'll not hear any wave sound")
#endif
#define MT2D_Audio_Init()
#define MT2D_Audio_Close()
#define MT2D_Play_Audio(name)
#define MT2D_Play_Music(name)
#define MT2D_PlayAudioFromMemory(audio, size, free)
#define MT2D_PlayMusicFromMemory(music, size, free)
#define MT2D_PlaySoundFromMemory(memoryBlock, size, free)
#endif

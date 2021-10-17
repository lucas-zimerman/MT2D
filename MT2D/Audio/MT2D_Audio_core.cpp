#include <MT2D/MT2D_Terminal_Define.h>

#ifndef SDL_USE_AUDIO
void MT2D_Audio_Init() {}
void MT2D_Audio_Close() {}
void MT2D_Play_Audio(char *Name) {}
void MT2D_Play_Music(char *Name) {}
void SDL_PlayMusicFromMemory(void* music, unsigned int size, bool free) {}
void SDL_PlaySoundFromMemory(void* memoryBlock, unsigned int size, bool free) {}
#endif

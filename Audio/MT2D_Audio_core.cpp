#include "MT2D/MT2D_Terminal_Define.h"

#ifdef SDL_USE
#include "SDL_MT2D/SDL_MT2DAudio.h"
#endif
#ifdef _DEBUG
    #include <stdio.h>
#endif

void MT2D_Audio_Init(){
    #ifdef _DEBUG
        printf("MT2D: Starting audio system\n");
    #endif
    #ifdef SDL_USE
        //Start the SDL audio system
        SDL_Start_Sound_System();
    #elif defined WIN32
        //Start the Windows audio system
    #elif defined __MSDOS__
        //Start the sound card?
    #elif defined linux
        //Start the gnuc audio system
    #endif
}
void MT2D_Audio_Close(){
    #ifdef _DEBUG
        printf("MT2D: Closing audio system\n");
    #endif

    #ifdef SDL_USE
        //Close the SDL audio system
        SDL_Close_Sound_System();
    #elif defined WIN32
        //Close the Windows audio system
    #elif defined __MSDOS__
        //Mute the sound card?
    #elif defined linux
        //Close the gnuc audio system
    #endif
}
void MT2D_Play_Audio(char *Name){
    #ifdef _DEBUG
        printf("MT2D: Playing Sound %s\n",Name);
    #endif
    #ifdef SDL_USE
        SDL_Play_Sound(Name);
    #endif
}
void MT2D_Play_Music(char *Name){
    #ifdef _DEBUG
        printf("MT2D: Playing Music\n");
    #endif
    #ifdef SDL_USE
        SDL_Play_Music(Name);
    #endif
}

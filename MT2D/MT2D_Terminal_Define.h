#ifndef MTTerm_Def_H
#define MTTerm_Def_H
/*===================================================================
MT2D_Terminal_Define.h Created by: Lucas Zimerman Fraulob

        the purpose of this header is to identify if the developer wants
        to use the system terminal or an emulated terminal (Using SDL)

        #define SDL_USE, unmark it and MT2D will use the SDL2 for drawing
        and also sounding. (Still in beta)

        This is not the best place for doing that stuff, but it's what I
        have for now.

        #define SDL_stbimage, Used to replace SDL_IMAGE (only used at Universal Windows Platform apps or others that do not support SDL_IMAGE
        unmark this in case you're using SDL_IMAGE
        files required for SDL_stbimage:
        -https://github.com/DanielGibson/Snippets/blob/master/SDL_stbimage.h
        -https://github.com/nothings/stb

        #define MT2D_SDL_GPU, Used for replace SDL render calls by SDL_GPU
        -https://github.com/grimfang4/sdl-gpu
===================================================================*/

#pragma region MSDOS_USER_SETTINGS
/*Only enable MSDOS_USE_GRAPHICS only if you really need to render graphic sprites*/
/*By default MSDOS target doesn't need any define to be enabled*/
#define MSDOS_USE_GRAPHICS /** Set the render mode in VGA mode, with 256 colors and a resolution of 320x200 ,this has less resolution and is slower than the normal MSDOS mode so if you dont need to use hraphic sprites dont enable this*/
//#define MSDOS_USE_DOUBLE_BUFFER /** Only enable this if MSDOS_USE_GRAPHICS is enabled, this avoid flickering on screen at the cost of 64KB of ram*/
#define MSDOS_USE_SPRITE_TILE_RENDER /** Uses a special method of rendering sprites on screeen, works for normal and double buffer mode**/
#pragma endregion


//#define SDL_USE_AUDIO
//#define DEBUG_OBJECTCORE
#define MT2D_USING_CONTAINER
//#define MT2D_WINDOWED_MODE
//#define SDL_USE
//#define MT2D_SCREEN_RESIZE
//#define MT2D_SDL_GPU
//#define MT2D_SDL_USING_DIRECTX_RENDER

//#define SDL_stbimage //Used to replace SDL_IMAGE (only used at Universal Windows Platform apps or others that do not support SDL_IMAGE

#pragma endregion

#pragma region MT2D Settings

#define MT2D_USING_CONTAINER /** Enables the container system */

#pragma endregion

/****************************** The below defines are MT2D settings that you should not touch!!! ****************************************/


#if !defined(_WIN32) && !defined(__MSDOS__) && !defined(__DOS__) && !defined(SDL_USE) // If there`s no way of using some form of hardware aceleration for displaying the screen,
// tell it`s using an ancient way of displaying stuff...
   #define MT2D_RENDERING_PRINTF //we are forced to use printf :(
#endif

#if defined(_DEBUG) /*|| 1*/ /*uncomment that line too allow a manual debug*/
//#define MT2D_DEBUG_ENABLED
#endif

#pragma region Internal_Defines

#if (defined(WIN32) || defined(_WIN64)) && defined(_WIN32)
        #define WINDOWS_DESKTOP_TARGET
#elif !(!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
        #define WINDOWS_UWP_TARGET
#endif

#if defined(WIN32) || defined(_WIN32)
        #define WINDOWS_TARGET
#elif defined(__ANDROID__)
        #define ANDROID_TARGET
#elif defined(linux)
        #define LINUX_TARGET
#elif defined(__EMSCRIPTEN__)
        #define BROWSER_TARGET
#endif
#pragma endregion

#pragma region Portable_Inline
#ifdef _MSC_VER
#define forceinline __forceinline
#elif defined(__GNUC__)
#define forceinline __attribute__((always_inline)) inline
#else
#define forceinline inline
#endif
#pragma endregion

#pragma region Portable_Inline
#ifdef _MSC_VER
#define forceinline __forceinline
#elif defined(__GNUC__)
#define forceinline __attribute__((always_inline)) inline
#else
#define forceinline inline
#endif
#pragma endregion

#endif 

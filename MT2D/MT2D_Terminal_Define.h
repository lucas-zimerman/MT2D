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

#define SDL_USE_AUDIO
//#define DEBUG_OBJECTCORE
//#define MT2D_USING_CONTAINER
#define MT2D_WINDOWED_MODE
#define SDL_USE
//#define MT2D_SCREEN_RESIZE
//#define MT2D_SDL_GPU
//#define MT2D_SDL_USING_DIRECTX_RENDER

//#define SDL_stbimage //Used to replace SDL_IMAGE (only used at Universal Windows Platform apps or others that do not support SDL_IMAGE

#if !defined(_WIN32) && !defined(__MSDOS__) && !defined(SDL_USE) // If there`s no way of using some form of hardware aceleration for displaying the screen,
// tell it`s using an ancient way of displaying stuff...
   #define MT2D_RENDERING_PRINTF //we are forced to use printf :(
#endif

#if defined(_DEBUG) /*|| 1*/ /*uncomment that line too allow a manual debug*/
#define MT2D_DEBUG_ENABLED
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
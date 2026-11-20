#ifndef MTTerm_Def_H
#define MTTerm_Def_H

/* The engine uses the (GNU only) `linux` macro for "unix like". Define it ourselves so */
/* strict -std=c++XX builds and macOS work too. */
#if (defined(__linux__) || defined(__APPLE__)) && !defined(linux)
#define linux 1
#endif
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

/*Only enable MSDOS_USE_GRAPHICS only if you really need to render graphic sprites*/
/*By default MSDOS target doesn't need any define to be enabled*/
/*Opt-in: pass -DMSDOS_USE_GRAPHICS (or uncomment the line below) when building for MS-DOS.*/
/* #define MSDOS_USE_GRAPHICS / ** Set the render mode in VGA mode, with 256 colors and a resolution of 320x200 ,this has less resolution and is slower than the normal MSDOS mode so if you dont need to use hraphic sprites dont enable this* / */
/*MSDOS_USE_DOUBLE_BUFFER no longer works (defining it does nothing, remove it, see MIGRATION.md): the VGA renderer builds each 4x8 tile in a small buffer and writes it once, without flicker and without the 64KB buffer.*/
/*Optional: -DMSDOS_VGA_WAIT_RETRACE waits the vertical retrace before drawing a frame.*/


/* Linux (terminal mode): draw the screen as a Sixel image (graphics, bitmap font) instead of text. Needs a terminal */
/* with Sixel support (xterm -ti vt340, foot, mlterm, wezterm, ...). Graphics only: no sound. Image sprites load the 8 bit */
/* BMP files of the MS-DOS VGA mode ("name.png" -> "d/name.bmp"). */
/* MT2D_Init exits with an error (code 1) when the terminal does not report Sixel, MT2D_SIXEL=1 skips that check. */
/* Ignored with SDL_USE and on the other platforms. Pass -DMT2D_TERMINAL_USE_GRAPHICS or uncomment the line below. */
/* #define MT2D_TERMINAL_USE_GRAPHICS */

/* Linux (terminal mode): play sound effects and music (.wav, .ogg) with command line players (paplay, pw-play, aplay, */
/* ffplay), nothing is linked. No IMF music. Ignored with SDL_USE and on the other platforms. */
/* #define MT2D_TERMINAL_USE_AUDIO */

/* #define SDL_USE_AUDIO */
/* #define DEBUG_OBJECTCORE */
#define MT2D_USING_CONTAINER
/* #define MT2D_WINDOWED_MODE */
/* #define SDL_USE */
/* #define MT2D_SCREEN_RESIZE */
/* #define MT2D_SDL_GPU */
/* #define MT2D_SDL_USING_DIRECTX_RENDER */

/* #define SDL_stbimage //Used to replace SDL_IMAGE (only used at Universal Windows Platform apps or others that do not support SDL_IMAGE */



#define MT2D_USING_CONTAINER /** Enables the container system */


/****************************** The below defines are MT2D settings that you should not touch!!! ****************************************/


#if !defined(_WIN32) && !defined(__MSDOS__) && !defined(__DOS__) && !defined(SDL_USE) /* If there`s no way of using some form of hardware aceleration for displaying the screen, */
/* tell it`s using an ancient way of displaying stuff... */
   #define MT2D_RENDERING_PRINTF /* we are forced to use printf :( */
#endif

/* The Sixel renderer of the Linux terminal backend (internal, use MT2D_TERMINAL_USE_GRAPHICS) */
#if defined(MT2D_TERMINAL_USE_GRAPHICS) && defined(linux) && !defined(SDL_USE) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
   #define MT2D_LINUX_SIXEL
#endif

/* The command line player audio of the Linux terminal backend (internal, use MT2D_TERMINAL_USE_AUDIO) */
#if defined(MT2D_TERMINAL_USE_AUDIO) && defined(linux) && !defined(SDL_USE) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
   #define MT2D_LINUX_AUDIO
#endif

#if defined(_DEBUG) /*|| 1*/ /*uncomment that line too allow a manual debug*/
/* #define MT2D_DEBUG_ENABLED */
#endif


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

#ifdef _MSC_VER
#define forceinline __forceinline
#elif defined(__GNUC__)
#define forceinline __attribute__((always_inline)) inline
#else
#define forceinline inline
#endif

#ifdef _MSC_VER
#define forceinline __forceinline
#elif defined(__GNUC__)
#define forceinline __attribute__((always_inline)) inline
#else
#define forceinline inline
#endif

#endif 

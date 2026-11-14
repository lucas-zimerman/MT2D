#ifndef MT2D_SETTINGS_H
#define MT2D_SETTINGS_H
/*
        One shared way to configure the engine, the same call on every platform.

          MT2D_Set_Settings(MT2D_SDL_SET_WINDOWED, 800, 600);

        or, for the operations that take a list of integers (a sound card setup, a palette), or when
        the values come from an array (a script, a config file):

          int pal[1 + 256] = { MT2D_DOS_SET_PALETTE, 0x000000, 0x0000AA, ... };
          MT2D_Set_Settings_Argv(1 + 256, pal);

        Each operation below says which of the two calls handles it; the other call answers
        MT2D_UNSUPPORTED for it.

        Nothing is stored here: MT2D_Set_Settings and MT2D_Set_Settings_Argv are macros for the
        functions of the platform (MT2D_SDL_Set_Settings, MT2D_SDL_Set_Settings_Argv,
        MT2D_Win_..., MT2D_Linux_..., MT2D_Dos_..., MT2D_Dos_VGA_...), which apply them to their own
        state or answer MT2D_UNSUPPORTED.
        To add a setting add an entry to MT2D_Setting_Operation, document its params, and handle
        it in the backends that support it.
*/
#include <MT2D/MT2D_Terminal_Define.h> /* SDL_USE and the platform macros that pick the backend below */
#include <MT2D/ObjectCore/MT2D_CDecl.h>

/*
        Operations are grouped by platform, a backend answers MT2D_UNSUPPORTED to the operations of
        the other ranges:
          1xxx Windows (terminal)   2xxx Linux (terminal)   3xxx macOS (terminal)
          4xxx MS-DOS               5xxx SDL                6xxx shared (every platform)
        argv[0] is the operation, the params follow. A param that is missing or not used is ignored.
*/
typedef enum MT2D_Setting_Operation {
        /* 1xxx Windows, 2xxx Linux, 3xxx macOS: none yet */

        /* 4xxx MS-DOS */
        MT2D_DOS_SET_AUDIO   = 4000, /* MT2D_Set_Settings_Argv. Sound Blaster setup: { op, port, irq, dma, rate }. A param that is missing or negative (port, irq and
                                        rate also 0) is not overridden: the BLASTER variable gives port, irq and dma, MT2D_DOS_SOUND_RATE the rate.
                                        port 0x210-0x280, irq 3-15, dma 0, 1 or 3, rate 4000-44100 Hz. If the card is running it is restarted with
                                        the new setup, otherwise it is used by the next MT2D_Audio_Init(). MT2D_UNSUPPORTED without DJGPP. */
        MT2D_DOS_SET_PALETTE = 4001, /* MT2D_Set_Settings_Argv. VGA only: { op, 0xRRGGBB x 256 }, the whole palette (argc must be 257, else MT2D_BAD_PARAMETER).
                                        Index 0 is the transparent color of the sprites. */

        /* 5xxx SDL */
        MT2D_SDL_SET_WINDOW_SIZE = 5000, /* MT2D_Set_Settings. param1 = width, param2 = height (pixels) of the window to be created, before MT2D_Init() only.
                                            MT2D_OK only means the request was given to SDL, see the note below. */
        MT2D_SDL_SET_WINDOWED    = 5001, /* MT2D_Set_Settings. leave fullscreen. param1 = width, param2 = height, both 0 keeps the current size. Works before and after MT2D_Init().
                                            MT2D_OK only means the request was given to SDL, see the note below. */
        MT2D_SDL_SET_FULLSCREEN  = 5002, /* MT2D_Set_Settings. no params, always at the monitor resolution (borderless, the video mode is not changed). Works before and after MT2D_Init(). */
        MT2D_SDL_ALT_ENTER_TOGGLE_FULLSCREEN = 5003 /* MT2D_Set_Settings. param1: 1 = Alt+Enter switches between fullscreen and windowed (the key is not given to the game),
                                            0 = it is a normal key (default). Works before and after MT2D_Init(). */

        /* Note: SDL has no way to tell if the platform honors a window size or windowed request, so on some platforms
           (mobile, where the OS owns the window, or any window manager that refuses it) MT2D_SDL_SET_WINDOW_SIZE and
           MT2D_SDL_SET_WINDOWED can return MT2D_OK and the window stays as it is. Do not assume the size you asked for:
           read the real one from the window resize events (the engine already follows them). */

        /* 6xxx shared: none yet */
} MT2D_Setting_Operation;

/* Result of the settings calls. Negative = refused, 0 = invalid call, 1 = done. */
typedef enum MT2D_Result {
        MT2D_UNSUPPORTED      = -3, /* this build/backend has no support for that setting */
        MT2D_BAD_PARAMETER    = -2, /* param out of range for that operation */
        MT2D_BEFORE_INIT_ONLY = -1, /* can only be configured before MT2D_Init() is called */
        MT2D_INVALID          =  0, /* bad argc/argv, or the operation could not be done (e.g. no sound card) */
        MT2D_OK               =  1
} MT2D_Result;

/* Same platform order as MT2D_Init in MT2D.h. The platform headers need the types above.
   MT2D_SETTINGS_NO_PLATFORM (used by the tests) skips them, so the fallbacks of MT2D_Settings.c are used. */
#if defined(MT2D_SETTINGS_NO_PLATFORM)
        /* no backend */
#elif defined(SDL_USE)
        #include <MT2D/SDL/MT2D_SDL_main.h>
        #define MT2D_Set_Settings MT2D_SDL_Set_Settings
        #define MT2D_Set_Settings_Argv MT2D_SDL_Set_Settings_Argv
#elif defined(_WIN32)
        #include <MT2D/_WINDOWS/MT2D_Win_Init.h>
        #define MT2D_Set_Settings MT2D_Win_Set_Settings
        #define MT2D_Set_Settings_Argv MT2D_Win_Set_Settings_Argv
#elif defined(MSDOS_USE_GRAPHICS)
        #include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>
        #define MT2D_Set_Settings MT2D_Dos_VGA_Set_Settings
        #define MT2D_Set_Settings_Argv MT2D_Dos_VGA_Set_Settings_Argv
#elif defined(__MSDOS__) || defined(__DOS__)
        #include <MT2D/_MSDOS/MT2D_Dos_Init.h>
        #define MT2D_Set_Settings MT2D_Dos_Set_Settings
        #define MT2D_Set_Settings_Argv MT2D_Dos_Set_Settings_Argv
#elif defined(linux)
        #include <MT2D/_LINUX/MT2D_Linux_Init.h>
        #define MT2D_Set_Settings MT2D_Linux_Set_Settings
        #define MT2D_Set_Settings_Argv MT2D_Linux_Set_Settings_Argv
#endif

MT2D_C_BEGIN
/* Sets one setting that has up to two params. Nothing is changed unless MT2D_OK is returned.
   On the platforms above this is the platform function itself (macro), anywhere else it is a
   fallback in MT2D_Settings.c that returns MT2D_UNSUPPORTED. */
#ifndef MT2D_Set_Settings
MT2D_Result MT2D_Set_Settings(MT2D_Setting_Operation operation, int param1, int param2);
#endif

/* Sets one setting from a list: argv[0] = operation, argv[1..argc-1] = its params.
   argc < 1 or argv == NULL returns MT2D_INVALID. Same macro / fallback rule as above. */
#ifndef MT2D_Set_Settings_Argv
MT2D_Result MT2D_Set_Settings_Argv(int argc, const int *argv);
#endif
MT2D_C_END
#endif

# Build logic of the MT2D engine library, shared by the top level CMakeLists.txt (the "mt2d" target that games link)
# and by the examples (examples/cmake/MT2D.cmake).
#
#   include(cmake/MT2DEngine.cmake)
#   mt2d_engine_target(<target> <sdl|terminal>)
#
# <target> becomes a static library with the engine sources of the backend; the include directory, the backend
# defines and the libraries are PUBLIC, so linking <target> is enough.
#
# Backends:
#   * terminal -> the native text backend of the system: DJGPP = MS-DOS text video memory (or the VGA mode with
#                 -DMT2D_DOS_VGA=ON), Windows = console API, Linux / macOS = curses (ncursesw is required).
#   * sdl      -> a window that emulates the text screen, also draws images. Needs SDL2 + SDL2_image (and
#                 SDL2_mixer for audio files, when found). Not available with DJGPP. With Emscripten the SDL2
#                 ports are used and it is the only backend.
#
# Options (cache variables, same names as the -D flags of the examples):
#   MT2D_SDL_AUDIO          ON   use SDL2_mixer (when found) to play audio files (sdl backend)
#   MT2D_TERMINAL_GRAPHICS  OFF  draw the Linux terminal screen as a Sixel image
#   MT2D_TERMINAL_AUDIO     OFF  play .wav / .ogg with command line players on the Linux terminal
#   MT2D_DOS_VGA            OFF  MS-DOS VGA renderer instead of the text mode one (DJGPP only)
#   MT2D_CSCRIPT_PARSER     ON   Cscript / object text compiler (OFF by default for MS-DOS text mode)

# Variables first: they are directory scoped, every directory that includes this file needs them.
get_filename_component(MT2D_REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(MT2D_ENGINE_DIR "${MT2D_REPO_DIR}/MT2D")

if(EMSCRIPTEN)
    set(MT2D_WEB TRUE)
else()
    set(MT2D_WEB FALSE)
endif()

if(DJGPP)
    set(MT2D_DOS TRUE)
else()
    set(MT2D_DOS FALSE)
endif()

set(MT2D_COMMON_SOURCES
    MT2D.c
    MT2D_Layout.c
    MT2D_Settings.c
    Audio/MT2D_Audio_core.c
    Container/aes.c
    Container/MT2D_Container.c
    Cursor/Cursor.c
    InputBox/MT2D_InputBox_Integer.c
    InputBox/MT2D_InputBox_Password.c
    InputBox/MT2D_InputBox_String.c
    MessageBox/MT2D_MessageBox.c
    MessageBox/MT2D_MessageBox_With_Result.c
    ObjectCore/Cscript.c
    ObjectCore/MT2D_Text_Image.c
    ObjectCore/Object.c
    ObjectCore/ObjectGlobals.c
    ObjectCore/ObjectLoader.c
    ObjectCore/SpriteBlock.c
    ObjectCore/Sprites.c
    ObjectCore/State.c
    ObjectCore/STDContainer.c
    ObjectCore/STDFunctions.c
    ObjectCore/Vars.c
    Tools/UserInterface/MT2D_Menu.c
    Tools/Video/MT2D_Generic_Line.c
)

option(MT2D_SDL_AUDIO "Use SDL2_mixer (when found) to play audio files with the SDL backend" ON)

# Experimental VGA mode (mode 13h) for MS-DOS, see MT2D_Terminal_Define.h (MSDOS_USE_GRAPHICS).
option(MT2D_DOS_VGA "Build the MS-DOS VGA renderer instead of the text mode one (DJGPP only)" OFF)

# Sixel graphics for the Linux terminal backend, see MT2D_Terminal_Define.h (MT2D_TERMINAL_USE_GRAPHICS).
option(MT2D_TERMINAL_GRAPHICS "Draw the Linux terminal screen as a Sixel image (needs a Sixel terminal)" OFF)

# Sound for the Linux terminal backend (paplay / pw-play / aplay / ffplay, nothing is linked), see MT2D_Terminal_Define.h (MT2D_TERMINAL_USE_AUDIO).
option(MT2D_TERMINAL_AUDIO "Play .wav / .ogg with command line players on the Linux terminal backend" OFF)

# The object text compiler (ObjectCore/Parser) is optional: games can ship compiled
# objects (.mto, made by the cscriptc tool) and only need the small ObjectLoader. Off by default on DOS.
if(MT2D_DOS AND NOT MT2D_DOS_VGA)
    option(MT2D_CSCRIPT_PARSER "Build the Cscript/object text compiler" OFF)
else()
    option(MT2D_CSCRIPT_PARSER "Build the Cscript/object text compiler" ON)
endif()

# The engine is C (and some C++ in _MSDOS/ and _WINDOWS/), a project may only declare CXX.
enable_language(C)
enable_language(CXX)
include(CheckCXXCompilerFlag)

if(COMMAND mt2d_engine_target)
    return()
endif()

# --------------------------------------------------------------------------------------------
# SDL2 (only looked up when an SDL target is added)
# Sets MT2D_SDL_LIBS, MT2D_SDL_INCLUDES and MT2D_HAS_MIXER.
# --------------------------------------------------------------------------------------------
macro(mt2d_find_sdl)
    if(NOT DEFINED MT2D_SDL_FOUND)
        set(MT2D_SDL_LIBS "")
        set(MT2D_SDL_INCLUDES "")
        set(MT2D_HAS_MIXER FALSE)
        if(MT2D_WEB)
            # nothing to find: the Emscripten ports are enabled with compiler flags (see mt2d_engine_target)
            set(MT2D_SDL_FOUND TRUE)
            set(MT2D_HAS_MIXER TRUE)
        else()
        find_package(SDL2 CONFIG QUIET)
        find_package(SDL2_image CONFIG QUIET)
        find_package(SDL2_mixer CONFIG QUIET)
        if(TARGET SDL2::SDL2 AND TARGET SDL2_image::SDL2_image)
            list(APPEND MT2D_SDL_LIBS SDL2::SDL2 SDL2_image::SDL2_image)
            if(WIN32 AND TARGET SDL2::SDL2main)
                # MT2D.h asks for SDL2main.lib with a #pragma comment(lib), vcpkg keeps it in a
                # separate folder, linking the target by path makes the linker find it.
                list(APPEND MT2D_SDL_LIBS SDL2::SDL2main)
            endif()
            if(TARGET SDL2_mixer::SDL2_mixer)
                list(APPEND MT2D_SDL_LIBS SDL2_mixer::SDL2_mixer)
                set(MT2D_HAS_MIXER TRUE)
            endif()
        else()
            # No CMake config packages (some distros), fall back to pkg-config.
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
            pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
            list(APPEND MT2D_SDL_LIBS PkgConfig::SDL2 PkgConfig::SDL2_IMAGE)
            pkg_check_modules(SDL2_MIXER IMPORTED_TARGET SDL2_mixer)
            if(SDL2_MIXER_FOUND)
                list(APPEND MT2D_SDL_LIBS PkgConfig::SDL2_MIXER)
                set(MT2D_HAS_MIXER TRUE)
            endif()
        endif()
        # The engine includes <SDL2/SDL.h> on unix and <SDL.h> on Windows, make both work.
        foreach(lib ${MT2D_SDL_LIBS})
            get_target_property(dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
            if(dirs)
                foreach(d ${dirs})
                    list(APPEND MT2D_SDL_INCLUDES "${d}" "${d}/..")
                endforeach()
            endif()
        endforeach()
        if(MT2D_SDL_INCLUDES)
            list(REMOVE_DUPLICATES MT2D_SDL_INCLUDES)
        endif()
        set(MT2D_SDL_FOUND TRUE)
        endif()
        if(NOT MT2D_SDL_AUDIO)
            set(MT2D_HAS_MIXER FALSE)
        endif()
    endif()
endmacro()

# --------------------------------------------------------------------------------------------
# mt2d_engine_target(<target> <sdl|terminal>)
# --------------------------------------------------------------------------------------------
function(mt2d_engine_target target backend)
    if(NOT backend STREQUAL "sdl" AND NOT backend STREQUAL "terminal")
        message(FATAL_ERROR "mt2d_engine_target: unknown backend '${backend}' (sdl or terminal)")
    endif()
    if(backend STREQUAL "sdl" AND MT2D_DOS)
        message(FATAL_ERROR "MT2D: the SDL backend is not available with DJGPP (use the terminal backend, -DMT2D_DOS_VGA=ON for images)")
    endif()
    if(backend STREQUAL "terminal" AND MT2D_WEB)
        message(FATAL_ERROR "MT2D: Emscripten has no terminal backend, use the SDL backend")
    endif()
    if(backend STREQUAL "sdl")
        mt2d_find_sdl()
    endif()

    set(sources ${MT2D_COMMON_SOURCES})
    if(backend STREQUAL "sdl")
        list(APPEND sources
            Joystick/MT2D_Joystick.c
            SDL/IO/SDL_MT2D_Joystick.c
            SDL/MT2D_SDL_Debug.c
            SDL/MT2D_SDL_Defines.c
            SDL/MT2D_SDL_Event_Handler.c
            SDL/MT2D_SDL_Frame.c
            SDL/MT2D_SDL_main.c
            SDL/MT2D_SDL_Redefine.c
            SDL/MT2D_SDL_System_Calls.c
            SDL/Audio/MT2D_SDL_Beep.c
            SDL/IO/SDL_MT2D_FILE.c
            SDL/IO/SDL_MT2D_Keyboard.c
            SDL/Render/MT2D_SDL_Render.c
            Touch/MT2D_Touch.c
        )
        if(MT2D_HAS_MIXER)
            list(APPEND sources SDL/Audio/MT2D_SDL_Audio.c)
        endif()
        if(MT2D_WEB)
            list(APPEND sources _EMNISCRIPT/IO/MT2D_Emni_Joystick.c)
        endif()
    elseif(MT2D_DOS)
        list(APPEND sources
            _MSDOS/M2Cursor.c
            _MSDOS/M2Frame.c
            _MSDOS/M2Init.c
            _MSDOS/M2SysCal.c
            _MSDOS/IO/M2Keybd.c
            # IMF music on the OPL2 chip (MT2D_Play_Music), see _MSDOS/M2Music.h
            Audio/MT2D_Music.c
            _MSDOS/M2Opl.c
            _MSDOS/M2Music.c
            # sound effects (MT2D_Play_Audio): software mixer + Sound Blaster DMA, see _MSDOS/M2Sound.h
            Audio/MT2D_Mixer.c
            Audio/MT2D_Wav.c
            _MSDOS/M2Sound.c
        )
        if(MT2D_DOS_VGA)
            list(APPEND sources Joystick/MT2D_Joystick.c _MSDOS/IO/MT2D_Dos_Joystick.c)
            # VGA mode 13h (320x200, 256 colors): text drawn with a bitmap font, BMP sprites on top
            list(APPEND sources
                _MSDOS/Render/DtxtRend.cpp
                _MSDOS/Render/dVgaRen.cpp
                _MSDOS/Render/MT2D_VGA_Init.cpp
                _MSDOS/Files/bmp.cpp
            )
        else()
            list(APPEND sources _MSDOS/Render/M2Render.c)
        endif()
    elseif(WIN32)
        list(APPEND sources
            _WINDOWS/MT2D_Win_Cursor.c
            _WINDOWS/MT2D_Win_Debug.c
            _WINDOWS/MT2D_Win_Init.c
            _WINDOWS/MT2D_Win_System_Calls.c
            _WINDOWS/IO/MT2D_Win_Keyboard.c
            _WINDOWS/Render/MT2D_Win_Render.c
        )
    else()
        list(APPEND sources
            _LINUX/MT2D_Linux_CP850.c
            _LINUX/MT2D_Linux_Cursor.c
            _LINUX/MT2D_Linux_Init.c
            _LINUX/MT2D_Linux_System_Calls.c
            _LINUX/Render/MT2D_Linux_Render.c
            _LINUX/Render/MT2D_Linux_Sixel.c
            _LINUX/Render/MT2D_Linux_Image.c
            _LINUX/Audio/MT2D_Linux_Audio.c
            _LINUX/IO/MT2D_Linux_Keyboard.c
        )
    endif()
    if(MT2D_CSCRIPT_PARSER)
        list(APPEND sources ObjectCore/Parser/CscriptCompiler.c)
    endif()
    list(TRANSFORM sources PREPEND "${MT2D_ENGINE_DIR}/")

    add_library(${target} STATIC ${sources})
    if(MT2D_CSCRIPT_PARSER)
        target_compile_definitions(${target} PUBLIC MT2D_USING_CSCRIPT_PARSER)
    endif()
    # "${MT2D_REPO_DIR}" makes <MT2D/...> includes work.
    target_include_directories(${target} PUBLIC "${MT2D_REPO_DIR}")
    set_target_properties(${target} PROPERTIES
        C_STANDARD 99
        C_EXTENSIONS ON
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
    )
    if(backend STREQUAL "sdl")
        target_include_directories(${target} PUBLIC ${MT2D_SDL_INCLUDES})
        target_compile_definitions(${target} PUBLIC SDL_USE)
        if(MT2D_HAS_MIXER)
            target_compile_definitions(${target} PUBLIC SDL_USE_AUDIO)
        endif()
        target_link_libraries(${target} PUBLIC ${MT2D_SDL_LIBS})
        if(MT2D_WEB)
            set(web_ports -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 "-sSDL2_IMAGE_FORMATS=[\"png\"]")
            if(MT2D_HAS_MIXER)
                list(APPEND web_ports -sUSE_SDL_MIXER=2 "-sSDL2_MIXER_FORMATS=[\"ogg\",\"wav\"]")
            endif()
            target_compile_options(${target} PUBLIC ${web_ports})
            target_link_options(${target} PUBLIC ${web_ports})
        endif()
    elseif(MT2D_DOS)
        if(MT2D_DOS_VGA)
            target_compile_definitions(${target} PUBLIC MSDOS_USE_GRAPHICS)
        endif()
    elseif(NOT WIN32)
        # wide character curses (ncursesw) is needed to draw the CP850 characters as Unicode.
        # macOS has no separate ncursesw: its curses already has the wide character functions.
        if(NOT APPLE)
            set(CURSES_NEED_WIDE TRUE)
        endif()
        if(MT2D_TERMINAL_GRAPHICS)
            target_compile_definitions(${target} PUBLIC MT2D_TERMINAL_USE_GRAPHICS)
        endif()
        if(MT2D_TERMINAL_AUDIO)
            target_compile_definitions(${target} PUBLIC MT2D_TERMINAL_USE_AUDIO)
        endif()
        find_package(Curses REQUIRED)
        target_include_directories(${target} PUBLIC ${CURSES_INCLUDE_DIRS})
        target_link_libraries(${target} PUBLIC ${CURSES_LIBRARIES})
        if(NOT APPLE)
            target_link_libraries(${target} PUBLIC m)
        endif()
    endif()
    # The engine is old code: keep the build output readable.
    if(MSVC)
        target_compile_options(${target} PRIVATE /w)
        target_compile_definitions(${target} PUBLIC _CRT_SECURE_NO_WARNINGS)
    else()
        check_cxx_compiler_flag(-fpermissive MT2D_HAS_FPERMISSIVE)
        target_compile_options(${target} PRIVATE -w)
        if(MT2D_HAS_FPERMISSIVE)
            # only the C++ sources left (_MSDOS, Windows) need it, gcc warns about it for C
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)
        endif()
    endif()
endfunction()

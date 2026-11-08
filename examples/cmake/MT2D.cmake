# Shared build logic for the MT2D examples.
#
# Every example folder includes this file, so each one can be configured on its own:
#
#   cmake -S examples/helloworld -B examples/helloworld/build
#   cmake --build examples/helloworld/build
#   ./examples/helloworld/bin/helloworld
#
# Backends (an example picks one with mt2d_add_example(... SDL) or without it):
#   * terminal (default) -> the engine's native text backend:
#                           DJGPP = MS-DOS text video memory, Windows = console API,
#                           Linux / macOS = curses (ncursesw is required).
#   * SDL                -> a window that emulates the text screen, also draws images.
#                           Needs SDL2 + SDL2_image (and optionally SDL2_mixer for audio files).
#                           Not available with DJGPP.
#   * WEB                -> Emscripten (build with emcmake, see web/README.md). SDL2 and SDL2_image come from
#                           the Emscripten ports, the output is an .html page. Only WEB examples are built there.
#
# Use -DMT2D_EXAMPLES_AUDIO=OFF to ignore SDL2_mixer even when it is installed.

if(COMMAND mt2d_add_example)
    return()
endif()

get_filename_component(MT2D_EXAMPLES_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
get_filename_component(MT2D_REPO_DIR "${MT2D_EXAMPLES_DIR}/.." ABSOLUTE)
set(MT2D_ENGINE_DIR "${MT2D_REPO_DIR}/MT2D")

option(MT2D_EXAMPLES_AUDIO "Use SDL2_mixer (when found) to play audio files" ON)

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

# Experimental VGA mode (mode 13h) for MS-DOS, see MT2D_Terminal_Define.h (MSDOS_USE_GRAPHICS).
option(MT2D_DOS_VGA "Build the MS-DOS VGA renderer instead of the text mode one (DJGPP only)" OFF)
if(MT2D_DOS AND MT2D_DOS_VGA)
    add_definitions(-DMSDOS_USE_GRAPHICS)
endif()

# The object text compiler (ObjectCore/Parser) is optional: games can ship compiled
# objects (.mto, made by the cscriptc tool) and only need the small ObjectLoader. Off by default on DOS.
if(MT2D_DOS AND NOT MT2D_DOS_VGA)
    option(MT2D_CSCRIPT_PARSER "Build the Cscript/object text compiler" OFF)
else()
    option(MT2D_CSCRIPT_PARSER "Build the Cscript/object text compiler" ON)
endif()

# --------------------------------------------------------------------------------------------
# SDL2 (only looked up when an SDL example is added)
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
        if(NOT MT2D_EXAMPLES_AUDIO)
            set(MT2D_HAS_MIXER FALSE)
        endif()
    endif()
endmacro()

# --------------------------------------------------------------------------------------------
# Engine libraries: mt2d (terminal) and mt2d_sdl
# --------------------------------------------------------------------------------------------
set(MT2D_COMMON_SOURCES
    MT2D.c
    MT2D_Layout.c
    Audio/MT2D_Audio_core.c
    Container/aes.c
    Container/MT2D_Container.c
    Cursor/Cursor.c
    InputBox/MT2D_InputBox_Integer.c
    InputBox/MT2D_InputBox_Password.c
    InputBox/MT2D_InputBox_String.c
    MessageBox/MT2D_MessageBox.c
    MessageBox/MT2D_MessageBox_With_Result.c
    ObjectCore/Cscript.cpp
    ObjectCore/MT2D_Text_Image.cpp
    ObjectCore/Object.cpp
    ObjectCore/ObjectGlobals.c
    ObjectCore/ObjectLoader.c
    ObjectCore/SpriteBlock.cpp
    ObjectCore/Sprites.cpp
    ObjectCore/State.cpp
    ObjectCore/STDContainer.cpp
    ObjectCore/STDFunctions.cpp
    ObjectCore/Vars.cpp
    Tools/UserInterface/MT2D_Menu.c
    Tools/Video/MT2D_Generic_Line.c
)

# New engine code is C (ObjectLoader, ObjectGlobals), the example projects only declare CXX.
enable_language(C)
include(CheckCXXCompilerFlag)

function(mt2d_engine_target target backend)
    set(sources ${MT2D_COMMON_SOURCES})
    if(backend STREQUAL "sdl")
        list(APPEND sources
            Joystick/MT2D_Joystick.c
            SDL/IO/SDL_MT2D_Joystick.c
            SDL/MT2D_SDL_Debug.c
            SDL/MT2D_SDL_Defines.c
            SDL/MT2D_SDL_Event_Handler.c
            SDL/MT2D_SDL_main.c
            SDL/MT2D_SDL_Redefine.c
            SDL/MT2D_SDL_System_Calls.c
            SDL/Audio/MT2D_SDL_Beep.c
            SDL/IO/SDL_MT2D_FILE.c
            SDL/IO/SDL_MT2D_Keyboard.c
            SDL/Render/MT2D_SDL_Render.c
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
    elseif(NOT MT2D_DOS AND NOT WIN32)
        # wide character curses (ncursesw) is needed to draw the CP850 characters as Unicode.
        # macOS has no separate ncursesw: its curses already has the wide character functions.
        if(NOT APPLE)
            set(CURSES_NEED_WIDE TRUE)
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
            # only the C++ sources left (ObjectCore) need it, gcc warns about it for C
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)
        endif()
    endif()
endfunction()

# --------------------------------------------------------------------------------------------
# mt2d_add_example(<name> SOURCES src/a.cpp ... [SDL] [WEB [PRELOAD file@/name ...]] [DOS_VGA] [DOS_MUSIC] [ASSETS file ...] [DOS_NAME short])
#
#   * WEB: an example made for the browser. It is only built with Emscripten (it uses SDL, plus SHELL, the
#     page common/web_shell.html) and the other examples are skipped there. The output is <example folder>/bin/index.html
#     (+ .js, .wasm, .data). The SDL font is preloaded into the virtual file system, PRELOAD adds more files
#     (images, audio) as "<file on disk>@/<name in the browser>": the engine finds them by that name, like in the bin folder.
#     SDL2_mixer is linked too (audio: MT2D_Play_Audio / MT2D_Play_Music, and the PC speaker emulation MT2D_System_Beep).
#   * the executable goes to <example folder>/bin
#   * SDL examples also get the SDL font/icon in bin, ASSETS (paths relative to the example
#     folder) are copied to bin too
#   * DOS_NAME is the executable name used with DJGPP (8.3 file names)
#   * SDL examples are skipped (with a message) when building with DJGPP, unless they also pass DOS_VGA
#     and the build uses -DMT2D_DOS_VGA=ON (the VGA renderer then draws their images, see tools/png2dos.py)
#   * DOS_MUSIC: an SDL example that only needs audio files also builds with DJGPP (terminal backend): the engine plays
#     the IMF songs of its "d" folder on the OPL2 chip, in place of the .ogg files (ode.ogg -> d/ode.imf), and the
#     .wav sound effects of its "d" folder on the Sound Blaster, in place of the .wav/.ogg files (jump.ogg -> d/jump.wav)
# --------------------------------------------------------------------------------------------
function(mt2d_add_example name)
    cmake_parse_arguments(EX "SDL;WEB;DOS_VGA;DOS_MUSIC" "DOS_NAME" "SOURCES;ASSETS;PRELOAD" ${ARGN})

    if(EX_WEB AND NOT MT2D_WEB)
        message(STATUS "MT2D example '${name}' is web only (use emcmake) and is skipped")
        return()
    endif()
    if(MT2D_WEB AND NOT EX_WEB)
        message(STATUS "MT2D example '${name}' does not run in the browser and is skipped")
        return()
    endif()
    if(EX_WEB)
        set(EX_SDL TRUE)
    endif()

    if(EX_SDL AND MT2D_DOS)
        if(MT2D_DOS_VGA AND EX_DOS_VGA)
            # The VGA renderer draws the image sprites too: build it with the terminal backend and
            # the converted images of the "d" folder (see tools/png2dos.py).
            set(EX_SDL FALSE)
            set(EX_DOS_IMAGES TRUE)
        elseif(EX_DOS_MUSIC)
            set(EX_SDL FALSE)
            set(EX_DOS_IMAGES TRUE) # copies the "d" folder (the IMF songs), the assets are the .ogg files
            set(EX_DOS_AUDIO TRUE)
        else()
            message(STATUS "MT2D example '${name}' needs SDL and is skipped for DJGPP")
            return()
        endif()
    endif()

    if(EX_SDL)
        mt2d_find_sdl()
        set(lib mt2d_sdl)
        if(NOT TARGET mt2d_sdl)
            mt2d_engine_target(mt2d_sdl sdl)
        endif()
    else()
        set(lib mt2d)
        if(NOT TARGET mt2d)
            mt2d_engine_target(mt2d terminal)
        endif()
    endif()

    add_executable(${name} ${EX_SOURCES})
    target_link_libraries(${name} PRIVATE ${lib})
    target_include_directories(${name} PRIVATE "${MT2D_EXAMPLES_DIR}/common")
    set_target_properties(${name} PROPERTIES
        C_STANDARD 99
        C_EXTENSIONS ON
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
    )

    set(bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${bin_dir}")
    # Multi-config generators (Visual Studio, Xcode) append a config folder otherwise.
    foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${cfg} "${bin_dir}")
    endforeach()

    if(EX_WEB)
        set_target_properties(${name} PROPERTIES OUTPUT_NAME index SUFFIX ".html")
        set(web_shell "${MT2D_EXAMPLES_DIR}/common/web_shell.html")
        set_target_properties(${name} PROPERTIES LINK_DEPENDS "${web_shell}")
        target_link_options(${name} PRIVATE
            --shell-file "${web_shell}"
            "SHELL:--preload-file ${MT2D_EXAMPLES_DIR}/assets/msdos_fnt.png@assets/msdos_fnt.png"
            -sALLOW_MEMORY_GROWTH=1)
        foreach(p ${EX_PRELOAD})
            target_link_options(${name} PRIVATE "SHELL:--preload-file ${p}")
        endforeach()
    endif()

    if(MT2D_DOS AND EX_DOS_NAME)
        set_target_properties(${name} PROPERTIES OUTPUT_NAME "${EX_DOS_NAME}")
    endif()

    if((EX_SDL AND MT2D_HAS_MIXER) OR EX_DOS_AUDIO)
        target_compile_definitions(${name} PRIVATE MT2D_EXAMPLE_AUDIO_FILES=1)
    endif()

    if(MSVC)
        target_compile_options(${name} PRIVATE /W3)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra)
    endif()

    set(files "")
    if(EX_SDL AND NOT EX_WEB)
        # SDL needs the font (and the icon) next to the executable.
        list(APPEND files "${MT2D_EXAMPLES_DIR}/assets/msdos_fnt.png" "${MT2D_EXAMPLES_DIR}/assets/msdos.ico")
    endif()
    foreach(a ${EX_ASSETS})
        if(EX_DOS_IMAGES AND NOT a MATCHES "\\.txt$")
            continue() # MS-DOS: only the scripts, the images come from the "d" folder
        endif()
        list(APPEND files "${CMAKE_CURRENT_SOURCE_DIR}/${a}")
    endforeach()
    if(EX_DOS_IMAGES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/d")
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/d" "${bin_dir}/d")
    endif()
    foreach(f ${files})
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${f}" "${bin_dir}/")
    endforeach()
endfunction()

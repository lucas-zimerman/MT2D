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
#
# Use -DMT2D_EXAMPLES_AUDIO=OFF to ignore SDL2_mixer even when it is installed.

if(COMMAND mt2d_add_example)
    return()
endif()

get_filename_component(MT2D_EXAMPLES_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
get_filename_component(MT2D_REPO_DIR "${MT2D_EXAMPLES_DIR}/.." ABSOLUTE)
set(MT2D_ENGINE_DIR "${MT2D_REPO_DIR}/MT2D")

option(MT2D_EXAMPLES_AUDIO "Use SDL2_mixer (when found) to play audio files" ON)

if(DJGPP)
    set(MT2D_DOS TRUE)
else()
    set(MT2D_DOS FALSE)
endif()

# The object text compiler (ObjectCore/Parser) is optional: games can ship compiled
# objects (.mto, made by the cscriptc tool) and only need the small ObjectLoader. Off by default on DOS.
if(MT2D_DOS)
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
        if(NOT MT2D_EXAMPLES_AUDIO)
            set(MT2D_HAS_MIXER FALSE)
        endif()
        set(MT2D_SDL_FOUND TRUE)
    endif()
endmacro()

# --------------------------------------------------------------------------------------------
# Engine libraries: mt2d (terminal) and mt2d_sdl
# --------------------------------------------------------------------------------------------
set(MT2D_COMMON_SOURCES
    MT2D.cpp
    MT2D_Layout.cpp
    Audio/MT2D_Audio_core.cpp
    Container/aes.cpp
    Container/MT2D_Container.cpp
    Cursor/Cursor.cpp
    InputBox/MT2D_InputBox_Integer.cpp
    InputBox/MT2D_InputBox_Password.cpp
    InputBox/MT2D_InputBox_String.cpp
    MessageBox/MT2D_MessageBox.cpp
    MessageBox/MT2D_MessageBox_With_Result.cpp
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
    Tools/UserInterface/MT2D_Menu.cpp
    Tools/Video/MT2D_Generic_Line.cpp
)

# New engine code is C (ObjectLoader, ObjectGlobals), the example projects only declare CXX.
enable_language(C)
include(CheckCXXCompilerFlag)

function(mt2d_engine_target target backend)
    set(sources ${MT2D_COMMON_SOURCES})
    if(backend STREQUAL "sdl")
        list(APPEND sources
            Joystick/MT2D_Joystick.cpp
            SDL/IO/SDL_MT2D_Joystick.cpp
            SDL/MT2D_SDL_Debug.cpp
            SDL/MT2D_SDL_Defines.cpp
            SDL/MT2D_SDL_Event_Handler.cpp
            SDL/MT2D_SDL_main.cpp
            SDL/MT2D_SDL_Redefine.cpp
            SDL/MT2D_SDL_System_Calls.cpp
            SDL/Audio/MT2D_SDL_Beep.cpp
            SDL/IO/SDL_MT2D_FILE.cpp
            SDL/IO/SDL_MT2D_Keyboard.cpp
            SDL/Render/MT2D_SDL_Render.cpp
        )
        if(MT2D_HAS_MIXER)
            list(APPEND sources SDL/Audio/MT2D_SDL_Audio.cpp)
        endif()
    elseif(MT2D_DOS)
        list(APPEND sources
            _MSDOS/M2Cursor.cpp
            _MSDOS/M2Init.cpp
            _MSDOS/M2SysCal.cpp
            _MSDOS/Render/M2Render.cpp
            _MSDOS/IO/M2Keybd.cpp
        )
    elseif(WIN32)
        list(APPEND sources
            _WINDOWS/MT2D_Win_Cursor.cpp
            _WINDOWS/MT2D_Win_Debug.cpp
            _WINDOWS/MT2D_Win_Init.cpp
            _WINDOWS/MT2D_Win_System_Calls.cpp
            _WINDOWS/IO/MT2D_Win_Keyboard.cpp
            _WINDOWS/Render/MT2D_Win_Render.cpp
        )
    else()
        list(APPEND sources
            _LINUX/MT2D_Linux_CP850.cpp
            _LINUX/MT2D_Linux_Cursor.cpp
            _LINUX/MT2D_Linux_Init.cpp
            _LINUX/MT2D_Linux_System_Calls.cpp
            _LINUX/Render/MT2D_Linux_Render.cpp
            _LINUX/IO/MT2D_Linux_Keyboard.cpp
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
            target_compile_options(${target} PRIVATE -fpermissive)
        endif()
    endif()
endfunction()

# --------------------------------------------------------------------------------------------
# mt2d_add_example(<name> SOURCES src/a.cpp ... [SDL] [ASSETS file ...] [DOS_NAME short])
#
#   * the executable goes to <example folder>/bin
#   * SDL examples also get the SDL font/icon in bin, ASSETS (paths relative to the example
#     folder) are copied to bin too
#   * DOS_NAME is the executable name used with DJGPP (8.3 file names)
#   * SDL examples are skipped (with a message) when building with DJGPP
# --------------------------------------------------------------------------------------------
function(mt2d_add_example name)
    cmake_parse_arguments(EX "SDL" "DOS_NAME" "SOURCES;ASSETS" ${ARGN})

    if(EX_SDL)
        if(MT2D_DOS)
            message(STATUS "MT2D example '${name}' needs SDL and is skipped for DJGPP")
            return()
        endif()
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

    if(MT2D_DOS AND EX_DOS_NAME)
        set_target_properties(${name} PROPERTIES OUTPUT_NAME "${EX_DOS_NAME}")
    endif()

    if(EX_SDL AND MT2D_HAS_MIXER)
        target_compile_definitions(${name} PRIVATE MT2D_EXAMPLE_AUDIO_FILES=1)
    endif()

    if(MSVC)
        target_compile_options(${name} PRIVATE /W3)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra)
    endif()

    set(files "")
    if(EX_SDL)
        # SDL needs the font (and the icon) next to the executable.
        list(APPEND files "${MT2D_EXAMPLES_DIR}/assets/msdos_fnt.png" "${MT2D_EXAMPLES_DIR}/assets/msdos.ico")
    endif()
    foreach(a ${EX_ASSETS})
        list(APPEND files "${CMAKE_CURRENT_SOURCE_DIR}/${a}")
    endforeach()
    foreach(f ${files})
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${f}" "${bin_dir}/")
    endforeach()
endfunction()

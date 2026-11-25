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
#                           -DMT2D_TERMINAL_GRAPHICS=ON draws it as a Sixel image instead (Linux, Sixel terminal).
#   * SDL                -> a window that emulates the text screen, also draws images.
#                           Needs SDL2 + SDL2_image (and optionally SDL2_mixer for audio files).
#                           Not available with DJGPP.
#   * WEB                -> Emscripten (build with emcmake, see web/README.md). SDL2 and SDL2_image come from
#                           the Emscripten ports, the output is an .html page. Only WEB examples are built there.
#
# Use -DMT2D_EXAMPLES_AUDIO=OFF (or -DMT2D_SDL_AUDIO=OFF) to ignore SDL2_mixer even when it is installed.
#
# The engine libraries (mt2d = terminal, mt2d_sdl = SDL) come from cmake/MT2DEngine.cmake, the same code as the
# "mt2d" target of the top level CMakeLists.txt that games link.

get_filename_component(MT2D_EXAMPLES_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${MT2D_EXAMPLES_DIR}/../cmake/MT2DEngine.cmake")

# Older name of MT2D_SDL_AUDIO, still accepted.
option(MT2D_EXAMPLES_AUDIO "Use SDL2_mixer (when found) to play audio files" ON)
if(NOT MT2D_EXAMPLES_AUDIO)
    set(MT2D_SDL_AUDIO OFF)
endif()

if(COMMAND mt2d_add_example)
    return()
endif()

# --------------------------------------------------------------------------------------------
# mt2d_add_example(<name> SOURCES src/a.cpp ... [SDL] [WEB [PRELOAD file@/name ...] [SHELL page.html]] [DOS_VGA] [TERMINAL_GRAPHICS] [DOS_MUSIC] [DOS_ONLY [NEEDS_VGA]] [ASSETS file ...] [DOS_NAME short])
#
#   * DOS_ONLY: an example for MS-DOS only (examples/dos), skipped with every other target. It uses the terminal
#     backend and its "d" folder is copied next to the executable. NEEDS_VGA also skips it unless -DMT2D_DOS_VGA=ON.
#   * WEB: an example made for the browser. It is only built with Emscripten (it uses SDL, plus SHELL, the
#     page common/web_shell.html) and the other examples are skipped there. The output is <example folder>/bin/index.html
#     (+ .js, .wasm, .data). The SDL font is preloaded into the virtual file system, PRELOAD adds more files
#     (images, audio) as "<file on disk>@/<name in the browser>": the engine finds them by that name, like in the bin folder.
#     SHELL replaces the page (a path relative to the example folder, it must contain {{{ SCRIPT }}}).
#     SDL2_mixer is linked too (audio: MT2D_Play_Audio / MT2D_Play_Music, and the PC speaker emulation MT2D_System_Beep).
#   * the executable goes to <example folder>/bin
#   * SDL examples also get the SDL font/icon in bin, ASSETS (paths relative to the example
#     folder) are copied to bin too
#   * TERMINAL_GRAPHICS: an SDL example that also runs on the Linux Sixel graphics terminal (-DMT2D_TERMINAL_GRAPHICS=ON): it is
#     built with the terminal backend and the "d" folder next to the executable (no joystick on that backend, no sound)
#   * DOS_NAME is the executable name used with DJGPP (8.3 file names)
#   * SDL examples are skipped (with a message) when building with DJGPP, unless they also pass DOS_VGA
#     and the build uses -DMT2D_DOS_VGA=ON (the VGA renderer then draws their images, see tools/png2dos.py)
#   * DOS_MUSIC: an SDL example that only needs audio files also builds with DJGPP (terminal backend): the engine plays
#     the IMF songs of its "d" folder on the OPL2 chip, in place of the .ogg files (ode.ogg -> d/ode.imf), and the
#     .wav sound effects of its "d" folder on the Sound Blaster, in place of the .wav/.ogg files (jump.ogg -> d/jump.wav)
# --------------------------------------------------------------------------------------------
function(mt2d_add_example name)
    cmake_parse_arguments(EX "SDL;WEB;DOS_VGA;DOS_MUSIC;DOS_ONLY;NEEDS_VGA;TERMINAL_GRAPHICS" "DOS_NAME;SHELL" "SOURCES;ASSETS;PRELOAD" ${ARGN})

    if(EX_DOS_ONLY)
        if(NOT MT2D_DOS)
            message(STATUS "MT2D example '${name}' is MS-DOS only (DJGPP) and is skipped")
            return()
        endif()
        if(EX_NEEDS_VGA AND NOT MT2D_DOS_VGA)
            message(STATUS "MT2D example '${name}' needs the VGA mode (-DMT2D_DOS_VGA=ON) and is skipped")
            return()
        endif()
        set(EX_DOS_IMAGES TRUE) # copies its "d" folder (sounds, images) next to the executable
    endif()
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

    if(EX_SDL AND EX_TERMINAL_GRAPHICS AND MT2D_TERMINAL_GRAPHICS AND NOT MT2D_DOS AND NOT MT2D_WEB AND NOT WIN32)
        # Linux Sixel graphics (-DMT2D_TERMINAL_GRAPHICS=ON): the terminal backend draws the images from
        # the converted BMPs of the "d" folder, like the MS-DOS VGA mode.
        set(EX_SDL FALSE)
        set(EX_DOS_IMAGES TRUE)
        set(EX_TERMINAL_GFX TRUE)
    endif()

    if(EX_SDL)
        mt2d_find_sdl() # for MT2D_HAS_MIXER
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
        if(EX_SHELL)
            set(web_shell "${CMAKE_CURRENT_SOURCE_DIR}/${EX_SHELL}")
        endif()
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
        if(EX_TERMINAL_GFX AND MT2D_TERMINAL_AUDIO AND a MATCHES "\\.(wav|ogg)$")
            list(APPEND files "${CMAKE_CURRENT_SOURCE_DIR}/${a}") # played by the command line players, from the executable folder
            continue()
        endif()
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

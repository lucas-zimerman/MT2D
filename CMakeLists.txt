cmake_minimum_required(VERSION 3.12)
project(MT2D C CXX)

# The MT2D engine as a static library, for games:
#
#   add_subdirectory(MT2D)                      # or FetchContent_Declare(MT2D ...) + FetchContent_MakeAvailable(MT2D)
#   target_link_libraries(mygame PRIVATE mt2d)  # (or mt2d::mt2d)
#
# The include directory (<MT2D/MT2D.h>), the backend defines and libraries are PUBLIC.
#
#   -DMT2D_SDL=ON                SDL2 window instead of the native terminal (needs SDL2 + SDL2_image)
#   -DMT2D_SDL_AUDIO=OFF         with SDL: do not use SDL2_mixer even when it is installed
#   -DMT2D_TERMINAL_GRAPHICS=ON  Linux terminal: draw the screen as a Sixel image
#   -DMT2D_TERMINAL_AUDIO=ON     Linux terminal: play .wav / .ogg with command line players
#   -DMT2D_DOS_VGA=ON            DJGPP (examples/cmake/djgpp.cmake): VGA renderer instead of text mode
#   -DMT2D_CSCRIPT_PARSER=OFF    leave out the Cscript / object text compiler
#
# Emscripten (emcmake) always uses SDL. See Documentation/general/platforms.md.

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MT2DEngine.cmake)

if(MT2D_WEB)
    option(MT2D_SDL "Use the SDL2 backend instead of the native terminal one" ON)
    if(NOT MT2D_SDL)
        message(FATAL_ERROR "MT2D: Emscripten needs the SDL backend (-DMT2D_SDL=ON)")
    endif()
else()
    option(MT2D_SDL "Use the SDL2 backend instead of the native terminal one" OFF)
endif()

if(MT2D_SDL)
    mt2d_engine_target(mt2d sdl)
else()
    mt2d_engine_target(mt2d terminal)
endif()
add_library(mt2d::mt2d ALIAS mt2d)

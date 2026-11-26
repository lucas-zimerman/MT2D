cmake_minimum_required(VERSION 3.12)
project(spriteediting CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
# Every sprite is made in code, there are no assets.
# spriteediting: image + ASCII sprites (SDL, MS-DOS VGA with -DMT2D_DOS_VGA=ON, Linux Sixel with -DMT2D_TERMINAL_GRAPHICS=ON)
mt2d_add_example(spriteediting SOURCES src/main.cpp SDL DOS_VGA TERMINAL_GRAPHICS DOS_NAME spredit)
# spriteediting_text: the same program on the text backend, only the ASCII page has sprites to edit
mt2d_add_example(spriteediting_text SOURCES src/main.cpp DOS_NAME spredtxt)

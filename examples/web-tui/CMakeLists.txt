cmake_minimum_required(VERSION 3.12)
project(web-tui C CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
# Web only: configure with the Emscripten toolchain (emcmake cmake ...), see README.md.
mt2d_add_example(web-tui SOURCES src/main.c WEB)

cmake_minimum_required(VERSION 3.12)
project(asciigraphics CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
mt2d_add_example(asciigraphics SOURCES src/main.cpp DOS_NAME asciigfx)

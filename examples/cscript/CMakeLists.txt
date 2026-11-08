cmake_minimum_required(VERSION 3.12)
project(cscript CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
mt2d_add_example(cscript SOURCES src/main.cpp SDL DOS_VGA DOS_NAME cscript)

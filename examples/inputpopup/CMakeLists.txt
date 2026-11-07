cmake_minimum_required(VERSION 3.12)
project(inputpopup CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
mt2d_add_example(inputpopup SOURCES src/main.cpp DOS_NAME inputpop)

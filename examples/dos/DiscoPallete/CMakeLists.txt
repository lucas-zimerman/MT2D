cmake_minimum_required(VERSION 3.12)
project(DiscoPallete CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/MT2D.cmake)
mt2d_add_example(DiscoPallete SOURCES src/main.c DOS_ONLY NEEDS_VGA DOS_NAME discopal)

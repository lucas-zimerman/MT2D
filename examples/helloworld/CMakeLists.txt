cmake_minimum_required(VERSION 3.12)
project(helloworld CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
mt2d_add_example(helloworld SOURCES src/main.cpp DOS_NAME hello)

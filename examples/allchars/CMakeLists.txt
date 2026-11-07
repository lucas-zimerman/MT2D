cmake_minimum_required(VERSION 3.12)
project(allchars CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
mt2d_add_example(allchars SOURCES src/main.cpp DOS_NAME allchars)

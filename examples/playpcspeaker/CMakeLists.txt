cmake_minimum_required(VERSION 3.12)
project(playpcspeaker CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
mt2d_add_example(playpcspeaker SOURCES src/main.cpp src/songs.cpp DOS_NAME pcspeak)

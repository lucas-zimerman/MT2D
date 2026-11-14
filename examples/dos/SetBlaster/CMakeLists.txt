cmake_minimum_required(VERSION 3.12)
project(SetBlaster CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/MT2D.cmake)
mt2d_add_example(SetBlaster SOURCES src/main.c DOS_ONLY DOS_NAME setblast)

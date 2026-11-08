# Spaceship game benchmark for the MS-DOS VGA renderer, see bench/README.md.
#   cmake -S bench/ship -B build -DCMAKE_TOOLCHAIN_FILE=<repo>/examples/cmake/djgpp.cmake -DMT2D_DOS_VGA=ON
cmake_minimum_required(VERSION 3.12)
project(shipbench C CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../examples/cmake/MT2D.cmake)
mt2d_add_example(shipbench SOURCES ship.c SDL DOS_VGA DOS_NAME shipbench)
if(TARGET shipbench)
    target_link_options(shipbench PRIVATE -Wl,--wrap=malloc,--wrap=realloc)
endif()

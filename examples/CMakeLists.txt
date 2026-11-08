cmake_minimum_required(VERSION 3.12)
project(MT2D_Examples CXX)

# Builds every example. Each folder can also be built on its own, e.g.:
#   cmake -S examples/helloworld -B examples/helloworld/build && cmake --build examples/helloworld/build
# Executables are written to <example>/bin. See README.md.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MT2D.cmake)

foreach(example helloworld allchars inputpopup menu playpcspeaker playsound playmusic asciigraphics graphics cscript sprites desert web-tui web-gui)
    add_subdirectory(${example})
endforeach()

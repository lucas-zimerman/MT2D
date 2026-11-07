# CMake toolchain file for DJGPP (MS-DOS, 32 bit protected mode), cross compiling from Linux/macOS/Windows.
#
#   export PATH=/path/to/djgpp/bin:$PATH
#   cmake -S examples/helloworld -B examples/helloworld/build -DCMAKE_TOOLCHAIN_FILE=$PWD/examples/cmake/djgpp.cmake
#   cmake --build examples/helloworld/build
#
# DJGPP_PREFIX can be set to another triplet (default i586-pc-msdosdjgpp).
if(NOT DJGPP_PREFIX)
    set(DJGPP_PREFIX i586-pc-msdosdjgpp)
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i586)
set(DJGPP TRUE)

set(CMAKE_C_COMPILER ${DJGPP_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${DJGPP_PREFIX}-g++)
set(CMAKE_AR ${DJGPP_PREFIX}-ar CACHE FILEPATH "")
set(CMAKE_RANLIB ${DJGPP_PREFIX}-ranlib CACHE FILEPATH "")
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

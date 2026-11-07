cmake_minimum_required(VERSION 3.10)
project(MT2D_Tests C CXX)

# Tests for the portable (terminal mode, no SDL) part of the MT2D engine.
#
#   cmake -S tests -B build/tests && cmake --build build/tests && ctest --test-dir build/tests --output-on-failure

# New engine code is C (see AGENTS.md), tests are C++.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# gnu++ (not strict c++) so that the `linux` macro used by the engine is defined.
set(CMAKE_CXX_EXTENSIONS ON)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(ENGINE "${REPO_ROOT}/MT2D")

# Some engine sources include headers with a different letter case
# (MT2D/Objectcore/...) or with backslashes (MT2D\MT2D.h). Those only work on
# Windows, so build a small shadow include tree instead of editing the engine.
set(SHADOW "${CMAKE_CURRENT_BINARY_DIR}/shadow")
file(MAKE_DIRECTORY "${SHADOW}/MT2D")
file(CREATE_LINK "${ENGINE}/ObjectCore" "${SHADOW}/MT2D/Objectcore" SYMBOLIC)
file(WRITE "${SHADOW}/MT2D\\MT2D.h" "#include <MT2D/MT2D.h>\n")
file(WRITE "${SHADOW}/MT2D\\MT2D_Debug.h" "#include <MT2D/MT2D_Debug.h>\n")

add_library(mt2d_engine STATIC
    ${ENGINE}/MT2D.cpp
    ${ENGINE}/MT2D_Layout.cpp
    ${ENGINE}/Audio/MT2D_Audio_core.cpp
    ${ENGINE}/Container/aes.cpp
    ${ENGINE}/Container/MT2D_Container.cpp
    ${ENGINE}/ObjectCore/Cscript.cpp
    ${ENGINE}/ObjectCore/MT2D_Text_Image.cpp
    ${ENGINE}/ObjectCore/ObjectGlobals.c
    ${ENGINE}/ObjectCore/ObjectLoader.c
    ${ENGINE}/ObjectCore/Object.cpp
    ${ENGINE}/ObjectCore/SpriteBlock.cpp
    ${ENGINE}/ObjectCore/Sprites.cpp
    ${ENGINE}/ObjectCore/State.cpp
    ${ENGINE}/ObjectCore/STDFunctions.cpp
    ${ENGINE}/ObjectCore/Vars.cpp
    ${ENGINE}/Tools/Video/MT2D_Generic_Line.cpp
    ${ENGINE}/_LINUX/MT2D_Linux_CP850.cpp
)
# compat first so its MT2D/MT2D.h wins over the real one.
target_include_directories(mt2d_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/framework"
    "${SHADOW}"
    "${REPO_ROOT}"
)
# The engine is old code, keep the build output readable.
target_compile_options(mt2d_engine PRIVATE -w $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)

# The text compiler (ObjectCore/Parser) is optional, games that ship compiled objects only need the loader.
option(MT2D_CSCRIPT_PARSER "Build the Cscript/object text compiler" ON)
set(TEST_NAMES vars state spriteblock cscript object sprites text_image stdfunctions container aes display layout cp850 objectloader objectglobals)
if(MT2D_CSCRIPT_PARSER)
    target_sources(mt2d_engine PRIVATE ${ENGINE}/ObjectCore/Parser/CscriptCompiler.c)
    target_compile_definitions(mt2d_engine PUBLIC MT2D_USING_CSCRIPT_PARSER)
    list(APPEND TEST_NAMES cscriptcompiler)
    add_executable(cscriptc ${ENGINE}/ObjectCore/Parser/cscriptc.c)
    target_link_libraries(cscriptc mt2d_engine)
endif()

enable_testing()

foreach(name ${TEST_NAMES})
    add_executable(test_${name} test_${name}.cpp)
    target_link_libraries(test_${name} mt2d_engine)
    target_compile_options(test_${name} PRIVATE -w -fpermissive)
    # Tests create temporary files, run each one in its own scratch directory.
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run_${name}")
    add_test(NAME ${name} COMMAND test_${name} WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run_${name}")
endforeach()

#MT2D Project Loader, created by Lucas Zimerman Fraulob
#This CMake will help you to setup most MT2D Projects


cmake_minimum_required(VERSION 2.8)

project(mt2d_project)

set(MT2D_CONFIG_FILE "" CACHE PATH "")
set(MT2D_SRC "" CACHE PATH "")
set(PROJECT_SRC "" CACHE PATH "")
set(DEBUG_INFO No CACHE BOOL "")
set(DEBUG_OBJECT_CORE No CACHE BOOL "")
set(FULLSCREEN No CACHE BOOL "")
set(AUDIO_SUPPORT No CACHE BOOL "")
set(USE_SDL Yes CACHE BOOL "")
set(SDL_SRC "" CACHE PATH "")
set(SDL_LIB "" CACHE FILEPATH "")
set(SDL_MAIN_LIB "" CACHE FILEPATH "")
set(SDL_IMAGE_SRC "" CACHE PATH "")	
set(SDL_IMAGE_LIB "" CACHE FILEPATH "")
set(SDL_MIXER_SRC Yes CACHE PATH "")
set(SDL_MIXER_LIB "" CACHE FILEPATH "")

file(GLOB_RECURSE mt2dcode
    ${MT2D_SRC}/MT2D/*.h
    ${MT2D_SRC}/MT2D/*.cpp
)
file(GLOB_RECURSE projectCode
    ${PROJECT_SRC}/*.h
    ${PROJECT_SRC}/*.cpp
)

file(GLOB sdlcode
    ${SDL_SRC}/*.h
)

file(GLOB sdlimgcode
    ${SDL_IMAGE_SRC}/*.h
)

file(GLOB sdlmixcode
    ${SDL_MIXER_SRC}/*.h
)
add_executable(myapp ${mt2dcode} ${projectCode} ${sdlcode} ${sdlimgcode} ${sdlmixcode} ${VARS_SOURCES})

#Adapted code based on Florian's source group
#https://stackoverflow.com/questions/32645217/visual-studio-as-an-editor-for-cmake-friendly-project
foreach(_source IN ITEMS ${projectCode})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${projectCode}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


foreach(_source IN ITEMS ${mt2dcode})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${MT2D_SRC}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


include_directories(${MT2D_SRC})

target_include_directories(myapp  PUBLIC  ${MT2D_SRC})

set(configFile "/*CMake generated Config File*/\n")
if(USE_SDL MATCHES ON)
	set(configFile "${configFile}#define SDL_USE\n")
	
	if (EXISTS "${SDL_SRC}")
		include_directories(myapp PRIVATE "${SDL_SRC}")
	else()
		message(FATAL_ERROR "unexpected: path to Src SDL2 not found ")
	endif()
	if (NOT EXISTS "${SDL_LIB}")
		message(FATAL_ERROR "unexpected: path to Link SDL2 not found ")
	else()
		target_link_libraries(myapp "${SDL_LIB}")
	endif()
	if (NOT EXISTS "${SDL_MAIN_LIB}")
		message(FATAL_ERROR "unexpected: path to Link SDL2 main not found ")
	else()
		target_link_libraries(myapp "${SDL_MAIN_LIB}")
	endif()
		
	
	if (NOT EXISTS "${SDL_IMAGE_SRC}")
		message(FATAL_ERROR "unexpected: path to SDL2_Image not found ")
	else()
		include_directories(myapp PRIVATE "${SDL_IMAGE_SRC}")
	endif()
	if (NOT EXISTS "${SDL_IMAGE_LIB}")
		message(FATAL_ERROR "unexpected: path to Link SDL2 not found ")
	else()
		target_link_libraries(myapp "${SDL_IMAGE_LIB}")
	endif()
	
endif()
if(FULLSCREEN MATCHES OFF)
	set(configFile "${configFile}#define MT2D_WINDOWED_MODE\n")
endif()
if(AUDIO_SUPPORT MATCHES ON)
	set(configFile "${configFile}#define SDL_USE_AUDIO\n")
	
	if(USE_SDL MATCHES ON)
		if (EXISTS SDL_MIXER_SRC)
			message(FATAL_ERROR "unexpected: path to SDL Mixer not found ")
		else()
			include_directories(myapp PRIVATE "${SDL_MIXER_SRC}")
		endif()
	endif()
	if (NOT EXISTS "${SDL_MIXER_LIB}")
		message(FATAL_ERROR "unexpected: path to Link SDL2 Mixer not found ")
	else()
		target_link_libraries(myapp "${SDL_MIXER_LIB}")
	endif()
	
endif()
if(DEBUG_OBJECT_CORE MATCHES ON)
	set(configFile "${configFile}#define DEBUG_OBJECTCORE\n")
endif()
if(DEBUG_INFO MATCHES ON)
	set(configFile "${configFile}#define MT2D_DEBUG_ENABLED\n")
endif()





include_directories(myapp PRIVATE "${MT2D_SRC}")





FILE(WRITE ${MT2D_CONFIG_FILE}/MT2D_Config.h
 "${configFile}"
)

install(TARGETS myapp DESTINATION bin)
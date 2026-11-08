cmake_minimum_required(VERSION 3.12)
project(web-gui C CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
# The images and sounds already exist in other examples, they are preloaded into the browser's virtual file system from there.
set(SPR ${CMAKE_CURRENT_SOURCE_DIR}/../sprites/assets)
set(SND ${CMAKE_CURRENT_SOURCE_DIR}/../playsound/assets)
set(MUS ${CMAKE_CURRENT_SOURCE_DIR}/../playmusic/assets)
set(WEB_FILES
    ${SPR}/robot0.png@/robot0.png ${SPR}/robot1.png@/robot1.png
    ${SPR}/ghost0.png@/ghost0.png ${SPR}/ghost1.png@/ghost1.png
    ${SPR}/coin0.png@/coin0.png ${SPR}/coin1.png@/coin1.png ${SPR}/coin2.png@/coin2.png ${SPR}/coin3.png@/coin3.png
    ${SND}/coin.wav@/coin.wav ${SND}/explosion.ogg@/explosion.ogg ${SND}/powerup.wav@/powerup.wav
    ${MUS}/ode.ogg@/ode.ogg)
mt2d_add_example(web-gui SOURCES src/main.c WEB PRELOAD ${WEB_FILES})

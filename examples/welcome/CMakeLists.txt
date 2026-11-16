cmake_minimum_required(VERSION 3.12)
project(welcome C CXX)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/MT2D.cmake)
# The images and sounds already exist in other examples (only assets/gameover.png is new, see tools/make_banner.py), they are preloaded into the browser's virtual file system from there.
set(GFX ${CMAKE_CURRENT_SOURCE_DIR}/../graphics/assets)
set(SPR ${CMAKE_CURRENT_SOURCE_DIR}/../sprites/assets)
set(DES ${CMAKE_CURRENT_SOURCE_DIR}/../desert/assets)
set(SND ${CMAKE_CURRENT_SOURCE_DIR}/../playsound/assets)
set(MUS ${CMAKE_CURRENT_SOURCE_DIR}/../playmusic/assets)
set(WEB_FILES "")
foreach(f ${GFX}/stars_far.png ${GFX}/stars_near.png ${GFX}/ship.png ${GFX}/planet_red.png ${GFX}/planet_blue.png ${GFX}/rock.png
          ${GFX}/coin0.png ${GFX}/coin1.png ${GFX}/coin2.png ${GFX}/coin3.png
          ${SPR}/robot0.png ${SPR}/robot1.png ${SPR}/ghost0.png ${SPR}/ghost1.png
          ${DES}/sky.png ${DES}/mesas.png ${DES}/dunes.png ${DES}/ground.png ${DES}/foreground.png
          ${DES}/truck0.png ${DES}/truck1.png ${DES}/truck2.png ${DES}/desert.ogg
          ${DES}/drift_u1.png ${DES}/drift_u2.png ${DES}/drift_d1.png ${DES}/drift_d2.png
          ${DES}/head_up.png ${DES}/head_mid.png ${DES}/head_down.png ${DES}/mound0.png ${DES}/mound1.png
          ${DES}/seg_big.png ${DES}/seg_small.png ${DES}/grain0.png ${DES}/grain1.png
          ${DES}/dust0.png ${DES}/dust1.png ${DES}/dust2.png ${DES}/dust3.png
          ${DES}/roar.wav ${DES}/screech.wav ${DES}/thud.wav ${DES}/whoosh.wav
          ${CMAKE_CURRENT_SOURCE_DIR}/assets/gameover.png
          ${SND}/blip.wav ${SND}/coin.wav ${SND}/explosion.ogg ${SND}/jump.ogg ${SND}/laser.wav ${SND}/powerup.wav
          ${MUS}/ode.ogg ${MUS}/elise.ogg)
    get_filename_component(name ${f} NAME)
    list(APPEND WEB_FILES "${f}@/${name}")
endforeach()
# page.html is the page of the web site (GitHub Pages), Logo.png is copied next to it
mt2d_add_example(welcome SOURCES src/main.c WEB SHELL page.html ASSETS ../../Logo.png PRELOAD ${WEB_FILES})

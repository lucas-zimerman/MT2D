/*===================================================================
MT2D_Linux_Init.h/ MT2D_Linux_Init.cpp

Init of the terminal (curses) backend used by Linux and macOS.
===================================================================*/
#if defined(linux) && !defined(MT2DLIN_INIT_H)
#define MT2DLIN_INIT_H

bool MT2D_Linux_Init();

#endif

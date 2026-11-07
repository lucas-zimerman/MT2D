/*===================================================================
MT2D_Linux_CP850.h / MT2D_Linux_CP850.cpp

Code page 850 (MS-DOS Latin 1) <-> Unicode conversion used by the
terminal (curses) backend of Linux and macOS.

The engine display buffers (WINDOW1/WINDOW2) hold CP850 bytes, like on
MS-DOS. Modern terminals speak Unicode, so every byte is converted to
its Unicode character before being drawn, and every typed character
is converted back to CP850 before it is given to the game.

This file does not depend on curses.
===================================================================*/
#ifndef MT2DLIN_CP850_H
#define MT2DLIN_CP850_H

/* Unicode code point of a CP850 byte, the way the IBM PC video memory shows it:
   1..31 and 127 are the graphical symbols (smiley, arrows...), 0 and 255 are blank. */
unsigned int MT2D_CP850_To_Unicode(unsigned char c);

/* CP850 byte of a Unicode code point, or -1 when CP850 has no such character. */
int MT2D_Unicode_To_CP850(unsigned int unicode);

#endif

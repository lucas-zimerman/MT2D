/* CscriptCompiler.h
   Compiles the object text format into the compact binary format read by ObjectCore_Load
   (see ObjectLoader.h). Only built when MT2D_USING_CSCRIPT_PARSER is defined (CMake option
   MT2D_CSCRIPT_PARSER), games that ship compiled files do not need it. It uses the STL, so it is
   meant for desktop tools (cscriptc) and for development builds.

        object movingdot
        {
            int Pos_X = 10;         // the built in vars can be set, anything else is a new int var
            int speed = 2;

            states
            {
            Spawn:                              // the first state is where the object starts
                "SPRITEX" 10 VarAdd(Pos_X, speed)   // sprite, ticks to wait, optional action
                none      0  JumpToStateIfVarHigher(Done, Pos_X, 300)
                Goto Spawn
            Done:
                Stop                            // Stop removes the object
            }
        }

   * A state restarts from its first frame when its last frame ends, so a state is a loop.
   * The action of a frame runs when the wait of that frame ends.
   * "path" is a sprite file, none is no sprite. Action arguments are numbers, names (object vars
     or states, max 18 chars), "texts" and the operators < > == != >= <= for JumpFramesIfVar.
   * Function names are not case sensitive, they are listed in CscriptFunctions.h.
   * // and slash-star comments are allowed.
*/
#pragma once
#ifndef MT2D_CSCRIPT_COMPILER_H
#define MT2D_CSCRIPT_COMPILER_H

#if defined(MT2D_USING_CSCRIPT_PARSER)
#include <string>
#include <vector>
#include <MT2D/ObjectCore/Object.h>

/* Returns false and fills error ("line N: message") when the text is not valid. */
bool CscriptCompiler_Compile(const char *text, std::vector<unsigned char> &out, std::string &error);

/* Compiles the text file and loads the object. NULL on error (error is optional). */
Object *ObjectCore_LoadScript(const char *textFile, std::string *error = 0);
#endif

#endif

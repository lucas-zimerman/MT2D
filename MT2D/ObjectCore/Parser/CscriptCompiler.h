/* CscriptCompiler.h
   Compiles the object text format into the compact binary format read by ObjectCore_Load
   (see ObjectLoader.h). Only built when MT2D_USING_CSCRIPT_PARSER is defined (CMake option
   MT2D_CSCRIPT_PARSER), games that ship compiled files do not need it. It uses the STL, so it is
   meant for desktop tools (cscriptc) and for development builds.

        object movingdot
        {
            int Pos_X = 10;           // the built in vars can be set, anything else is a new int var
            int speed = 2;

            states
            {
            Spawn:                    // the first state is where the object starts
                Show "SPRITEX"        // Show belongs to the Wait right after it
                Wait 10
                Pos_X += speed        // runs when the wait above ends
                if (Pos_X > 300) { Goto Done }
                Goto Spawn
            Done:
                Stop                  // Stop removes the object
            }
        }

   * Show "file" + Wait n : shows the sprite for n ticks. A Wait that does not come right after a
     Show shows no sprite, and a Show that is not directly followed by Wait is an error.
     Show none is allowed.
   * Statements run in order. x++  x--  x += v  x -= v  x = v  (v is a number or a var),
     Function(args), Goto State and Stop each take no time of their own.
     An action runs when the Wait before it ends, so
         Show "A"  Wait 4  x++  Show "B"  Wait 5      shows A for 4 ticks, then x++, then B for 5.
   * while (condition) { ... }, if (condition) { ... } else if (condition) { ... } else { ... }
     A condition is made of comparisons (< > == = != >= <=) between a var and a var or number,
     combined with && , || , ! and parentheses. A lone var or number is true when it is not 0.
     && and || stop at the first answer, like in C. The blocks are compiled to JumpFrames and
     JumpFramesIfVar frames (no wait, no sprite), so they cost no time by themselves. A loop where
     no frame waits never gives the control back to the game, that is up to the script.
     State labels cannot be inside a block, use Goto to leave one.
   * A state restarts from its first frame when its last frame ends, so a state is a loop.
   * Function arguments are numbers, names (object vars or states, max 18 chars), "texts" and the
     operators < > == != >= <= (for JumpFramesIfVar). Function names are not case sensitive, they
     are listed in CscriptFunctions.h. Do not call the JumpFrames... or JumpNextFrame... functions by hand
     with frame counts: the compiler merges frames, use while/if instead.
   * solid; and renderonly; set the flags of the object (see Object).
   * Show image "file" w h and Show container "file" w h load an image (file or container) scaled to
     w x h, plain Show "file" is an ASCII sprite. The same file and size is loaded once.
   * if (random(25)) is true 25 percent of the time (0 to 100, a number).
   * global marks something the game registers (ObjectGlobals.h): a var (global Health--,
     Pos_X -= global GameSpeed, if (global JumpRequest == 1)), an object (the first argument of
     GotoStateIfHitObject and CreateObject, written global Doggo) or a function (global Spawn(1, 2)).
     The compiler cannot know them, ObjectCore_Load fails if one is not registered.
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

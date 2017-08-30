==============================================================================
                     MONOCHROME TEXT 2D ENGINE
==============================================================================

1.What is MT2D
2.What operational system would it work?
3.Basics


1)What is MT2D
 Monochrome text 2D engine, is an FRAMEWORK where you can produce saftware 
based on text user interface, using a real terminal or an emulated one.
 As the name says, there's no color, and there's no size change, it's just the
basic 25x80 screen (there may be some platform specific console size where the
size is not going to be 25x80).

2)What operational system would it work?
 WINDOWS, MS-DOS, LINUX AND EVEN WINDOWS PHONE. (or other os that support SDL).

 3)Basics.
MT2D is divided into pieces of code where each piece does a specific job, this
way you can select what pieces you want to load into yor software, avoiding
that the compiler loads uneeded code into your software.
 The main pieces are:
-MT2D: deals with the basic screen code, it initializes the main routines
 that are needed to run before any other MT2D related code and also stores
 the screen buffer.

-Display: Used for Rendering and Clearing the screen.
 
-Keyboard: Used to interact with a keyboard.
 
-Layout: used to draw custom a square box, you can define the way this square
 box will looks like (mostly used for making windows or even the software layout).
 
-System Calls: Used for making system specific calls like Delay and PC Speaker Beeps.

-Terminal Define: where you configure the way MT2D should work or be compiled.

-Audio Core: The audio engine from MT2D.

-Cursor: Interact witht the screen cursor.

-Input Box: shows a popup with a message and receive an user data.

-Message Box: shows a user popup with a message.

-ObjectCore: a bunch of code for making game logic objects.

-SDL: SDL specific code to use SDL as an emulated terminal.

-Generic Line: Draw a line.

-Menu: Shows a generic custom menu where an user can select an option from a list of options.
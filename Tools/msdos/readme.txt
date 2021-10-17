Use the following tools if your Environment supports Long File Names but the Debugger doesn't.
NOTE: You'll only need the tools if you plan to debug the Framework code.

mkshort.bat

Transform the long file name files into short file names, the change will only take effect to source files 
and not headers.
The source files will be located on mt2d/_msdos/_short/*


syncshor.bat

If you made any changes to the files from the _short folder, this script will overwrite the LFN variants of the short files.
That's required since the main codebase will ignore short files to keep a single codebase in sync.
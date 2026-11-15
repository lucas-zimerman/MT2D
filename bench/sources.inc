# Engine sources the benchmarks link (sourced by the build scripts). $E is the MT2D folder of the tree being built.
# Older checkouts have these ObjectCore files as .cpp, newer ones as .c.
SRC=""
for n in Cscript Object State STDFunctions Vars SpriteBlock Sprites MT2D_Text_Image; do
    if [ -f "$E/ObjectCore/$n.c" ]; then SRC="$SRC $E/ObjectCore/$n.c"; else SRC="$SRC $E/ObjectCore/$n.cpp"; fi
done
SRC="$SRC $E/MT2D.c $E/MT2D_Layout.c $E/Audio/MT2D_Audio_core.c $E/Container/aes.c $E/Container/MT2D_Container.c $E/Tools/Video/MT2D_Generic_Line.c $E/_LINUX/MT2D_Linux_CP850.c"

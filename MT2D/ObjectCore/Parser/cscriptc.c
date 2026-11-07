/* cscriptc: compiles an object text file into a .mto file for ObjectCore_Load.
        cscriptc movingdot.txt movingdot.mto
*/
#include <stdio.h>
#include <stdlib.h>
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>

int main(int argc, char **argv) {
        FILE *in, *out;
        long length;
        char *text;
        unsigned char *bin = 0;
        unsigned long binSize = 0;
        char error[256];
        int written;

        if (argc != 3) {
                fprintf(stderr, "usage: cscriptc <input.txt> <output.mto>\n");
                return 2;
        }
        in = fopen(argv[1], "rb");
        if (!in) {
                fprintf(stderr, "cscriptc: cannot open %s\n", argv[1]);
                return 1;
        }
        fseek(in, 0, SEEK_END);
        length = ftell(in);
        fseek(in, 0, SEEK_SET);
        text = (char*)malloc((size_t)length + 1);
        if (!text) { fclose(in); fprintf(stderr, "cscriptc: out of memory\n"); return 1; }
        length = (long)fread(text, 1, (size_t)length, in);
        text[length] = 0;
        fclose(in);
        if (!CscriptCompiler_Compile(text, &bin, &binSize, error, (int)sizeof(error))) {
                fprintf(stderr, "%s:%s\n", argv[1], error);
                free(text);
                return 1;
        }
        free(text);
        out = fopen(argv[2], "wb");
        written = out && fwrite(bin, 1, binSize, out) == binSize;
        if (out && fclose(out) != 0) written = 0;
        CscriptCompiler_Free(bin);
        if (!written) {
                fprintf(stderr, "cscriptc: cannot write %s\n", argv[2]);
                return 1;
        }
        return 0;
}

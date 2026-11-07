/* cscriptc: compiles an object text file into a .mto file for ObjectCore_Load.
        cscriptc movingdot.txt movingdot.mto
*/
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>

int main(int argc, char **argv) {
        if (argc != 3) {
                fprintf(stderr, "usage: cscriptc <input.txt> <output.mto>\n");
                return 2;
        }
        std::ifstream in(argv[1], std::ios::in | std::ios::binary);
        if (!in) {
                fprintf(stderr, "cscriptc: cannot open %s\n", argv[1]);
                return 1;
        }
        std::stringstream buf;
        buf << in.rdbuf();
        std::vector<unsigned char> out;
        std::string error;
        if (!CscriptCompiler_Compile(buf.str().c_str(), out, error)) {
                fprintf(stderr, "%s:%s\n", argv[1], error.c_str());
                return 1;
        }
        std::ofstream o(argv[2], std::ios::out | std::ios::binary);
        o.write((const char *)&out[0], (std::streamsize)out.size());
        if (!o) {
                fprintf(stderr, "cscriptc: cannot write %s\n", argv[2]);
                return 1;
        }
        return 0;
}

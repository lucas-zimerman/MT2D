/* CscriptCompiler.cpp, see CscriptCompiler.h */
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>

#if defined(MT2D_USING_CSCRIPT_PARSER)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <MT2D/ObjectCore/CscriptFunctions.h>
#include <MT2D/ObjectCore/ObjectLoader.h>

namespace {

const int NAME_MAX_LEN = 18; /* MT2D_VAR_NAME_MAX - 1 */

struct FnInfo { const char *Name; int Kind; int Arity; };
#define COMPILER_FN_ENTRY(Name, Symbol, Kind, Arity) { #Name, 0, Arity },
const FnInfo Functions[] = { MT2D_CSCRIPT_FUNCTIONS(COMPILER_FN_ENTRY) };
#undef COMPILER_FN_ENTRY
const int FunctionCount = (int)(sizeof(Functions) / sizeof(Functions[0]));

const char *BuiltinVars[] = { "Pos_X", "Pos_Y", "Size_X", "Size_Y", "Acel_X", "Acel_Y" };

struct CompileError { std::string Message; };

enum TokType { T_END, T_IDENT, T_INT, T_STR, T_PUNCT };
struct Tok { TokType Type; std::string Text; long Value; int Line; };

bool IEq(const std::string &a, const char *b) {
        size_t n = strlen(b);
        if (a.size() != n) return false;
        for (size_t i = 0; i < n; i++) {
                if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
        }
        return true;
}

void Fail(int line, const std::string &msg) {
        std::ostringstream o;
        o << "line " << line << ": " << msg;
        CompileError e;
        e.Message = o.str();
        throw e;
}

/* ---- tokenizer ---------------------------------------------------------- */
std::vector<Tok> Tokenize(const char *p) {
        std::vector<Tok> out;
        int line = 1;
        while (*p) {
                char c = *p;
                if (c == '\n') { line++; p++; }
                else if (isspace((unsigned char)c)) { p++; }
                else if (c == '/' && p[1] == '/') { while (*p && *p != '\n') p++; }
                else if (c == '/' && p[1] == '*') {
                        p += 2;
                        while (*p && !(p[0] == '*' && p[1] == '/')) { if (*p == '\n') line++; p++; }
                        if (!*p) Fail(line, "comment is never closed");
                        p += 2;
                }
                else if (isalpha((unsigned char)c) || c == '_') {
                        Tok t; t.Type = T_IDENT; t.Value = 0; t.Line = line;
                        while (isalnum((unsigned char)*p) || *p == '_') t.Text += *p++;
                        out.push_back(t);
                }
                else if (isdigit((unsigned char)c) || (c == '-' && isdigit((unsigned char)p[1]))) {
                        Tok t; t.Type = T_INT; t.Line = line;
                        const char *start = p;
                        if (*p == '-') p++;
                        while (isdigit((unsigned char)*p)) p++;
                        t.Text.assign(start, p - start);
                        t.Value = atol(t.Text.c_str());
                        if (t.Text.size() > 11 || t.Value > 2147483647L || t.Value < -2147483647L - 1) Fail(line, "number is too big: " + t.Text);
                        out.push_back(t);
                }
                else if (c == '"') {
                        Tok t; t.Type = T_STR; t.Value = 0; t.Line = line;
                        p++;
                        while (*p && *p != '"' && *p != '\n') t.Text += *p++;
                        if (*p != '"') Fail(line, "text is never closed");
                        p++;
                        out.push_back(t);
                }
                else {
                        Tok t; t.Type = T_PUNCT; t.Value = 0; t.Line = line;
                        t.Text = std::string(1, c);
                        if ((c == '=' || c == '!' || c == '<' || c == '>') && p[1] == '=') { t.Text += '='; p++; }
                        else if (!strchr("{}(),;:<>=!", c)) Fail(line, std::string("unexpected character '") + c + "'");
                        p++;
                        out.push_back(t);
                }
        }
        Tok e; e.Type = T_END; e.Value = 0; e.Line = line;
        out.push_back(e);
        return out;
}

/* ---- model -------------------------------------------------------------- */
struct Arg { int Kind; long Value; std::string Text; int Line; }; /* 0 number, 1 name, 2 text */
struct Frame { int Sprite; int Wait; int Fn; std::vector<Arg> Args; int Line; };
struct State { std::string Name; int Line; std::vector<Frame> Frames; };
struct VarRec { int Slot; std::string Name; long Value; };

class Compiler {
public:
        explicit Compiler(const std::vector<Tok> &t) : T(t), P(0) {}

        void ParseFile() {
                ExpectKeyword("object");
                if (Cur().Type != T_IDENT) Fail(Cur().Line, "expected the object name");
                P++; /* the name only documents the file, ObjectCore_Load takes the file name */
                ExpectPunct("{");
                while (!IsPunct("}")) {
                        if (IsKeyword("int")) ParseVar();
                        else if (IsKeyword("states")) ParseStates();
                        else Fail(Cur().Line, "expected 'int', 'states' or '}'");
                }
                P++;
                if (Cur().Type != T_END) Fail(Cur().Line, "unexpected text after the object");
                if (States.empty()) Fail(Cur().Line, "the object has no states");
                Validate();
        }

        void Write(std::vector<unsigned char> &o) const {
                o.push_back('M'); o.push_back('T'); o.push_back('O'); o.push_back('1');
                Put16(o, (unsigned)Sprites.size());
                for (size_t i = 0; i < Sprites.size(); i++) {
                        o.push_back((unsigned char)Sprites[i].size());
                        o.insert(o.end(), Sprites[i].begin(), Sprites[i].end());
                }
                Put16(o, (unsigned)States.size());
                for (size_t s = 0; s < States.size(); s++) {
                        const State &st = States[s];
                        o.push_back((unsigned char)st.Name.size());
                        o.insert(o.end(), st.Name.begin(), st.Name.end());
                        Put16(o, (unsigned)st.Frames.size());
                        for (size_t f = 0; f < st.Frames.size(); f++) {
                                const Frame &fr = st.Frames[f];
                                Put16(o, fr.Sprite < 0 ? 0xFFFF : (unsigned)fr.Sprite);
                                Put16(o, (unsigned)fr.Wait);
                                Put16(o, fr.Fn < 0 ? 0xFFFF : (unsigned)fr.Fn);
                                if (fr.Fn < 0) continue;
                                unsigned strBytes = 0;
                                for (size_t a = 0; a < fr.Args.size(); a++) if (fr.Args[a].Kind == 2) strBytes += (unsigned)fr.Args[a].Text.size() + 1;
                                o.push_back((unsigned char)fr.Args.size());
                                Put16(o, strBytes);
                                for (size_t a = 0; a < fr.Args.size(); a++) {
                                        const Arg &ar = fr.Args[a];
                                        o.push_back((unsigned char)ar.Kind);
                                        if (ar.Kind == 0) Put32(o, ar.Value);
                                        else {
                                                o.push_back((unsigned char)ar.Text.size());
                                                o.insert(o.end(), ar.Text.begin(), ar.Text.end());
                                        }
                                }
                        }
                }
                unsigned newVars = 0;
                for (size_t i = 0; i < Vars.size(); i++) if (Vars[i].Slot < 0) newVars++;
                o.push_back((unsigned char)newVars);
                o.push_back((unsigned char)Vars.size());
                for (size_t i = 0; i < Vars.size(); i++) {
                        const VarRec &v = Vars[i];
                        o.push_back(v.Slot < 0 ? 0xFF : (unsigned char)v.Slot);
                        if (v.Slot < 0) {
                                o.push_back((unsigned char)v.Name.size());
                                o.insert(o.end(), v.Name.begin(), v.Name.end());
                        }
                        Put32(o, v.Value);
                }
        }

private:
        const std::vector<Tok> &T;
        size_t P;
        std::vector<VarRec> Vars;
        std::vector<State> States;
        std::vector<std::string> Sprites;
        std::map<std::string, int> SpriteIndex;

        static void Put16(std::vector<unsigned char> &o, unsigned v) { o.push_back((unsigned char)(v & 0xFF)); o.push_back((unsigned char)((v >> 8) & 0xFF)); }
        static void Put32(std::vector<unsigned char> &o, long v) {
                unsigned long u = (unsigned long)v;
                for (int i = 0; i < 4; i++) o.push_back((unsigned char)((u >> (8 * i)) & 0xFF));
        }

        const Tok &Cur() const { return T[P]; }
        const Tok &Peek(size_t n) const { return T[P + n < T.size() ? P + n : T.size() - 1]; }
        bool IsPunct(const char *s) const { return Cur().Type == T_PUNCT && Cur().Text == s; }
        bool IsKeyword(const char *s) const { return Cur().Type == T_IDENT && IEq(Cur().Text, s); }
        void ExpectPunct(const char *s) {
                if (!IsPunct(s)) Fail(Cur().Line, std::string("expected '") + s + "'");
                P++;
        }
        void ExpectKeyword(const char *s) {
                if (!IsKeyword(s)) Fail(Cur().Line, std::string("expected '") + s + "'");
                P++;
        }
        std::string ExpectName(const char *what) {
                if (Cur().Type != T_IDENT) Fail(Cur().Line, std::string("expected ") + what);
                if ((int)Cur().Text.size() > NAME_MAX_LEN) Fail(Cur().Line, "name '" + Cur().Text + "' is longer than 18 characters");
                return T[P++].Text;
        }

        void ParseVar() {
                P++; /* int */
                int line = Cur().Line;
                std::string name = ExpectName("a var name");
                long value = 0;
                if (IsPunct("=")) {
                        P++;
                        if (Cur().Type != T_INT) Fail(Cur().Line, "expected a number");
                        value = T[P++].Value;
                }
                ExpectPunct(";");
                VarRec v; v.Name = name; v.Value = value; v.Slot = -1;
                for (int i = 0; i < 6; i++) if (name == BuiltinVars[i]) v.Slot = i;
                for (size_t i = 0; i < Vars.size(); i++) if (Vars[i].Name == name) Fail(line, "var '" + name + "' is declared twice");
                if (Vars.size() >= 255) Fail(line, "too many vars");
                Vars.push_back(v);
        }

        void ParseStates() {
                P++; /* states */
                ExpectPunct("{");
                while (!IsPunct("}")) {
                        State st; st.Line = Cur().Line;
                        st.Name = ExpectName("a state name");
                        ExpectPunct(":");
                        for (size_t i = 0; i < States.size(); i++) if (States[i].Name == st.Name) Fail(st.Line, "state '" + st.Name + "' is declared twice");
                        while (!IsPunct("}") && !(Cur().Type == T_IDENT && Peek(1).Type == T_PUNCT && Peek(1).Text == ":" && !IsKeyword("none"))) {
                                st.Frames.push_back(ParseFrame());
                                if (IsPunct(";")) P++;
                        }
                        if (st.Frames.empty()) Fail(st.Line, "state '" + st.Name + "' has no frames");
                        if (st.Frames.size() > 65535) Fail(st.Line, "state '" + st.Name + "' has too many frames");
                        if (States.size() >= 65535) Fail(st.Line, "too many states");
                        States.push_back(st);
                }
                P++;
        }

        int SpriteId(const std::string &path, int line) {
                if (path.empty()) return -1;
                if (path.size() > 255) Fail(line, "sprite path is too long");
                std::map<std::string, int>::iterator it = SpriteIndex.find(path);
                if (it != SpriteIndex.end()) return it->second;
                if (Sprites.size() >= 65535) Fail(line, "too many sprites");
                int id = (int)Sprites.size();
                Sprites.push_back(path);
                SpriteIndex[path] = id;
                return id;
        }

        int FindFunction(const std::string &name) const {
                for (int i = 0; i < FunctionCount; i++) if (IEq(name, Functions[i].Name)) return i;
                return -1;
        }

        Frame Control(const char *fn, int line) {
                Frame f; f.Sprite = -1; f.Wait = 0; f.Line = line; f.Fn = FindFunction(fn);
                return f;
        }

        Frame ParseFrame() {
                Frame f; f.Sprite = -1; f.Wait = 0; f.Fn = -1; f.Line = Cur().Line;
                if (IsKeyword("goto")) {
                        P++;
                        f = Control("SetState", f.Line);
                        Arg a; a.Kind = 1; a.Value = 0; a.Line = Cur().Line; a.Text = ExpectName("a state name");
                        f.Args.push_back(a);
                        return f;
                }
                if (IsKeyword("stop")) {
                        P++;
                        return Control("RemoveObject", f.Line);
                }
                if (Cur().Type == T_STR) {
                        f.Sprite = SpriteId(Cur().Text, Cur().Line);
                        P++;
                }
                else if (IsKeyword("none")) {
                        P++;
                }
                else Fail(Cur().Line, "expected a frame: a \"sprite\" or none, then the wait, then an optional action");
                if (Cur().Type != T_INT || Cur().Value < 0 || Cur().Value > 65535) Fail(Cur().Line, "expected the wait (0 to 65535) after the sprite");
                f.Wait = (int)T[P++].Value;
                if (Cur().Type == T_IDENT && !IsKeyword("none") && !IsKeyword("goto") && !IsKeyword("stop") &&
                        !(Peek(1).Type == T_PUNCT && Peek(1).Text == ":")) {
                        ParseAction(f);
                }
                return f;
        }

        void ParseAction(Frame &f) {
                int line = Cur().Line;
                std::string name = Cur().Text;
                f.Fn = FindFunction(name);
                if (f.Fn < 0) Fail(line, "unknown function '" + name + "'");
                P++;
                if (IsPunct("(")) {
                        P++;
                        while (!IsPunct(")")) {
                                if (!f.Args.empty()) ExpectPunct(",");
                                f.Args.push_back(ParseArg());
                        }
                        P++;
                }
                if ((int)f.Args.size() != Functions[f.Fn].Arity) {
                        std::ostringstream o;
                        o << Functions[f.Fn].Name << " needs " << Functions[f.Fn].Arity << " argument(s), got " << f.Args.size();
                        Fail(line, o.str());
                }
        }

        Arg ParseArg() {
                Arg a; a.Value = 0; a.Line = Cur().Line;
                const Tok &t = Cur();
                if (t.Type == T_INT) { a.Kind = 0; a.Value = t.Value; P++; }
                else if (t.Type == T_STR) {
                        if (t.Text.size() > 255) Fail(t.Line, "text is too long");
                        a.Kind = 2; a.Text = t.Text; P++;
                }
                else if (t.Type == T_IDENT) { a.Kind = 1; a.Text = ExpectName("a name"); }
                else if (t.Type == T_PUNCT) {
                        a.Kind = 0;
                        if (t.Text == "<") a.Value = '<';
                        else if (t.Text == ">") a.Value = '>';
                        else if (t.Text == "=" || t.Text == "==") a.Value = '=';
                        else if (t.Text == "!=") a.Value = '!';
                        else if (t.Text == ">=") a.Value = 242;
                        else if (t.Text == "<=") a.Value = 243;
                        else Fail(t.Line, "expected an argument");
                        P++;
                }
                else Fail(t.Line, "expected an argument");
                return a;
        }

        /* every name used by an action must be a var, a built in var or a state */
        void Validate() {
                std::set<std::string> known;
                for (int i = 0; i < 6; i++) known.insert(BuiltinVars[i]);
                for (size_t i = 0; i < Vars.size(); i++) known.insert(Vars[i].Name);
                for (size_t i = 0; i < States.size(); i++) known.insert(States[i].Name);
                for (size_t s = 0; s < States.size(); s++)
                        for (size_t f = 0; f < States[s].Frames.size(); f++) {
                                const Frame &fr = States[s].Frames[f];
                                for (size_t a = 0; a < fr.Args.size(); a++)
                                        if (fr.Args[a].Kind == 1 && !known.count(fr.Args[a].Text))
                                                Fail(fr.Args[a].Line, "'" + fr.Args[a].Text + "' is not a var or a state");
                        }
        }
};

} // namespace

bool CscriptCompiler_Compile(const char *text, std::vector<unsigned char> &out, std::string &error) {
        out.clear();
        error.clear();
        if (!text) { error = "no text"; return false; }
        try {
                std::vector<Tok> tokens = Tokenize(text);
                Compiler c(tokens);
                c.ParseFile();
                c.Write(out);
        }
        catch (CompileError &e) {
                out.clear();
                error = e.Message;
                return false;
        }
        return true;
}

Object *ObjectCore_LoadScript(const char *textFile, std::string *error) {
        std::string local;
        std::string &err = error ? *error : local;
        std::ifstream in(textFile, std::ios::in | std::ios::binary);
        if (!in) { err = std::string("cannot open ") + textFile; return 0; }
        std::stringstream buf;
        buf << in.rdbuf();
        std::vector<unsigned char> bin;
        std::string text = buf.str();
        if (!CscriptCompiler_Compile(text.c_str(), bin, err)) return 0;
        Object *o = ObjectCore_LoadFromMemory(&bin[0], (unsigned long)bin.size());
        if (!o) err = "the compiled object could not be loaded (missing sprite file?)";
        return o;
}
#endif

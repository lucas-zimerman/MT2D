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
#define COMPILER_KIND_OBJ 0
#define COMPILER_KIND_VARS 1
#define COMPILER_KIND_OBJOBJ 2
#define COMPILER_FN_ENTRY(Name, Symbol, Kind, Arity) { #Name, COMPILER_KIND_##Kind, Arity },
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
                        else if ((c == '&' || c == '|') && p[1] == c) { t.Text += c; p++; }
                        else if ((c == '+' || c == '-') && (p[1] == c || p[1] == '=')) { t.Text += p[1]; p++; }
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
struct Arg { int Kind; long Value; std::string Text; int Line; }; /* 0 number, 1 name, 2 text, 3 global */
const int FN_NONE = -1;
const int FN_GLOBAL = -2; /* the function is a registered global one, Frame.GlobalFn is its name */
struct Frame {
        int Sprite; int Wait; int Fn; std::vector<Arg> Args; int Line;
        std::string GlobalFn;
        bool IsWait; /* made by Wait: an action that follows can share this frame */
        Frame() : Sprite(-1), Wait(0), Fn(FN_NONE), Line(0), IsWait(false) {}
};
struct State { std::string Name; int Line; std::vector<Frame> Frames; };
struct VarRec { int Slot; std::string Name; long Value; };
/* a condition tree: one comparison, && , || or ! */
enum CondType { C_CMP, C_AND, C_OR, C_NOT, C_RAND };
struct SpriteDef { int Kind; int Sx; int Sy; std::string Path; }; /* Kind 0 ASCII file, 1 image file, 2 image from the container */
struct Cond { CondType Type; Arg L, R; long Op; int A, B; };

/* the comparison that is true when "L op R" is false */
long InverseOp(long op) {
        switch (op) {
        case '<': return 242;
        case '>': return 243;
        case '=': return '!';
        case '!': return '=';
        case 242: return '<';
        default:  return '>'; /* 243 */
        }
}

class Compiler {
public:
        explicit Compiler(const std::vector<Tok> &t) : T(t), P(0), Barrier((size_t)-1), Solid(false), RenderOnly(false) {}

        void ParseFile() {
                ExpectKeyword("object");
                if (Cur().Type != T_IDENT) Fail(Cur().Line, "expected the object name");
                P++; /* the name only documents the file, ObjectCore_Load takes the file name */
                ExpectPunct("{");
                while (!IsPunct("}")) {
                        if (IsKeyword("int")) ParseVar();
                        else if (IsKeyword("states")) ParseStates();
                        else if (IsKeyword("solid")) { P++; Solid = true; if (IsPunct(";")) P++; }
                        else if (IsKeyword("renderonly")) { P++; RenderOnly = true; if (IsPunct(";")) P++; }
                        else Fail(Cur().Line, "expected 'int', 'solid', 'renderonly', 'states' or '}'");
                }
                P++;
                if (Cur().Type != T_END) Fail(Cur().Line, "unexpected text after the object");
                if (States.empty()) Fail(Cur().Line, "the object has no states");
                Validate();
        }

        void Write(std::vector<unsigned char> &o) const {
                o.push_back('M'); o.push_back('T'); o.push_back('O'); o.push_back('1');
                o.push_back((unsigned char)((Solid ? 1 : 0) | (RenderOnly ? 2 : 0)));
                Put16(o, (unsigned)Sprites.size());
                for (size_t i = 0; i < Sprites.size(); i++) {
                        const SpriteDef &sd = Sprites[i];
                        o.push_back((unsigned char)sd.Kind);
                        if (sd.Kind > 0) { Put16(o, (unsigned)sd.Sx); Put16(o, (unsigned)sd.Sy); }
                        o.push_back((unsigned char)sd.Path.size());
                        o.insert(o.end(), sd.Path.begin(), sd.Path.end());
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
                                Put16(o, fr.Fn == FN_NONE ? 0xFFFF : fr.Fn == FN_GLOBAL ? 0xFFFE : (unsigned)fr.Fn);
                                if (fr.Fn == FN_NONE) continue;
                                if (fr.Fn == FN_GLOBAL) {
                                        o.push_back((unsigned char)fr.GlobalFn.size());
                                        o.insert(o.end(), fr.GlobalFn.begin(), fr.GlobalFn.end());
                                }
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
        std::vector<Cond> Conds;
        size_t Barrier; /* index of a frame some jump lands on: nothing may be merged into the frame before it */
        std::vector<SpriteDef> Sprites;
        std::map<std::string, int> SpriteIndex;
        bool Solid;
        bool RenderOnly;

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

        bool AtLabel() const {
                return Cur().Type == T_IDENT && Peek(1).Type == T_PUNCT && Peek(1).Text == ":";
        }

        void ParseStates() {
                P++; /* states */
                ExpectPunct("{");
                while (!IsPunct("}")) {
                        State st; st.Line = Cur().Line;
                        st.Name = ExpectName("a state name");
                        ExpectPunct(":");
                        Barrier = (size_t)-1;
                        for (size_t i = 0; i < States.size(); i++) if (States[i].Name == st.Name) Fail(st.Line, "state '" + st.Name + "' is declared twice");
                        ParseStatements(st, 0);
                        if (st.Frames.empty()) Fail(st.Line, "state '" + st.Name + "' has no frames");
                        if (st.Frames.size() > 65535) Fail(st.Line, "state '" + st.Name + "' has too many frames");
                        if (States.size() >= 65535) Fail(st.Line, "too many states");
                        States.push_back(st);
                }
                P++;
        }

        /* frames, while and if until the closing '}' (or the next label when depth is 0) */
        void ParseStatements(State &st, int depth) {
                while (!IsPunct("}")) {
                        if (Cur().Type == T_END) Fail(Cur().Line, "expected '}'");
                        if (AtLabel()) {
                                if (depth == 0) break;
                                Fail(Cur().Line, "a state label cannot be inside a block");
                        }
                        if (IsKeyword("while")) ParseWhile(st, depth);
                        else if (IsKeyword("if")) ParseIf(st, depth);
                        else if (IsKeyword("else")) Fail(Cur().Line, "'else' without 'if'");
                        else {
                                ParseSimple(st);
                                if (IsPunct(";")) P++;
                        }
                }
        }

        void ParseBlock(State &st, int depth) {
                ExpectPunct("{");
                ParseStatements(st, depth + 1);
                ExpectPunct("}");
        }

        /* ---- control flow: every jump is a frame of JumpFrames/JumpFramesIfVar with wait 0 ---- */
        /* A jump of N frames makes the object land on (its own index + N + 1), see Cscript_JumpFrames. */

        Frame JumpFrame(const char *fn, int line) {
                Frame f; f.Line = line; f.Fn = FindFunction(fn);
                Arg a; a.Kind = 0; a.Value = 0; a.Line = line;
                f.Args.push_back(a);
                return f;
        }

        /* the jumps listed will land on the next frame that is added to the state */
        void PatchHere(State &st, std::vector<size_t> &jumps) {
                if (!jumps.empty()) Barrier = st.Frames.size();
                for (size_t i = 0; i < jumps.size(); i++) {
                        st.Frames[jumps[i]].Args[0].Value = (long)st.Frames.size() - (long)jumps[i] - 1;
                }
                jumps.clear();
        }

        void ParseIf(State &st, int depth) {
                int line = Cur().Line;
                P++; /* if */
                std::vector<size_t> whenFalse;
                ParseCondition(st, false, whenFalse);
                ParseBlock(st, depth);
                if (IsKeyword("else")) {
                        P++;
                        std::vector<size_t> toEnd;
                        toEnd.push_back(st.Frames.size());
                        st.Frames.push_back(JumpFrame("JumpFrames", line));
                        PatchHere(st, whenFalse);
                        if (IsKeyword("if")) ParseIf(st, depth);
                        else ParseBlock(st, depth);
                        PatchHere(st, toEnd);
                }
                else {
                        PatchHere(st, whenFalse);
                }
        }

        void ParseWhile(State &st, int depth) {
                int line = Cur().Line;
                P++; /* while */
                size_t start = st.Frames.size();
                std::vector<size_t> whenFalse;
                ParseCondition(st, false, whenFalse);
                ParseBlock(st, depth);
                Frame back = JumpFrame("JumpFrames", line);
                back.Args[0].Value = (long)start - (long)st.Frames.size() - 1;
                st.Frames.push_back(back);
                PatchHere(st, whenFalse);
        }

        /* ( condition ): adds the frames that jump (the frames are added to "jumps") when the
           condition is jumpWhen, and continue with the next frame otherwise. */
        void ParseCondition(State &st, bool jumpWhen, std::vector<size_t> &jumps) {
                int line = Cur().Line;
                ExpectPunct("(");
                Conds.clear();
                int root = ParseOr();
                ExpectPunct(")");
                EmitCond(st, root, jumpWhen, jumps, line);
        }

        int AddCond(CondType type, int a, int b) {
                Cond c; c.Type = type; c.Op = 0; c.A = a; c.B = b;
                c.L.Kind = 0; c.L.Value = 0; c.L.Line = 0;
                c.R = c.L;
                Conds.push_back(c);
                return (int)Conds.size() - 1;
        }

        int ParseOr() {
                int a = ParseAnd();
                while (IsPunct("||")) { P++; int b = ParseAnd(); a = AddCond(C_OR, a, b); }
                return a;
        }

        int ParseAnd() {
                int a = ParseNot();
                while (IsPunct("&&")) { P++; int b = ParseNot(); a = AddCond(C_AND, a, b); }
                return a;
        }

        int ParseNot() {
                if (IsPunct("!")) { P++; int a = ParseNot(); return AddCond(C_NOT, a, -1); }
                return ParsePrimary();
        }

        Arg ParseOperand() {
                Arg a; a.Value = 0; a.Line = Cur().Line; a.Kind = 0;
                if (Cur().Type == T_INT) { a.Value = Cur().Value; P++; }
                else if (IsKeyword("global")) { P++; a.Kind = 3; a.Text = ExpectName("a global name"); }
                else if (Cur().Type == T_IDENT) { a.Kind = 1; a.Text = ExpectName("a var name"); }
                else Fail(Cur().Line, "expected a var or a number");
                return a;
        }

        int ParsePrimary() {
                if (IsPunct("(")) {
                        P++;
                        int e = ParseOr();
                        ExpectPunct(")");
                        return e;
                }
                if (IsKeyword("random") && Peek(1).Type == T_PUNCT && Peek(1).Text == "(") {
                        P += 2;
                        if (Cur().Type != T_INT || Cur().Value < 0 || Cur().Value > 100) Fail(Cur().Line, "random(...) needs the chance in percent, from 0 to 100");
                        int id = AddCond(C_RAND, -1, -1);
                        Conds[id].Op = T[P++].Value;
                        ExpectPunct(")");
                        return id;
                }
                int id = AddCond(C_CMP, -1, -1);
                Arg l = ParseOperand();
                long op = '!'; /* a lone operand is true when it is not 0 */
                Arg r; r.Kind = 0; r.Value = 0; r.Line = l.Line;
                if (Cur().Type == T_PUNCT) {
                        const std::string &t = Cur().Text;
                        int found = 1;
                        if (t == "<") op = '<';
                        else if (t == ">") op = '>';
                        else if (t == "=" || t == "==") op = '=';
                        else if (t == "!=") op = '!';
                        else if (t == ">=") op = 242;
                        else if (t == "<=") op = 243;
                        else found = 0;
                        if (found) { P++; r = ParseOperand(); }
                }
                Conds[id].L = l; Conds[id].R = r; Conds[id].Op = op;
                return id;
        }

        /* jumps to "out" when node is jumpWhen, falls through otherwise */
        void EmitCond(State &st, int id, bool jumpWhen, std::vector<size_t> &out, int line) {
                Cond c = Conds[id]; /* copy: the vector does not change while emitting, but keep it simple */
                switch (c.Type) {
                case C_CMP: {
                        Frame f = JumpFrame("JumpFramesIfVar", line);
                        Arg op; op.Kind = 0; op.Line = line; op.Value = jumpWhen ? c.Op : InverseOp(c.Op);
                        f.Args.push_back(c.L);
                        f.Args.push_back(op);
                        f.Args.push_back(c.R);
                        out.push_back(st.Frames.size());
                        st.Frames.push_back(f);
                        break;
                }
                case C_NOT:
                        EmitCond(st, c.A, !jumpWhen, out, line);
                        break;
                case C_RAND: {
                        /* JumpFramesIfRandom jumps when rand() % max <= chance: p percent = chance p - 1 of 100 */
                        Frame f = JumpFrame("JumpFramesIfRandom", line);
                        Arg chance; chance.Kind = 0; chance.Value = c.Op - 1; chance.Line = line;
                        Arg max; max.Kind = 0; max.Value = 100; max.Line = line;
                        f.Args.push_back(chance);
                        f.Args.push_back(max);
                        if (jumpWhen) {
                                out.push_back(st.Frames.size());
                        }
                        else {
                                /* hit: skip the next frame, miss: that frame jumps out */
                                f.Args[0].Value = 1;
                        }
                        st.Frames.push_back(f);
                        if (!jumpWhen) {
                                out.push_back(st.Frames.size());
                                st.Frames.push_back(JumpFrame("JumpFrames", line));
                        }
                        break;
                }
                case C_AND:
                        if (!jumpWhen) { /* jump if any is false */
                                EmitCond(st, c.A, false, out, line);
                                EmitCond(st, c.B, false, out, line);
                        }
                        else { /* jump if both are true: a false A skips B and the jump */
                                std::vector<size_t> skip;
                                EmitCond(st, c.A, false, skip, line);
                                EmitCond(st, c.B, true, out, line);
                                PatchHere(st, skip);
                        }
                        break;
                case C_OR:
                        if (jumpWhen) { /* jump if any is true */
                                EmitCond(st, c.A, true, out, line);
                                EmitCond(st, c.B, true, out, line);
                        }
                        else { /* jump if both are false: a true A skips B and the jump */
                                std::vector<size_t> skip;
                                EmitCond(st, c.A, true, skip, line);
                                EmitCond(st, c.B, false, out, line);
                                PatchHere(st, skip);
                        }
                        break;
                }
        }

        int SpriteId(int kind, const std::string &path, int sx, int sy, int line) {
                if (path.empty()) return -1;
                if (path.size() > 255) Fail(line, "sprite path is too long");
                std::ostringstream key;
                key << kind << ':' << sx << ':' << sy << ':' << path;
                std::map<std::string, int>::iterator it = SpriteIndex.find(key.str());
                if (it != SpriteIndex.end()) return it->second;
                if (Sprites.size() >= 65535) Fail(line, "too many sprites");
                SpriteDef d; d.Kind = kind; d.Sx = sx; d.Sy = sy; d.Path = path;
                int id = (int)Sprites.size();
                Sprites.push_back(d);
                SpriteIndex[key.str()] = id;
                return id;
        }

        int FindFunction(const std::string &name) const {
                for (int i = 0; i < FunctionCount; i++) if (IEq(name, Functions[i].Name)) return i;
                return -1;
        }

        /* An action runs when the wait of the frame ends, so it can share the frame of the Wait
           right before it, unless some jump lands between them. */
        void AddAction(State &st, const Frame &f) {
                size_t n = st.Frames.size();
                if (n > 0 && Barrier != n && st.Frames[n - 1].IsWait && st.Frames[n - 1].Fn == FN_NONE) {
                        st.Frames[n - 1].Fn = f.Fn;
                        st.Frames[n - 1].GlobalFn = f.GlobalFn;
                        st.Frames[n - 1].Args = f.Args;
                        return;
                }
                st.Frames.push_back(f);
        }

        int ParseWaitTime() {
                if (Cur().Type != T_INT || Cur().Value < 0 || Cur().Value > 65535) Fail(Cur().Line, "expected the wait time (0 to 65535)");
                return (int)T[P++].Value;
        }

        Frame NamedCall(const char *fn, int line, const std::string &name) {
                Frame f; f.Line = line; f.Fn = FindFunction(fn);
                Arg a; a.Kind = 1; a.Value = 0; a.Line = line; a.Text = name;
                f.Args.push_back(a);
                return f;
        }

        static bool IsAssignText(const std::string &t) { return t == "++" || t == "--" || t == "+=" || t == "-=" || t == "="; }
        bool IsAssignOp() const { return Cur().Type == T_PUNCT && IsAssignText(Cur().Text); }

        /* x++ x-- x += v x -= v x = v : the target token is already read, kind 1 = object var, 3 = global var */
        void ParseAssignment(State &st, int line, const std::string &var, int kind) {
                std::string op = T[P++].Text;
                Frame f = NamedCall(op == "++" ? "VarInc" : op == "--" ? "VarDec" : op == "+=" ? "VarAdd" : op == "-=" ? "VarSub" : "SetVar", line, var);
                f.Args[0].Kind = kind;
                if (op != "++" && op != "--") f.Args.push_back(ParseOperand());
                AddAction(st, f);
        }

        /* Show "sprite" + Wait n, Wait n, Goto, Stop, x++ x-- x += v x -= v x = v and Function(args) */
        void ParseSimple(State &st) {
                int line = Cur().Line;
                if (IsKeyword("show")) {
                        P++;
                        Frame f; f.Line = line; f.IsWait = true;
                        if (Cur().Type == T_STR) { f.Sprite = SpriteId(0, Cur().Text, 1, 1, Cur().Line); P++; }
                        else if (IsKeyword("image") || IsKeyword("container")) {
                                int kind = IsKeyword("image") ? 1 : 2;
                                P++;
                                if (Cur().Type != T_STR) Fail(Cur().Line, "expected the \"file\" of the sprite");
                                std::string path = Cur().Text;
                                int pathLine = Cur().Line;
                                P++;
                                int scale[2];
                                for (int k = 0; k < 2; k++) {
                                        if (Cur().Type != T_INT || Cur().Value < 1 || Cur().Value > 65535) Fail(Cur().Line, "expected the sprite width and height after the file");
                                        scale[k] = (int)T[P++].Value;
                                }
                                f.Sprite = SpriteId(kind, path, scale[0], scale[1], pathLine);
                        }
                        else if (IsKeyword("none")) { P++; }
                        else Fail(Cur().Line, "expected a \"sprite\", image \"file\" w h, container \"file\" w h or none after Show");
                        if (IsPunct(";")) P++;
                        if (!IsKeyword("wait")) Fail(line, "Show must be directly followed by Wait");
                        P++;
                        f.Wait = ParseWaitTime();
                        st.Frames.push_back(f);
                }
                else if (IsKeyword("wait")) {
                        P++;
                        Frame f; f.Line = line; f.IsWait = true;
                        f.Wait = ParseWaitTime();
                        st.Frames.push_back(f);
                }
                else if (IsKeyword("goto")) {
                        P++;
                        AddAction(st, NamedCall("SetState", line, ExpectName("a state name")));
                }
                else if (IsKeyword("stop")) {
                        P++;
                        Frame f; f.Line = line; f.Fn = FindFunction("RemoveObject");
                        AddAction(st, f);
                }
                else if (IsKeyword("global")) {
                        P++;
                        std::string name = ExpectName("a global name");
                        if (IsAssignOp()) ParseAssignment(st, line, name, 3);
                        else {
                                Frame f; f.Line = line; f.Fn = FN_GLOBAL; f.GlobalFn = name;
                                ParseCallArgs(f);
                                AddAction(st, f);
                        }
                }
                else if (Cur().Type == T_IDENT) {
                        if (Peek(1).Type == T_PUNCT && IsAssignText(Peek(1).Text)) {
                                std::string var = ExpectName("a var name");
                                ParseAssignment(st, line, var, 1);
                        }
                        else {
                                Frame f; f.Line = line;
                                ParseAction(f);
                                AddAction(st, f);
                        }
                }
                else Fail(line, "expected a statement: Show, Wait, a function call, x++, x = 1, while, if, Goto or Stop");
        }

        /* optional ( arg, arg ... ) */
        void ParseCallArgs(Frame &f) {
                if (IsPunct("(")) {
                        P++;
                        while (!IsPunct(")")) {
                                if (!f.Args.empty()) ExpectPunct(",");
                                f.Args.push_back(ParseArg());
                        }
                        P++;
                }
        }

        void ParseAction(Frame &f) {
                int line = Cur().Line;
                std::string name = Cur().Text;
                f.Fn = FindFunction(name);
                if (f.Fn < 0) Fail(line, "unknown function '" + name + "'");
                std::string lower = name;
                for (size_t i = 0; i < lower.size(); i++) lower[i] = (char)tolower((unsigned char)lower[i]);
                if (lower.compare(0, 10, "jumpframes") == 0 || lower.compare(0, 13, "jumpnextframe") == 0)
                        Fail(line, "'" + name + "' counts frames, use if / while instead");
                P++;
                ParseCallArgs(f);
                if ((int)f.Args.size() != Functions[f.Fn].Arity) {
                        std::ostringstream o;
                        o << Functions[f.Fn].Name << " needs " << Functions[f.Fn].Arity << " argument(s), got " << f.Args.size();
                        Fail(line, o.str());
                }
                if (Functions[f.Fn].Kind == COMPILER_KIND_OBJOBJ && f.Args[0].Kind != 3)
                        Fail(line, std::string(Functions[f.Fn].Name) + " needs a global object as its first argument, like global Name");
        }

        Arg ParseArg() {
                Arg a; a.Value = 0; a.Line = Cur().Line;
                const Tok &t = Cur();
                if (t.Type == T_INT) { a.Kind = 0; a.Value = t.Value; P++; }
                else if (t.Type == T_STR) {
                        if (t.Text.size() > 255) Fail(t.Line, "text is too long");
                        a.Kind = 2; a.Text = t.Text; P++;
                }
                else if (t.Type == T_IDENT && IsKeyword("global")) { P++; a.Kind = 3; a.Text = ExpectName("a global name"); }
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

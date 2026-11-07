/* CscriptCompiler.c, see CscriptCompiler.h
   Plain C: growable arrays (Vec) instead of the STL and setjmp/longjmp instead of exceptions.
   Everything the compiler allocates is reachable from the Compiler struct, so one call frees it
   after a success or an error. Names and texts are not copied, tokens point into the source text.
*/
#include <MT2D/ObjectCore/Parser/CscriptCompiler.h>

#if defined(MT2D_USING_CSCRIPT_PARSER)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include <MT2D/ObjectCore/CscriptFunctions.h>
#include <MT2D/ObjectCore/ObjectLoader.h>

/* growable array: p = elements, n = used, cap = allocated */
typedef struct { void *p; size_t n, cap; } Vec;

#define NAME_MAX_LEN 18 /* MT2D_VAR_NAME_MAX - 1 */

typedef struct { const char *Name; int Kind; int Arity; } FnInfo;
#define COMPILER_KIND_OBJ 0
#define COMPILER_KIND_VARS 1
#define COMPILER_KIND_OBJOBJ 2
#define COMPILER_FN_ENTRY(Name, Symbol, Kind, Arity) { #Name, COMPILER_KIND_##Kind, Arity },
static const FnInfo Functions[] = { MT2D_CSCRIPT_FUNCTIONS(COMPILER_FN_ENTRY) };
#undef COMPILER_FN_ENTRY
#define FunctionCount ((int)(sizeof(Functions) / sizeof(Functions[0])))

static const char *BuiltinVars[] = { "Pos_X", "Pos_Y", "Size_X", "Size_Y", "Acel_X", "Acel_Y" };

enum { T_END, T_IDENT, T_INT, T_STR, T_PUNCT };
typedef struct { int Type; const char *Text; int Len; long Value; int Line; } Tok;

/* argument kinds: 0 number, 1 name, 2 text, 3 global */
typedef struct { int Kind; long Value; const char *Text; int Len; int Line; } Arg;

#define FN_NONE (-1)
#define FN_GLOBAL (-2) /* the function is a registered global one, Frame.GlobalFn is its name */
typedef struct {
        int Sprite; int Wait; int Fn;
        int ArgStart; int ArgCount; /* range of Compiler.ArgPool */
        int Line;
        const char *GlobalFn; int GlobalLen;
        int IsWait;    /* made by Wait: an action that follows can share this frame */
        int NextPatch; /* intrusive list of jump frames waiting to be patched, -1 = end */
} Frame;

typedef struct { const char *Name; int Len; int Line; Vec Frames; } State;
typedef struct { int Slot; const char *Name; int Len; long Value; } VarRec;
typedef struct { int Kind; int Sx; int Sy; const char *Path; int Len; } SpriteDef; /* Kind 0 ASCII file, 1 image file, 2 image from the container */

/* a condition tree: one comparison, && , || , ! or random */
enum { C_CMP, C_AND, C_OR, C_NOT, C_RAND };
typedef struct { int Type; Arg L, R; long Op; int A, B; } Cond;

typedef struct Compiler {
        jmp_buf Jmp;
        char Err[256];
        Vec Toks;      /* Tok */
        int P;
        Vec Vars;      /* VarRec */
        Vec States;    /* State */
        Vec ArgPool;   /* Arg */
        Vec Conds;     /* Cond */
        Vec Sprites;   /* SpriteDef */
        Vec Out;       /* unsigned char */
        long Barrier;  /* index of a frame some jump lands on: nothing may be merged into the frame before it */
        int Solid;
        int RenderOnly;
} Compiler;

/* ---- helpers --------------------------------------------------------------- */
static void Fail(Compiler *c, int line, const char *fmt, ...) {
        va_list ap;
        int n = 0;
        if (line > 0) n = snprintf(c->Err, sizeof(c->Err), "line %d: ", line);
        va_start(ap, fmt);
        vsnprintf(c->Err + n, sizeof(c->Err) - (size_t)n, fmt, ap);
        va_end(ap);
        longjmp(c->Jmp, 1);
}

static void *VecPush(Compiler *c, Vec *v, size_t size) {
        if (v->n == v->cap) {
                size_t cap = v->cap ? v->cap * 2 : 8;
                void *p = realloc(v->p, cap * size);
                if (!p) Fail(c, 0, "out of memory");
                v->p = p;
                v->cap = cap;
        }
        memset((char*)v->p + v->n * size, 0, size);
        return (char*)v->p + (v->n++) * size;
}

#define AT(vec, type, i) (((type*)(vec).p)[i])
#define COUNT(vec) ((int)(vec).n)

static int IEqN(const char *a, int n, const char *b) {
        int i;
        if ((int)strlen(b) != n) return 0;
        for (i = 0; i < n; i++) {
                if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return 0;
        }
        return 1;
}

static int IPrefixN(const char *a, int n, const char *prefix) {
        int len = (int)strlen(prefix);
        return n >= len && IEqN(a, len, prefix);
}

static int SameN(const char *a, int an, const char *b, int bn) {
        return an == bn && memcmp(a, b, (size_t)an) == 0;
}

/* ---- tokenizer ---------------------------------------------------------- */
static void AddTok(Compiler *c, int type, const char *text, int len, long value, int line) {
        Tok *t = (Tok*)VecPush(c, &c->Toks, sizeof(Tok));
        t->Type = type; t->Text = text; t->Len = len; t->Value = value; t->Line = line;
}

static void Tokenize(Compiler *c, const char *p) {
        int line = 1;
        while (*p) {
                char ch = *p;
                if (ch == '\n') { line++; p++; }
                else if (isspace((unsigned char)ch)) { p++; }
                else if (ch == '/' && p[1] == '/') { while (*p && *p != '\n') p++; }
                else if (ch == '/' && p[1] == '*') {
                        p += 2;
                        while (*p && !(p[0] == '*' && p[1] == '/')) { if (*p == '\n') line++; p++; }
                        if (!*p) Fail(c, line, "comment is never closed");
                        p += 2;
                }
                else if (isalpha((unsigned char)ch) || ch == '_') {
                        const char *start = p;
                        while (isalnum((unsigned char)*p) || *p == '_') p++;
                        AddTok(c, T_IDENT, start, (int)(p - start), 0, line);
                }
                else if (isdigit((unsigned char)ch) || (ch == '-' && isdigit((unsigned char)p[1]))) {
                        const char *start = p;
                        char digits[16];
                        long value;
                        int len;
                        if (*p == '-') p++;
                        while (isdigit((unsigned char)*p)) p++;
                        len = (int)(p - start);
                        if (len > 11) Fail(c, line, "number is too big: %.*s", len, start);
                        memcpy(digits, start, (size_t)len);
                        digits[len] = 0;
                        value = atol(digits);
                        if (value > 2147483647L || value < -2147483647L - 1) Fail(c, line, "number is too big: %.*s", len, start);
                        AddTok(c, T_INT, start, len, value, line);
                }
                else if (ch == '"') {
                        const char *start;
                        p++;
                        start = p;
                        while (*p && *p != '"' && *p != '\n') p++;
                        if (*p != '"') Fail(c, line, "text is never closed");
                        AddTok(c, T_STR, start, (int)(p - start), 0, line);
                        p++;
                }
                else {
                        int len = 1;
                        if ((ch == '=' || ch == '!' || ch == '<' || ch == '>') && p[1] == '=') len = 2;
                        else if ((ch == '&' || ch == '|') && p[1] == ch) len = 2;
                        else if ((ch == '+' || ch == '-') && (p[1] == ch || p[1] == '=')) len = 2;
                        else if (!strchr("{}(),;:<>=!", ch)) Fail(c, line, "unexpected character '%c'", ch);
                        AddTok(c, T_PUNCT, p, len, 0, line);
                        p += len;
                }
        }
        AddTok(c, T_END, p, 0, 0, line);
}

/* ---- token access -------------------------------------------------------- */
static Tok *Cur(Compiler *c) { return &AT(c->Toks, Tok, c->P); }
static Tok *Peek(Compiler *c, int n) {
        int i = c->P + n;
        if (i >= COUNT(c->Toks)) i = COUNT(c->Toks) - 1;
        return &AT(c->Toks, Tok, i);
}
static int TokIs(const Tok *t, const char *s) { return t->Type == T_PUNCT && (int)strlen(s) == t->Len && memcmp(t->Text, s, (size_t)t->Len) == 0; }
static int IsPunct(Compiler *c, const char *s) { return TokIs(Cur(c), s); }
static int IsKeyword(Compiler *c, const char *s) { return Cur(c)->Type == T_IDENT && IEqN(Cur(c)->Text, Cur(c)->Len, s); }

static void ExpectPunct(Compiler *c, const char *s) {
        if (!IsPunct(c, s)) Fail(c, Cur(c)->Line, "expected '%s'", s);
        c->P++;
}

static void ExpectKeyword(Compiler *c, const char *s) {
        if (!IsKeyword(c, s)) Fail(c, Cur(c)->Line, "expected '%s'", s);
        c->P++;
}

/* an identifier of at most 18 characters, returned as text + length */
static const Tok *ExpectName(Compiler *c, const char *what) {
        Tok *t = Cur(c);
        if (t->Type != T_IDENT) Fail(c, t->Line, "expected %s", what);
        if (t->Len > NAME_MAX_LEN) Fail(c, t->Line, "name '%.*s' is longer than 18 characters", t->Len, t->Text);
        c->P++;
        return t;
}

static void Put8(Compiler *c, unsigned v) { *(unsigned char*)VecPush(c, &c->Out, 1) = (unsigned char)v; }
static void Put16(Compiler *c, unsigned v) { Put8(c, v & 0xFF); Put8(c, (v >> 8) & 0xFF); }
static void Put32(Compiler *c, long v) {
        unsigned long u = (unsigned long)v;
        int i;
        for (i = 0; i < 4; i++) Put8(c, (unsigned)((u >> (8 * i)) & 0xFF));
}
static void PutBytes(Compiler *c, const char *s, int n) {
        int i;
        for (i = 0; i < n; i++) Put8(c, (unsigned char)s[i]);
}

/* the comparison that is true when "L op R" is false */
static long InverseOp(long op) {
        switch (op) {
        case '<': return 242;
        case '>': return 243;
        case '=': return '!';
        case '!': return '=';
        case 242: return '<';
        default:  return '>'; /* 243 */
        }
}

static int FindFunction(const char *name, int len) {
        int i;
        for (i = 0; i < FunctionCount; i++) if (IEqN(name, len, Functions[i].Name)) return i;
        return -1;
}

static int FindFunctionC(const char *name) { return FindFunction(name, (int)strlen(name)); }

/* ---- frames --------------------------------------------------------------- */
static Frame NewFrame(int line) {
        Frame f;
        memset(&f, 0, sizeof(f));
        f.Sprite = -1; f.Fn = FN_NONE; f.Line = line; f.NextPatch = -1;
        return f;
}

static void PushFrame(Compiler *c, State *st, Frame f) { *(Frame*)VecPush(c, &st->Frames, sizeof(Frame)) = f; }

static Frame *FrameAt(State *st, int i) { return &AT(st->Frames, Frame, i); }

static void PushArg(Compiler *c, Frame *f, Arg a) {
        if (f->ArgCount >= 255) Fail(c, a.Line, "too many arguments");
        *(Arg*)VecPush(c, &c->ArgPool, sizeof(Arg)) = a;
        f->ArgCount++;
}

static Arg NumArg(long value, int line) {
        Arg a;
        memset(&a, 0, sizeof(a));
        a.Value = value; a.Line = line;
        return a;
}

static Arg *FrameArg(Compiler *c, const Frame *f, int i) { return &AT(c->ArgPool, Arg, f->ArgStart + i); }

/* ---- control flow: every jump is a frame of JumpFrames/JumpFramesIfVar with wait 0 ---- */
/* A jump of N frames makes the object land on (its own index + N + 1), see Cscript_JumpFrames. */

static Frame JumpFrame(Compiler *c, const char *fn, int line) {
        Frame f = NewFrame(line);
        f.Fn = FindFunctionC(fn);
        f.ArgStart = COUNT(c->ArgPool);
        PushArg(c, &f, NumArg(0, line));
        return f;
}

/* adds the frame at index "frame" to a list of jumps waiting for their target */
static void ListAdd(State *st, int *list, int frame) {
        FrameAt(st, frame)->NextPatch = *list;
        *list = frame;
}

/* the jumps listed will land on the next frame that is added to the state */
static void PatchHere(Compiler *c, State *st, int *list) {
        int n = COUNT(st->Frames);
        int i = *list;
        if (i >= 0) c->Barrier = n;
        while (i >= 0) {
                Frame *f = FrameAt(st, i);
                FrameArg(c, f, 0)->Value = (long)n - (long)i - 1;
                i = f->NextPatch;
                f->NextPatch = -1;
        }
        *list = -1;
}

/* An action runs when the wait of the frame ends, so it can share the frame of the Wait
   right before it, unless some jump lands between them. */
static void AddAction(Compiler *c, State *st, Frame f) {
        int n = COUNT(st->Frames);
        if (n > 0 && c->Barrier != n && FrameAt(st, n - 1)->IsWait && FrameAt(st, n - 1)->Fn == FN_NONE) {
                Frame *last = FrameAt(st, n - 1);
                last->Fn = f.Fn;
                last->GlobalFn = f.GlobalFn;
                last->GlobalLen = f.GlobalLen;
                last->ArgStart = f.ArgStart;
                last->ArgCount = f.ArgCount;
                return;
        }
        PushFrame(c, st, f);
}

/* ---- conditions ------------------------------------------------------------ */
static int AddCond(Compiler *c, int type, int a, int b) {
        Cond *k = (Cond*)VecPush(c, &c->Conds, sizeof(Cond));
        k->Type = type; k->A = a; k->B = b;
        return COUNT(c->Conds) - 1;
}

static Arg ParseOperand(Compiler *c) {
        Arg a;
        const Tok *t;
        memset(&a, 0, sizeof(a));
        a.Line = Cur(c)->Line;
        if (Cur(c)->Type == T_INT) { a.Value = Cur(c)->Value; c->P++; }
        else if (IsKeyword(c, "global")) { c->P++; t = ExpectName(c, "a global name"); a.Kind = 3; a.Text = t->Text; a.Len = t->Len; }
        else if (Cur(c)->Type == T_IDENT) { t = ExpectName(c, "a var name"); a.Kind = 1; a.Text = t->Text; a.Len = t->Len; }
        else Fail(c, Cur(c)->Line, "expected a var or a number");
        return a;
}

static int ParseOr(Compiler *c);

static int ParsePrimary(Compiler *c) {
        int id;
        Arg l, r;
        long op = '!'; /* a lone operand is true when it is not 0 */
        if (IsPunct(c, "(")) {
                int e;
                c->P++;
                e = ParseOr(c);
                ExpectPunct(c, ")");
                return e;
        }
        if (IsKeyword(c, "random") && TokIs(Peek(c, 1), "(")) {
                c->P += 2;
                if (Cur(c)->Type != T_INT || Cur(c)->Value < 0 || Cur(c)->Value > 100)
                        Fail(c, Cur(c)->Line, "random(...) needs the chance in percent, from 0 to 100");
                id = AddCond(c, C_RAND, -1, -1);
                AT(c->Conds, Cond, id).Op = Cur(c)->Value;
                c->P++;
                ExpectPunct(c, ")");
                return id;
        }
        id = AddCond(c, C_CMP, -1, -1);
        l = ParseOperand(c);
        r = NumArg(0, l.Line);
        if (Cur(c)->Type == T_PUNCT) {
                const Tok *t = Cur(c);
                int found = 1;
                if (TokIs(t, "<")) op = '<';
                else if (TokIs(t, ">")) op = '>';
                else if (TokIs(t, "=") || TokIs(t, "==")) op = '=';
                else if (TokIs(t, "!=")) op = '!';
                else if (TokIs(t, ">=")) op = 242;
                else if (TokIs(t, "<=")) op = 243;
                else found = 0;
                if (found) { c->P++; r = ParseOperand(c); }
        }
        AT(c->Conds, Cond, id).L = l;
        AT(c->Conds, Cond, id).R = r;
        AT(c->Conds, Cond, id).Op = op;
        return id;
}

static int ParseNot(Compiler *c) {
        if (IsPunct(c, "!")) {
                int a;
                c->P++;
                a = ParseNot(c);
                return AddCond(c, C_NOT, a, -1);
        }
        return ParsePrimary(c);
}

static int ParseAnd(Compiler *c) {
        int a = ParseNot(c);
        while (IsPunct(c, "&&")) { int b; c->P++; b = ParseNot(c); a = AddCond(c, C_AND, a, b); }
        return a;
}

static int ParseOr(Compiler *c) {
        int a = ParseAnd(c);
        while (IsPunct(c, "||")) { int b; c->P++; b = ParseAnd(c); a = AddCond(c, C_OR, a, b); }
        return a;
}

/* jumps to "out" when node is jumpWhen, falls through otherwise */
static void EmitCond(Compiler *c, State *st, int id, int jumpWhen, int *out, int line) {
        Cond k = AT(c->Conds, Cond, id); /* a copy: the arrays may move while frames are added */
        switch (k.Type) {
        case C_CMP: {
                Frame f = JumpFrame(c, "JumpFramesIfVar", line);
                PushArg(c, &f, k.L);
                PushArg(c, &f, NumArg(jumpWhen ? k.Op : InverseOp(k.Op), line));
                PushArg(c, &f, k.R);
                PushFrame(c, st, f);
                ListAdd(st, out, COUNT(st->Frames) - 1);
                break;
        }
        case C_NOT:
                EmitCond(c, st, k.A, !jumpWhen, out, line);
                break;
        case C_RAND: {
                /* JumpFramesIfRandom jumps when rand() % max <= chance: p percent = chance p - 1 of 100 */
                Frame f = JumpFrame(c, "JumpFramesIfRandom", line);
                PushArg(c, &f, NumArg(k.Op - 1, line));
                PushArg(c, &f, NumArg(100, line));
                if (!jumpWhen) FrameArg(c, &f, 0)->Value = 1; /* hit: skip the next frame, miss: that frame jumps out */
                PushFrame(c, st, f);
                if (jumpWhen) {
                        ListAdd(st, out, COUNT(st->Frames) - 1);
                }
                else {
                        PushFrame(c, st, JumpFrame(c, "JumpFrames", line));
                        ListAdd(st, out, COUNT(st->Frames) - 1);
                }
                break;
        }
        case C_AND:
                if (!jumpWhen) { /* jump if any is false */
                        EmitCond(c, st, k.A, 0, out, line);
                        EmitCond(c, st, k.B, 0, out, line);
                }
                else { /* jump if both are true: a false A skips B and the jump */
                        int skip = -1;
                        EmitCond(c, st, k.A, 0, &skip, line);
                        EmitCond(c, st, k.B, 1, out, line);
                        PatchHere(c, st, &skip);
                }
                break;
        case C_OR:
                if (jumpWhen) { /* jump if any is true */
                        EmitCond(c, st, k.A, 1, out, line);
                        EmitCond(c, st, k.B, 1, out, line);
                }
                else { /* jump if both are false: a true A skips B and the jump */
                        int skip = -1;
                        EmitCond(c, st, k.A, 1, &skip, line);
                        EmitCond(c, st, k.B, 0, out, line);
                        PatchHere(c, st, &skip);
                }
                break;
        }
}

/* ( condition ): adds the frames that jump (they are added to the list "jumps") when the
   condition is jumpWhen, and continue with the next frame otherwise. */
static void ParseCondition(Compiler *c, State *st, int jumpWhen, int *jumps) {
        int line = Cur(c)->Line;
        int root;
        ExpectPunct(c, "(");
        c->Conds.n = 0;
        root = ParseOr(c);
        ExpectPunct(c, ")");
        EmitCond(c, st, root, jumpWhen, jumps, line);
}

static void ParseStatements(Compiler *c, State *st, int depth);

static void ParseBlock(Compiler *c, State *st, int depth) {
        ExpectPunct(c, "{");
        ParseStatements(c, st, depth + 1);
        ExpectPunct(c, "}");
}

static void ParseIf(Compiler *c, State *st, int depth) {
        int line = Cur(c)->Line;
        int whenFalse = -1;
        c->P++; /* if */
        ParseCondition(c, st, 0, &whenFalse);
        ParseBlock(c, st, depth);
        if (IsKeyword(c, "else")) {
                int toEnd = -1;
                c->P++;
                PushFrame(c, st, JumpFrame(c, "JumpFrames", line));
                ListAdd(st, &toEnd, COUNT(st->Frames) - 1);
                PatchHere(c, st, &whenFalse);
                if (IsKeyword(c, "if")) ParseIf(c, st, depth);
                else ParseBlock(c, st, depth);
                PatchHere(c, st, &toEnd);
        }
        else {
                PatchHere(c, st, &whenFalse);
        }
}

static void ParseWhile(Compiler *c, State *st, int depth) {
        int line = Cur(c)->Line;
        int start = COUNT(st->Frames);
        int whenFalse = -1;
        Frame back;
        c->P++; /* while */
        ParseCondition(c, st, 0, &whenFalse);
        ParseBlock(c, st, depth);
        back = JumpFrame(c, "JumpFrames", line);
        FrameArg(c, &back, 0)->Value = (long)start - (long)COUNT(st->Frames) - 1;
        PushFrame(c, st, back);
        PatchHere(c, st, &whenFalse);
}

/* ---- simple statements ------------------------------------------------------ */
static int SpriteId(Compiler *c, int kind, const char *path, int len, int sx, int sy, int line) {
        int i;
        SpriteDef *d;
        if (len == 0) return -1;
        if (len > 255) Fail(c, line, "sprite path is too long");
        for (i = 0; i < COUNT(c->Sprites); i++) {
                SpriteDef *s = &AT(c->Sprites, SpriteDef, i);
                if (s->Kind == kind && s->Sx == sx && s->Sy == sy && SameN(s->Path, s->Len, path, len)) return i;
        }
        if (COUNT(c->Sprites) >= 65535) Fail(c, line, "too many sprites");
        d = (SpriteDef*)VecPush(c, &c->Sprites, sizeof(SpriteDef));
        d->Kind = kind; d->Sx = sx; d->Sy = sy; d->Path = path; d->Len = len;
        return COUNT(c->Sprites) - 1;
}

static int ParseWaitTime(Compiler *c) {
        int v;
        if (Cur(c)->Type != T_INT || Cur(c)->Value < 0 || Cur(c)->Value > 65535) Fail(c, Cur(c)->Line, "expected the wait time (0 to 65535)");
        v = (int)Cur(c)->Value;
        c->P++;
        return v;
}

static Frame NamedCall(Compiler *c, const char *fn, int line, const char *name, int len) {
        Frame f = NewFrame(line);
        Arg a = NumArg(0, line);
        f.Fn = FindFunctionC(fn);
        f.ArgStart = COUNT(c->ArgPool);
        a.Kind = 1; a.Text = name; a.Len = len;
        PushArg(c, &f, a);
        return f;
}

static int IsAssignTok(const Tok *t) { return TokIs(t, "++") || TokIs(t, "--") || TokIs(t, "+=") || TokIs(t, "-=") || TokIs(t, "="); }

/* x++ x-- x += v x -= v x = v : the target token is already read, kind 1 = object var, 3 = global var */
static void ParseAssignment(Compiler *c, State *st, int line, const char *var, int len, int kind) {
        const Tok *op = Cur(c);
        const char *fn = TokIs(op, "++") ? "VarInc" : TokIs(op, "--") ? "VarDec" : TokIs(op, "+=") ? "VarAdd" : TokIs(op, "-=") ? "VarSub" : "SetVar";
        int needsValue = !(TokIs(op, "++") || TokIs(op, "--"));
        Frame f;
        c->P++;
        f = NamedCall(c, fn, line, var, len);
        FrameArg(c, &f, 0)->Kind = kind;
        if (needsValue) PushArg(c, &f, ParseOperand(c));
        AddAction(c, st, f);
}

static Arg ParseArg(Compiler *c) {
        Arg a = NumArg(0, Cur(c)->Line);
        const Tok *t = Cur(c);
        if (t->Type == T_INT) { a.Value = t->Value; c->P++; }
        else if (t->Type == T_STR) {
                if (t->Len > 255) Fail(c, t->Line, "text is too long");
                a.Kind = 2; a.Text = t->Text; a.Len = t->Len; c->P++;
        }
        else if (t->Type == T_IDENT && IsKeyword(c, "global")) {
                const Tok *n;
                c->P++;
                n = ExpectName(c, "a global name");
                a.Kind = 3; a.Text = n->Text; a.Len = n->Len;
        }
        else if (t->Type == T_IDENT) {
                const Tok *n = ExpectName(c, "a name");
                a.Kind = 1; a.Text = n->Text; a.Len = n->Len;
        }
        else if (t->Type == T_PUNCT) {
                if (TokIs(t, "<")) a.Value = '<';
                else if (TokIs(t, ">")) a.Value = '>';
                else if (TokIs(t, "=") || TokIs(t, "==")) a.Value = '=';
                else if (TokIs(t, "!=")) a.Value = '!';
                else if (TokIs(t, ">=")) a.Value = 242;
                else if (TokIs(t, "<=")) a.Value = 243;
                else Fail(c, t->Line, "expected an argument");
                c->P++;
        }
        else Fail(c, t->Line, "expected an argument");
        return a;
}

/* optional ( arg, arg ... ) */
static void ParseCallArgs(Compiler *c, Frame *f) {
        f->ArgStart = COUNT(c->ArgPool);
        if (IsPunct(c, "(")) {
                c->P++;
                while (!IsPunct(c, ")")) {
                        if (f->ArgCount > 0) ExpectPunct(c, ",");
                        PushArg(c, f, ParseArg(c));
                }
                c->P++;
        }
}

static void ParseAction(Compiler *c, Frame *f) {
        const Tok *name = Cur(c);
        int line = name->Line;
        f->Fn = FindFunction(name->Text, name->Len);
        if (f->Fn < 0) Fail(c, line, "unknown function '%.*s'", name->Len, name->Text);
        if (IPrefixN(name->Text, name->Len, "jumpframes") || IPrefixN(name->Text, name->Len, "jumpnextframe"))
                Fail(c, line, "'%.*s' counts frames, use if / while instead", name->Len, name->Text);
        c->P++;
        ParseCallArgs(c, f);
        if (f->ArgCount != Functions[f->Fn].Arity)
                Fail(c, line, "%s needs %d argument(s), got %d", Functions[f->Fn].Name, Functions[f->Fn].Arity, f->ArgCount);
        if (Functions[f->Fn].Kind == COMPILER_KIND_OBJOBJ && FrameArg(c, f, 0)->Kind != 3)
                Fail(c, line, "%s needs a global object as its first argument, like global Name", Functions[f->Fn].Name);
}

/* Show "sprite" + Wait n, Wait n, Goto, Stop, x++ x-- x += v x -= v x = v and Function(args) */
static void ParseSimple(Compiler *c, State *st) {
        int line = Cur(c)->Line;
        if (IsKeyword(c, "show")) {
                Frame f = NewFrame(line);
                f.IsWait = 1;
                c->P++;
                if (Cur(c)->Type == T_STR) { f.Sprite = SpriteId(c, 0, Cur(c)->Text, Cur(c)->Len, 1, 1, Cur(c)->Line); c->P++; }
                else if (IsKeyword(c, "image") || IsKeyword(c, "container")) {
                        int kind = IsKeyword(c, "image") ? 1 : 2;
                        const Tok *path;
                        int scale[2];
                        int k;
                        c->P++;
                        if (Cur(c)->Type != T_STR) Fail(c, Cur(c)->Line, "expected the \"file\" of the sprite");
                        path = Cur(c);
                        c->P++;
                        for (k = 0; k < 2; k++) {
                                if (Cur(c)->Type != T_INT || Cur(c)->Value < 1 || Cur(c)->Value > 65535)
                                        Fail(c, Cur(c)->Line, "expected the sprite width and height after the file");
                                scale[k] = (int)Cur(c)->Value;
                                c->P++;
                        }
                        f.Sprite = SpriteId(c, kind, path->Text, path->Len, scale[0], scale[1], path->Line);
                }
                else if (IsKeyword(c, "none")) { c->P++; }
                else Fail(c, Cur(c)->Line, "expected a \"sprite\", image \"file\" w h, container \"file\" w h or none after Show");
                if (IsPunct(c, ";")) c->P++;
                if (!IsKeyword(c, "wait")) Fail(c, line, "Show must be directly followed by Wait");
                c->P++;
                f.Wait = ParseWaitTime(c);
                PushFrame(c, st, f);
        }
        else if (IsKeyword(c, "wait")) {
                Frame f = NewFrame(line);
                f.IsWait = 1;
                c->P++;
                f.Wait = ParseWaitTime(c);
                PushFrame(c, st, f);
        }
        else if (IsKeyword(c, "goto")) {
                const Tok *name;
                c->P++;
                name = ExpectName(c, "a state name");
                AddAction(c, st, NamedCall(c, "SetState", line, name->Text, name->Len));
        }
        else if (IsKeyword(c, "stop")) {
                Frame f = NewFrame(line);
                c->P++;
                f.Fn = FindFunctionC("RemoveObject");
                f.ArgStart = COUNT(c->ArgPool);
                AddAction(c, st, f);
        }
        else if (IsKeyword(c, "global")) {
                const Tok *name;
                c->P++;
                name = ExpectName(c, "a global name");
                if (IsAssignTok(Cur(c))) ParseAssignment(c, st, line, name->Text, name->Len, 3);
                else {
                        Frame f = NewFrame(line);
                        f.Fn = FN_GLOBAL; f.GlobalFn = name->Text; f.GlobalLen = name->Len;
                        ParseCallArgs(c, &f);
                        AddAction(c, st, f);
                }
        }
        else if (Cur(c)->Type == T_IDENT) {
                if (IsAssignTok(Peek(c, 1)) && Peek(c, 1)->Type == T_PUNCT) {
                        const Tok *var = ExpectName(c, "a var name");
                        ParseAssignment(c, st, line, var->Text, var->Len, 1);
                }
                else {
                        Frame f = NewFrame(line);
                        ParseAction(c, &f);
                        AddAction(c, st, f);
                }
        }
        else Fail(c, line, "expected a statement: Show, Wait, a function call, x++, x = 1, while, if, Goto or Stop");
}

static int AtLabel(Compiler *c) {
        return Cur(c)->Type == T_IDENT && TokIs(Peek(c, 1), ":");
}

/* frames, while and if until the closing '}' (or the next label when depth is 0) */
static void ParseStatements(Compiler *c, State *st, int depth) {
        while (!IsPunct(c, "}")) {
                if (Cur(c)->Type == T_END) Fail(c, Cur(c)->Line, "expected '}'");
                if (AtLabel(c)) {
                        if (depth == 0) break;
                        Fail(c, Cur(c)->Line, "a state label cannot be inside a block");
                }
                if (IsKeyword(c, "while")) ParseWhile(c, st, depth);
                else if (IsKeyword(c, "if")) ParseIf(c, st, depth);
                else if (IsKeyword(c, "else")) Fail(c, Cur(c)->Line, "'else' without 'if'");
                else {
                        ParseSimple(c, st);
                        if (IsPunct(c, ";")) c->P++;
                }
        }
}

/* ---- top level ------------------------------------------------------------- */
static void ParseVar(Compiler *c) {
        const Tok *name;
        VarRec *v;
        int line, i;
        long value = 0;
        c->P++; /* int */
        line = Cur(c)->Line;
        name = ExpectName(c, "a var name");
        if (IsPunct(c, "=")) {
                c->P++;
                if (Cur(c)->Type != T_INT) Fail(c, Cur(c)->Line, "expected a number");
                value = Cur(c)->Value;
                c->P++;
        }
        ExpectPunct(c, ";");
        for (i = 0; i < COUNT(c->Vars); i++) {
                if (SameN(AT(c->Vars, VarRec, i).Name, AT(c->Vars, VarRec, i).Len, name->Text, name->Len))
                        Fail(c, line, "var '%.*s' is declared twice", name->Len, name->Text);
        }
        if (COUNT(c->Vars) >= 255) Fail(c, line, "too many vars");
        v = (VarRec*)VecPush(c, &c->Vars, sizeof(VarRec));
        v->Name = name->Text; v->Len = name->Len; v->Value = value; v->Slot = -1;
        for (i = 0; i < 6; i++) if (SameN(name->Text, name->Len, BuiltinVars[i], (int)strlen(BuiltinVars[i]))) v->Slot = i;
}

static void ParseStates(Compiler *c) {
        c->P++; /* states */
        ExpectPunct(c, "{");
        while (!IsPunct(c, "}")) {
                State *st;
                const Tok *name;
                int line = Cur(c)->Line, i;
                name = ExpectName(c, "a state name");
                ExpectPunct(c, ":");
                c->Barrier = -1;
                for (i = 0; i < COUNT(c->States); i++) {
                        if (SameN(AT(c->States, State, i).Name, AT(c->States, State, i).Len, name->Text, name->Len))
                                Fail(c, line, "state '%.*s' is declared twice", name->Len, name->Text);
                }
                if (COUNT(c->States) >= 65535) Fail(c, line, "too many states");
                st = (State*)VecPush(c, &c->States, sizeof(State)); /* no state is added while its statements are parsed, the pointer stays valid */
                st->Name = name->Text; st->Len = name->Len; st->Line = line;
                ParseStatements(c, st, 0);
                if (COUNT(st->Frames) == 0) Fail(c, line, "state '%.*s' has no frames", name->Len, name->Text);
                if (COUNT(st->Frames) > 65535) Fail(c, line, "state '%.*s' has too many frames", name->Len, name->Text);
        }
        c->P++;
}

/* every name used by an action must be a var, a built in var or a state */
static void Validate(Compiler *c) {
        int s, f, a, i;
        for (s = 0; s < COUNT(c->States); s++) {
                State *st = &AT(c->States, State, s);
                for (f = 0; f < COUNT(st->Frames); f++) {
                        const Frame *fr = FrameAt(st, f);
                        for (a = 0; a < fr->ArgCount; a++) {
                                const Arg *ar = FrameArg(c, fr, a);
                                int known = 0;
                                if (ar->Kind != 1) continue;
                                for (i = 0; i < 6 && !known; i++) known = SameN(ar->Text, ar->Len, BuiltinVars[i], (int)strlen(BuiltinVars[i]));
                                for (i = 0; i < COUNT(c->Vars) && !known; i++) known = SameN(ar->Text, ar->Len, AT(c->Vars, VarRec, i).Name, AT(c->Vars, VarRec, i).Len);
                                for (i = 0; i < COUNT(c->States) && !known; i++) known = SameN(ar->Text, ar->Len, AT(c->States, State, i).Name, AT(c->States, State, i).Len);
                                if (!known) Fail(c, ar->Line, "'%.*s' is not a var or a state", ar->Len, ar->Text);
                        }
                }
        }
}

static void ParseFile(Compiler *c) {
        ExpectKeyword(c, "object");
        if (Cur(c)->Type != T_IDENT) Fail(c, Cur(c)->Line, "expected the object name");
        c->P++; /* the name only documents the file, ObjectCore_Load takes the file name */
        ExpectPunct(c, "{");
        while (!IsPunct(c, "}")) {
                if (IsKeyword(c, "int")) ParseVar(c);
                else if (IsKeyword(c, "states")) ParseStates(c);
                else if (IsKeyword(c, "solid")) { c->P++; c->Solid = 1; if (IsPunct(c, ";")) c->P++; }
                else if (IsKeyword(c, "renderonly")) { c->P++; c->RenderOnly = 1; if (IsPunct(c, ";")) c->P++; }
                else Fail(c, Cur(c)->Line, "expected 'int', 'solid', 'renderonly', 'states' or '}'");
        }
        c->P++;
        if (Cur(c)->Type != T_END) Fail(c, Cur(c)->Line, "unexpected text after the object");
        if (COUNT(c->States) == 0) Fail(c, Cur(c)->Line, "the object has no states");
        Validate(c);
}

static void Write(Compiler *c) {
        int i, s, f, a, newVars = 0;
        PutBytes(c, "MTO1", 4);
        Put8(c, (c->Solid ? 1 : 0) | (c->RenderOnly ? 2 : 0));
        Put16(c, (unsigned)COUNT(c->Sprites));
        for (i = 0; i < COUNT(c->Sprites); i++) {
                const SpriteDef *sd = &AT(c->Sprites, SpriteDef, i);
                Put8(c, (unsigned)sd->Kind);
                if (sd->Kind > 0) { Put16(c, (unsigned)sd->Sx); Put16(c, (unsigned)sd->Sy); }
                Put8(c, (unsigned)sd->Len);
                PutBytes(c, sd->Path, sd->Len);
        }
        Put16(c, (unsigned)COUNT(c->States));
        for (s = 0; s < COUNT(c->States); s++) {
                State *st = &AT(c->States, State, s);
                Put8(c, (unsigned)st->Len);
                PutBytes(c, st->Name, st->Len);
                Put16(c, (unsigned)COUNT(st->Frames));
                for (f = 0; f < COUNT(st->Frames); f++) {
                        const Frame *fr = FrameAt(st, f);
                        unsigned strBytes = 0;
                        Put16(c, fr->Sprite < 0 ? 0xFFFF : (unsigned)fr->Sprite);
                        Put16(c, (unsigned)fr->Wait);
                        Put16(c, fr->Fn == FN_NONE ? 0xFFFF : fr->Fn == FN_GLOBAL ? 0xFFFE : (unsigned)fr->Fn);
                        if (fr->Fn == FN_NONE) continue;
                        if (fr->Fn == FN_GLOBAL) {
                                Put8(c, (unsigned)fr->GlobalLen);
                                PutBytes(c, fr->GlobalFn, fr->GlobalLen);
                        }
                        for (a = 0; a < fr->ArgCount; a++) if (FrameArg(c, fr, a)->Kind == 2) strBytes += (unsigned)FrameArg(c, fr, a)->Len + 1;
                        Put8(c, (unsigned)fr->ArgCount);
                        Put16(c, strBytes);
                        for (a = 0; a < fr->ArgCount; a++) {
                                const Arg *ar = FrameArg(c, fr, a);
                                Put8(c, (unsigned)ar->Kind);
                                if (ar->Kind == 0) Put32(c, ar->Value);
                                else {
                                        Put8(c, (unsigned)ar->Len);
                                        PutBytes(c, ar->Text, ar->Len);
                                }
                        }
                }
        }
        for (i = 0; i < COUNT(c->Vars); i++) if (AT(c->Vars, VarRec, i).Slot < 0) newVars++;
        Put8(c, (unsigned)newVars);
        Put8(c, (unsigned)COUNT(c->Vars));
        for (i = 0; i < COUNT(c->Vars); i++) {
                const VarRec *v = &AT(c->Vars, VarRec, i);
                Put8(c, v->Slot < 0 ? 0xFF : (unsigned)v->Slot);
                if (v->Slot < 0) {
                        Put8(c, (unsigned)v->Len);
                        PutBytes(c, v->Name, v->Len);
                }
                Put32(c, v->Value);
        }
}

static void CompilerFree(Compiler *c) {
        int s;
        for (s = 0; s < COUNT(c->States); s++) free(AT(c->States, State, s).Frames.p);
        free(c->Toks.p); free(c->Vars.p); free(c->States.p); free(c->ArgPool.p);
        free(c->Conds.p); free(c->Sprites.p); free(c->Out.p);
        free(c);
}

static void CopyError(char *error, int size, const char *text) {
        if (error && size > 0) {
                strncpy(error, text, (size_t)size - 1);
                error[size - 1] = 0;
        }
}

bool CscriptCompiler_Compile(const char *text, unsigned char **out, unsigned long *size, char *error, int errorSize) {
        Compiler *c;
        int ok = 0;
        *out = 0;
        *size = 0;
        CopyError(error, errorSize, "");
        if (!text) { CopyError(error, errorSize, "no text"); return 0; }
        c = (Compiler*)calloc(1, sizeof(Compiler));
        if (!c) { CopyError(error, errorSize, "out of memory"); return 0; }
        c->Barrier = -1;
        if (setjmp(c->Jmp) == 0) {
                Tokenize(c, text);
                ParseFile(c);
                Write(c);
                *out = (unsigned char*)c->Out.p;
                *size = (unsigned long)c->Out.n;
                c->Out.p = 0;
                ok = 1;
        }
        else {
                CopyError(error, errorSize, c->Err);
        }
        CompilerFree(c);
        return ok;
}

void CscriptCompiler_Free(unsigned char *out) { free(out); }

Object *ObjectCore_LoadScript(const char *textFile, char *error, int errorSize) {
        FILE *in = fopen(textFile, "rb");
        char *text;
        long length;
        unsigned char *bin = 0;
        unsigned long binSize = 0;
        Object *o;
        if (!in) {
                char msg[256];
                snprintf(msg, sizeof(msg), "cannot open %s", textFile);
                CopyError(error, errorSize, msg);
                return 0;
        }
        fseek(in, 0, SEEK_END);
        length = ftell(in);
        fseek(in, 0, SEEK_SET);
        text = (char*)malloc((size_t)length + 1);
        if (!text || length < 0) { fclose(in); free(text); CopyError(error, errorSize, "out of memory"); return 0; }
        length = (long)fread(text, 1, (size_t)length, in);
        text[length] = 0;
        fclose(in);
        if (!CscriptCompiler_Compile(text, &bin, &binSize, error, errorSize)) { free(text); return 0; }
        free(text);
        o = ObjectCore_LoadFromMemory(bin, binSize);
        if (!o) CopyError(error, errorSize, "the compiled object could not be loaded (missing sprite file?)");
        CscriptCompiler_Free(bin);
        return o;
}
#endif

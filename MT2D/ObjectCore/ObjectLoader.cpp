/* ObjectLoader.cpp, see ObjectLoader.h for the file format. */
#include <stdlib.h>
#include <string.h>
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/File/MT2D_File.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
#include <MT2D/ObjectCore/CscriptFunctions.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/State.h>

#define LOADER_NAME_MAX (MT2D_VAR_NAME_MAX - 1)

/* ---- function table: ids -> pointers, no strings ------------------------ */
struct LoaderFn {
        void (*Obj)(Object *);
        void (*Vars)(Object *, MT2D_VAR **);
};
#define LOADER_FN_OBJ(Symbol)  { Symbol, 0 }
#define LOADER_FN_VARS(Symbol) { 0, Symbol }
#define LOADER_FN_ENTRY(Name, Symbol, Kind, Arity) LOADER_FN_##Kind(Symbol),
static const LoaderFn Loader_Functions[CSCRIPT_FN_COUNT] = { MT2D_CSCRIPT_FUNCTIONS(LOADER_FN_ENTRY) };
#undef LOADER_FN_ENTRY
#define LOADER_ARITY_ENTRY(Name, Symbol, Kind, Arity) Arity,
static const unsigned char Loader_Arity[CSCRIPT_FN_COUNT] = { MT2D_CSCRIPT_FUNCTIONS(LOADER_ARITY_ENTRY) };
#undef LOADER_ARITY_ENTRY

/* ---- reader: a file or a block of memory --------------------------------- */
struct Reader {
        MT2D_FILE *File;
        const unsigned char *Mem;
        unsigned long Size;
        unsigned long Pos;
        bool Ok;
};

static bool Rd_Bytes(Reader &r, void *dst, unsigned long n) {
        if (!r.Ok) return false;
        if (n == 0) return true;
        if (r.File) {
                if (MT2D_FILE_READ(r.File, dst, 1, (size_t)n) != (size_t)n) r.Ok = false;
        }
        else if (r.Size - r.Pos < n) {
                r.Ok = false;
        }
        else {
                memcpy(dst, r.Mem + r.Pos, n);
                r.Pos += n;
        }
        return r.Ok;
}

static unsigned Rd_U8(Reader &r) {
        unsigned char b[1] = { 0 };
        Rd_Bytes(r, b, 1);
        return b[0];
}

static unsigned Rd_U16(Reader &r) {
        unsigned char b[2] = { 0, 0 };
        Rd_Bytes(r, b, 2);
        return (unsigned)b[0] | ((unsigned)b[1] << 8);
}

static int Rd_I32(Reader &r) {
        unsigned char b[4] = { 0, 0, 0, 0 };
        Rd_Bytes(r, b, 4);
        unsigned long v = (unsigned long)b[0] | ((unsigned long)b[1] << 8) | ((unsigned long)b[2] << 16) | ((unsigned long)b[3] << 24);
        return (int)(long)v; /* two's complement, same on every target we build for */
}

/* ---- frame function: Cscript + arg pointers + arg vars + texts in ONE block */
static Cscript *Load_Function(Reader &r, unsigned fn, unsigned argc, unsigned stringBytes) {
        if (argc == 0 && stringBytes != 0) { r.Ok = false; return 0; }
        size_t total = sizeof(Cscript) + argc * sizeof(MT2D_VAR*) + argc * sizeof(MT2D_VAR) + stringBytes;
        Cscript *c = (Cscript*)malloc(total);
        if (!c) { r.Ok = false; return 0; }
        memset(c, 0, total);
        c->Packed = 1;
        if (argc == 0) {
                c->_Obj = Loader_Functions[fn].Obj;
        }
        else {
                MT2D_VAR **ptrs = (MT2D_VAR**)(c + 1);
                MT2D_VAR *vars = (MT2D_VAR*)(ptrs + argc);
                char *text = (char*)(vars + argc);
                char *textEnd = text + stringBytes;
                char name[256];
                unsigned i;
                for (i = 0; i < argc && r.Ok; i++) {
                        unsigned kind = Rd_U8(r);
                        ptrs[i] = &vars[i];
                        if (kind == 0) {
                                MT2D_VAR_Init_Int(&vars[i], "c", Rd_I32(r));
                        }
                        else if (kind == 1) {
                                unsigned len = Rd_U8(r);
                                if (len < 1 || len > LOADER_NAME_MAX) { r.Ok = false; break; }
                                Rd_Bytes(r, name, len);
                                name[len] = 0;
                                if (r.Ok && !MT2D_VAR_Init_Pointer(&vars[i], name)) r.Ok = false;
                        }
                        else if (kind == 2) {
                                unsigned len = Rd_U8(r);
                                if ((unsigned long)(textEnd - text) < (unsigned long)len + 1) { r.Ok = false; break; }
                                Rd_Bytes(r, text, len);
                                text[len] = 0;
                                vars[i].Type = VAR_STRING;
                                vars[i].Data.p = text;
                                text += len + 1;
                        }
                        else {
                                r.Ok = false;
                        }
                }
                if (r.Ok && text != textEnd) r.Ok = false; /* stringBytes must be exact */
                c->TempVars = ptrs;
                c->_Obj_Vars = Loader_Functions[fn].Vars;
        }
        if (!r.Ok) { free(c); return 0; }
        return c;
}

static void Load_ReleaseUnusedSprites(Sprite **sprites, unsigned count) {
        for (unsigned i = 0; i < count; i++) {
                if (sprites[i] && sprites[i]->refCount == 0) {
                        Sprite_Delete(sprites[i]); /* refCount 0 frees it */
                }
        }
}

static Object *Load(Reader &r) {
        char magic[4];
        char path[256];
        unsigned i, j;
        unsigned spriteCount = 0, stateCount = 0, built = 0;
        Sprite **sprites = 0;
        ObjectState **states = 0;
        Object *obj = 0;

        Rd_Bytes(r, magic, 4);
        if (!r.Ok || memcmp(magic, "MTO1", 4) != 0) return 0;

        spriteCount = Rd_U16(r);
        if (spriteCount) {
                sprites = (Sprite**)calloc(spriteCount, sizeof(Sprite*));
                if (!sprites) return 0;
        }
        for (i = 0; i < spriteCount && r.Ok; i++) {
                unsigned len = Rd_U8(r);
                Rd_Bytes(r, path, len);
                path[len] = 0;
                if (r.Ok) {
                        sprites[i] = Load_Sprite(path);
                        if (!sprites[i]) r.Ok = false;
                }
        }

        if (r.Ok) {
                stateCount = Rd_U16(r);
                if (stateCount < 1) r.Ok = false;
        }
        if (r.Ok) {
                states = (ObjectState**)calloc(stateCount, sizeof(ObjectState*));
                if (!states) r.Ok = false;
        }
        for (i = 0; i < stateCount && r.Ok; i++) {
                unsigned nameLen = Rd_U8(r);
                unsigned frames;
                ObjectState *st;
                if (!r.Ok || nameLen < 1 || nameLen > LOADER_NAME_MAX) { r.Ok = false; break; }
                st = (ObjectState*)malloc(sizeof(ObjectState) + nameLen + 1); /* the name is stored after the state */
                if (!st) { r.Ok = false; break; }
                st->Name = (char*)(st + 1);
                Rd_Bytes(r, st->Name, nameLen);
                st->Name[nameLen] = 0;
                st->Sprites = 0; st->Functions = 0; st->WaitSprites = 0; st->Count = 0;
                states[built++] = st;
                frames = Rd_U16(r);
                if (!r.Ok || frames < 1) { r.Ok = false; break; }
                st->Sprites = (Sprite**)malloc(frames * sizeof(Sprite*));
                st->Functions = (Cscript**)malloc(frames * sizeof(Cscript*));
                st->WaitSprites = (int*)malloc(frames * sizeof(int));
                if (!st->Sprites || !st->Functions || !st->WaitSprites) { r.Ok = false; break; }
                for (j = 0; j < frames && r.Ok; j++) {
                        unsigned sp = Rd_U16(r);
                        unsigned wait = Rd_U16(r);
                        unsigned fn = Rd_U16(r);
                        Cscript *c = 0;
                        if (!r.Ok) break;
                        if (sp != CSCRIPT_FN_NONE && sp >= spriteCount) { r.Ok = false; break; }
                        if (fn != CSCRIPT_FN_NONE) {
                                unsigned argc, stringBytes;
                                if (fn >= CSCRIPT_FN_COUNT) { r.Ok = false; break; }
                                argc = Rd_U8(r);
                                stringBytes = Rd_U16(r);
                                if (!r.Ok || argc != Loader_Arity[fn]) { r.Ok = false; break; }
                                c = Load_Function(r, fn, argc, stringBytes);
                                if (!c) break;
                        }
                        st->Sprites[j] = sp == CSCRIPT_FN_NONE ? 0 : sprites[sp];
                        st->WaitSprites[j] = (int)wait;
                        st->Functions[j] = c;
                        if (st->Sprites[j]) st->Sprites[j]->refCount++;
                        st->Count++;
                }
        }

        if (r.Ok) {
                obj = Object_Create(false, false, 0, 0, 0, 0, states, stateCount);
                if (!obj) r.Ok = false;
        }
        if (r.Ok) {
                unsigned newVars = Rd_U8(r);
                unsigned records = Rd_U8(r);
                unsigned added = 0;
                MT2D_VAR *grown = r.Ok ? (MT2D_VAR*)realloc(obj->User_Vars, (Object_First_User_Var_Index + newVars + 1) * sizeof(MT2D_VAR)) : 0; /* + the spare slot Object_Add_Variable keeps */
                if (!grown) r.Ok = false;
                else obj->User_Vars = grown;
                for (i = 0; i < records && r.Ok; i++) {
                        unsigned slot = Rd_U8(r);
                        if (slot < Object_First_User_Var_Index) {
                                obj->User_Vars[slot].Data.i = Rd_I32(r);
                        }
                        else if (slot == 0xFF && added < newVars) {
                                unsigned len = Rd_U8(r);
                                if (len < 1 || len > LOADER_NAME_MAX) { r.Ok = false; break; }
                                Rd_Bytes(r, path, len);
                                path[len] = 0;
                                int value = Rd_I32(r);
                                if (r.Ok && MT2D_VAR_Init_Int(&obj->User_Vars[obj->User_Vars_Count], path, value)) {
                                        obj->User_Vars_Count++;
                                        added++;
                                }
                                else r.Ok = false;
                        }
                        else r.Ok = false;
                }
                if (r.Ok && added != newVars) r.Ok = false;
        }

        Load_ReleaseUnusedSprites(sprites, spriteCount);
        free(sprites);
        if (!r.Ok) {
                if (obj) {
                        Object_Delete(obj, true);
                }
                else {
                        for (i = 0; i < built; i++) MT2D_ObjectState_Delete(states[i]);
                }
                free(states);
                return 0;
        }
        return obj;
}

Object *ObjectCore_LoadFromMemory(const unsigned char *data, unsigned long size) {
        Reader r;
        if (!data) return 0;
        r.File = 0; r.Mem = data; r.Size = size; r.Pos = 0; r.Ok = true;
        return Load(r);
}

Object *ObjectCore_Load(const char *file) {
        Reader r;
        Object *o;
        MT2D_FILE *f = file ? MT2D_FILE_OPEN(file, "rb") : 0;
        if (!f) return 0;
        r.File = f; r.Mem = 0; r.Size = 0; r.Pos = 0; r.Ok = true;
        o = Load(r);
        MT2D_FILE_CLOSE(f);
        return o;
}

void ObjectCore_Unload(Object *Model) {
        if (Model) {
                ObjectState **states = Model->State;
                Object_Delete(Model, true);
                free(states);
        }
}

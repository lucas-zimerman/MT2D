/* ObjectLoader.cpp, see ObjectLoader.h for the file format. */
#include <stdlib.h>
#include <string.h>
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/File/MT2D_File.h>
#include <MT2D/ObjectCore/ObjectLoader.h>
#include <MT2D/ObjectCore/ObjectGlobals.h>
#include <MT2D/ObjectCore/CscriptFunctions.h>
#include <MT2D/ObjectCore/STDFunctions.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/ObjectCore/State.h>

#define LOADER_NAME_MAX (MT2D_VAR_NAME_MAX - 1)
#define LOADER_NONE 0xFFFF
#define LOADER_GLOBAL_FN 0xFFFE

/* ---- built in functions: id -> entry, no strings ------------------------ */
#define LOADER_FN_OBJ(Symbol, Arity)    { CSCRIPT_KIND_OBJ, Arity, Symbol, 0, 0 }
#define LOADER_FN_VARS(Symbol, Arity)   { CSCRIPT_KIND_VARS, Arity, 0, Symbol, 0 }
#define LOADER_FN_OBJOBJ(Symbol, Arity) { CSCRIPT_KIND_OBJ_OBJ_VARS, Arity, 0, 0, Symbol }
#define LOADER_FN_ENTRY(Name, Symbol, Kind, Arity) LOADER_FN_##Kind(Symbol, Arity),
static const ObjectCore_CscriptEntry Loader_Functions[CSCRIPT_FN_COUNT] = { MT2D_CSCRIPT_FUNCTIONS(LOADER_FN_ENTRY) };
#undef LOADER_FN_ENTRY

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

/* reads { u8 len, name } (1..18 characters) into out (at least 256 bytes) */
static bool Rd_Name(Reader &r, char *out) {
        unsigned len = Rd_U8(r);
        if (!r.Ok || len < 1 || len > LOADER_NAME_MAX) { r.Ok = false; return false; }
        Rd_Bytes(r, out, len);
        out[len] = 0;
        return r.Ok;
}

/* ---- frame function: Cscript + arg pointers + arg vars + texts in ONE block */
static Cscript *Load_Function(Reader &r, const ObjectCore_CscriptEntry &fn, unsigned argc, unsigned stringBytes) {
        bool takesObject = fn.Kind == CSCRIPT_KIND_OBJ_OBJ_VARS;
        unsigned varArgs = takesObject ? argc - 1 : argc;
        if (takesObject && argc < 1) { r.Ok = false; return 0; }
        if (argc == 0 && stringBytes != 0) { r.Ok = false; return 0; }
        if (fn.Kind == CSCRIPT_KIND_VARS && argc == 0) { r.Ok = false; return 0; }
        size_t total = sizeof(Cscript) + varArgs * sizeof(MT2D_VAR*) + varArgs * sizeof(MT2D_VAR) + stringBytes;
        Cscript *c = (Cscript*)malloc(total);
        if (!c) { r.Ok = false; return 0; }
        memset(c, 0, total);
        c->Packed = 1;
        MT2D_VAR **ptrs = (MT2D_VAR**)(c + 1);
        MT2D_VAR *vars = (MT2D_VAR*)(ptrs + varArgs);
        char *text = (char*)(vars + varArgs);
        char *textEnd = text + stringBytes;
        char name[256];
        unsigned i, slot = 0;
        for (i = 0; i < argc && r.Ok; i++) {
                unsigned kind = Rd_U8(r);
                bool isObject = takesObject && i == 0;
                if (isObject) {
                        if (kind != 3 || !Rd_Name(r, name)) { r.Ok = false; break; }
                        c->TempObj = ObjectCore_FindGlobalObject(name);
                        if (!c->TempObj) { ObjectCore_SetLastError("global object is not registered:", name); r.Ok = false; }
                        continue;
                }
                if (kind == 0) {
                        MT2D_VAR_Init_Int(&vars[slot], "c", Rd_I32(r));
                        ptrs[slot] = &vars[slot];
                }
                else if (kind == 1) {
                        if (!Rd_Name(r, name)) break;
                        if (!MT2D_VAR_Init_Pointer(&vars[slot], name)) r.Ok = false;
                        ptrs[slot] = &vars[slot];
                }
                else if (kind == 2) {
                        unsigned len = Rd_U8(r);
                        if ((unsigned long)(textEnd - text) < (unsigned long)len + 1) { r.Ok = false; break; }
                        Rd_Bytes(r, text, len);
                        text[len] = 0;
                        vars[slot].Type = VAR_STRING;
                        vars[slot].Data.p = text;
                        ptrs[slot] = &vars[slot];
                        text += len + 1;
                }
                else if (kind == 3) {
                        if (!Rd_Name(r, name)) break;
                        ptrs[slot] = ObjectCore_FindGlobalVar(name); /* the registered var itself, not a copy */
                        if (!ptrs[slot]) { ObjectCore_SetLastError("global var is not registered:", name); r.Ok = false; }
                }
                else {
                        r.Ok = false;
                }
                slot++;
        }
        if (r.Ok && text != textEnd) r.Ok = false; /* stringBytes must be exact */
        if (fn.Kind == CSCRIPT_KIND_OBJ) {
                c->_Obj = fn.Obj;
        }
        else if (fn.Kind == CSCRIPT_KIND_VARS) {
                c->_Obj_Vars = fn.Vars;
                c->TempVars = ptrs;
        }
        else {
                c->_Obj_Obj_Vars = fn.ObjObj;
                c->TempVars = varArgs ? ptrs : 0;
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

static Sprite *Load_OneSprite(unsigned kind, char *path, int sx, int sy) {
        if (kind == 0) return Load_Sprite(path);
        if (kind == 1) return Load_Sprite_Image(path, sx, sy);
#if defined(MT2D_USING_CONTAINER)
        if (kind == 2) return Load_Sprite_Image_From_Container(path, sx, sy);
#endif
        return 0;
}

static Object *Load(Reader &r) {
        char magic[4];
        char path[256];
        unsigned i, j;
        unsigned flags = 0;
        unsigned spriteCount = 0, stateCount = 0, built = 0;
        Sprite **sprites = 0;
        ObjectState **states = 0;
        Object *obj = 0;

        ObjectCore_ClearLastError();
        Rd_Bytes(r, magic, 4);
        if (!r.Ok || memcmp(magic, "MTO1", 4) != 0) return 0;
        flags = Rd_U8(r);

        spriteCount = Rd_U16(r);
        if (spriteCount) {
                sprites = (Sprite**)calloc(spriteCount, sizeof(Sprite*));
                if (!sprites) return 0;
        }
        for (i = 0; i < spriteCount && r.Ok; i++) {
                unsigned kind = Rd_U8(r);
                int sx = 1, sy = 1;
                if (kind > 2) { r.Ok = false; break; }
                if (kind > 0) { sx = (int)Rd_U16(r); sy = (int)Rd_U16(r); }
                unsigned len = Rd_U8(r);
                Rd_Bytes(r, path, len);
                path[len] = 0;
                if (r.Ok) {
                        sprites[i] = Load_OneSprite(kind, path, sx, sy);
                        if (!sprites[i]) { ObjectCore_SetLastError("sprite could not be loaded:", path); r.Ok = false; }
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
                        if (sp != LOADER_NONE && sp >= spriteCount) { r.Ok = false; break; }
                        if (fn != LOADER_NONE) {
                                ObjectCore_CscriptEntry entry;
                                unsigned argc, stringBytes;
                                if (fn == LOADER_GLOBAL_FN) {
                                        if (!Rd_Name(r, path)) break;
                                        if (!ObjectCore_FindGlobalCScript(path, &entry)) {
                                                ObjectCore_SetLastError("global function is not registered:", path);
                                                r.Ok = false;
                                                break;
                                        }
                                }
                                else if (fn < CSCRIPT_FN_COUNT) {
                                        entry = Loader_Functions[fn];
                                }
                                else { r.Ok = false; break; }
                                argc = Rd_U8(r);
                                stringBytes = Rd_U16(r);
                                if (!r.Ok || (entry.ArgCount >= 0 && (unsigned)entry.ArgCount != argc)) {
                                        if (r.Ok) ObjectCore_SetLastError("wrong argument count for the function in state", st->Name);
                                        r.Ok = false;
                                        break;
                                }
                                c = Load_Function(r, entry, argc, stringBytes);
                                if (!c) break;
                        }
                        st->Sprites[j] = sp == LOADER_NONE ? 0 : sprites[sp];
                        st->WaitSprites[j] = (int)wait;
                        st->Functions[j] = c;
                        if (st->Sprites[j]) st->Sprites[j]->refCount++;
                        st->Count++;
                }
        }

        if (r.Ok) {
                obj = Object_Create((flags & 1) != 0, (flags & 2) != 0, 0, 0, 0, 0, states, stateCount);
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
        if (!f) {
                ObjectCore_ClearLastError();
                if (file) ObjectCore_SetLastError("file could not be opened:", file);
                return 0;
        }
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

Object *ObjectCore_Clone(Object *Model, int x, int y) {
        if (!Model) return 0;
        Object *o = Object_Create(
                Model->Solid, Model->RenderOnly,
                Model->User_Vars[Object_SizeX_Var_Index].Data.i,
                Model->User_Vars[Object_SizeY_Var_Index].Data.i,
                x, y, Model->State, Model->States_Count);
        if (!o) return 0;
        o->User_Vars[Object_AcelX_Var_Index].Data.i = Model->User_Vars[Object_AcelX_Var_Index].Data.i;
        o->User_Vars[Object_AcelY_Var_Index].Data.i = Model->User_Vars[Object_AcelY_Var_Index].Data.i;
        int extra = Model->User_Vars_Count - Object_First_User_Var_Index;
        if (extra > 0) {
                /* one allocation, plus the spare slot Object_Add_Variable keeps */
                MT2D_VAR *grown = (MT2D_VAR*)realloc(o->User_Vars, (Object_First_User_Var_Index + extra + 1) * sizeof(MT2D_VAR));
                if (!grown) { Object_Delete(o, false); return 0; }
                o->User_Vars = grown;
                for (int i = 0; i < extra; i++) {
                        MT2D_VAR *copy = MT2D_VAR_CLONE(&Model->User_Vars[Object_First_User_Var_Index + i]);
                        if (!copy) { Object_Delete(o, false); return 0; }
                        o->User_Vars[o->User_Vars_Count++] = *copy;
                        free(copy); /* only the shell, Name and Data now belong to the new object */
                }
        }
        return o;
}

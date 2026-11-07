#include <MT2D/ObjectCore/STDFunctions.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef BENCH_OLD
#define VI(v) (*(int*)(v)->Data)
#else
#define VI(v) ((v)->Data.i)
#endif
bool Object_Hit_Object(Object*, Object*);

static unsigned long g_calls = 0;
extern "C" {
void *__real_malloc(size_t); void *__real_realloc(void*,size_t);
void *__wrap_malloc(size_t n){ g_calls++; return __real_malloc(n); }
void *__wrap_realloc(void*p,size_t n){ g_calls++; return __real_realloc(p,n); }
}
static double now(){ return (double)uclock() * 1000.0 / UCLOCKS_PER_SEC; }
#define RUN(name, reps, ...) { double best = 1e18; for (int r_ = 0; r_ < (reps); r_++) { double t0_ = now(); __VA_ARGS__; double t1_ = now(); if (t1_ - t0_ < best) best = t1_ - t0_; } printf("RESULT %-30s ms=%10.2f\n", name, best); fflush(stdout); }

static ObjectState **mkstates(){
    ObjectState **s=(ObjectState**)malloc(sizeof(ObjectState*));
    s[0]=MT2D_OBJECT_CREATE_STATE((char*)"Idle");
    MT2D_ObjectState_ADD(s[0],0,3,0);
    return s;
}

int main(){
    int i, f, j;
    const int NV = 200000, NO = 10000, NH = 1000, NA = 500000, NM = 2000000;
    printf("INFO sizeof(MT2D_VAR)=%u\n",(unsigned)sizeof(MT2D_VAR));
    { unsigned long c0=g_calls;
      RUN("A var create+free x200k",3, for(i=0;i<NV;i++){ MT2D_VAR*v=MT2D_Object_Create_Var_Int("Health",i); MT2D_VAR_Free(v,1);} );
      printf("RESULT %-30s mallocs/var=%.2f\n","A allocations",(double)(g_calls-c0)/(3.0*NV)); }
    ObjectState **st=mkstates();
    Object **objs=(Object**)malloc(NO*sizeof(Object*));
    { unsigned long c0=g_calls; double t0=now();
      for(i=0;i<NO;i++){ objs[i]=Object_Create(true,false,2,2,i%80,i%25,st,1); }
      double t1=now();
      printf("RESULT %-30s ms=%10.2f  mallocs/object=%.1f\n","B create 10k objects",t1-t0,(double)(g_calls-c0)/NO); fflush(stdout); }
    RUN("C Cscript_Move 10k x100",3, for(f=0;f<100;f++) for(i=0;i<NO;i++){ Cscript_Move(objs[i]); } );
    { volatile int hits=0;
      RUN("D Hit_Object 1000^2",3, int h=0; for(i=0;i<NH;i++) for(j=0;j<NH;j++) h+=Object_Hit_Object(objs[i],objs[j]); hits=h; ); }
    { MT2D_VAR *pos=MT2D_Object_Create_Var_Pointer("Pos_X");
      MT2D_VAR *k=MT2D_Object_Create_Var_Int("K",1);
      MT2D_VAR *vars[2]; vars[0]=pos; vars[1]=k;
      RUN("E Cscript_VAR_ADD x500k",3, for(i=0;i<NA;i++){ Cscript_VAR_ADD(objs[i%NO],vars);} );
      MT2D_VAR *a=MT2D_Object_Create_Var_Int("A",0), *b=MT2D_Object_Create_Var_Char("B",2), *c=MT2D_Object_Create_Var_Int("C",3);
      RUN("E2 ADD/SUB/GetInt x2M",3, long s=0; for(i=0;i<NM;i++){ MT2D_Object_ADD(a,c); MT2D_Object_SUB(a,b); s+=MT2D_Object_VAR_GetInt(a);} VI(a)=(int)s; ); }
    { MT2D_VAR *src=MT2D_Object_Create_Var_Int("S",5);
      RUN("F VAR_CLONE+free x200k",3, for(i=0;i<NV;i++){ MT2D_VAR*c=MT2D_VAR_CLONE(src); MT2D_VAR_Free(c,1);} ); }
    RUN("G Object_Delete 10k",1, for(i=0;i<NO;i++) Object_Delete(objs[i],false); );
    printf("DONE\n");
    return 0;
}

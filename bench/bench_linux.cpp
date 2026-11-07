#include <MT2D/ObjectCore/STDFunctions.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/State.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#ifdef BENCH_OLD
#define VI(v) (*(int*)(v)->Data)
#else
#define VI(v) ((v)->Data.i)
#endif
bool Object_Hit_Object(Object*, Object*);

/* --- allocation accounting (link with --wrap) --- */
static size_t g_calls = 0;
extern "C" {
void *__real_malloc(size_t); void __real_free(void*); void *__real_realloc(void*,size_t);
void *__wrap_malloc(size_t n){ g_calls++; return __real_malloc(n); }
void *__wrap_realloc(void*p,size_t n){ g_calls++; return __real_realloc(p,n); }
}
static size_t heap_live(){ struct mallinfo2 m = mallinfo2(); return m.uordblks; }

/* --- counters --- */
static int perf_fd = -1;
static void perf_open(){
    struct perf_event_attr a; memset(&a,0,sizeof a); a.type=PERF_TYPE_HARDWARE; a.size=sizeof a;
    a.config=PERF_COUNT_HW_INSTRUCTIONS; a.disabled=1; a.exclude_kernel=1; a.exclude_hv=1;
    perf_fd=(int)syscall(SYS_perf_event_open,&a,0,-1,-1,0);
}
static double now(){ timespec t; clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t); return t.tv_sec*1e3+t.tv_nsec/1e6; }
struct M { double ms; long long ins; };
static M g_best;
template<class F> static void run(const char *name, int reps, F f){
    double best=1e18; long long bins=-1;
    for(int r=0;r<reps;r++){
        long long c0=0,c1=0;
        if(perf_fd>=0){ ioctl(perf_fd,PERF_EVENT_IOC_RESET,0); ioctl(perf_fd,PERF_EVENT_IOC_ENABLE,0); }
        double t0=now(); f(); double t1=now();
        if(perf_fd>=0){ ioctl(perf_fd,PERF_EVENT_IOC_DISABLE,0); if(read(perf_fd,&c1,8)!=8) c1=-1; }
        (void)c0;
        if(t1-t0<best){best=t1-t0;bins=c1;}
    }
    printf("RESULT %-28s ms=%9.2f  instr=%lld\n",name,best,bins);
}

static ObjectState **mkstates(){
    ObjectState **s=(ObjectState**)malloc(sizeof(ObjectState*));
    s[0]=MT2D_OBJECT_CREATE_STATE((char*)"Idle");
    MT2D_ObjectState_ADD(s[0],0,3,0);
    return s;
}

int main(){
    perf_open();
    printf("INFO sizeof(MT2D_VAR)=%zu perf=%s\n",sizeof(MT2D_VAR),perf_fd>=0?"yes":"no");

    /* A: create + free vars */
    { const int N=2000000;
      size_t c0=g_calls;
      run("A var create+free x2M",5,[&]{ for(int i=0;i<N;i++){ MT2D_VAR*v=MT2D_Object_Create_Var_Int("Health",i); MT2D_VAR_Free(v,1);} });
      printf("RESULT %-28s mallocs/var=%.2f\n","A allocations",(double)(g_calls-c0)/(5.0*N)); }

    /* B: memory of 50k objects */
    const int NO=50000;
    ObjectState **st=mkstates();
    Object **objs=(Object**)malloc(NO*sizeof(Object*));
    { size_t h0=heap_live(), c0=g_calls;
      double t0=now();
      for(int i=0;i<NO;i++){ objs[i]=Object_Create(true,false,2,2,i%80,i%25,st,1); }
      double t1=now();
      size_t h1=heap_live();
      printf("RESULT %-28s ms=%9.2f\n","B create 50k objects",t1-t0);
      printf("RESULT %-28s %zu bytes total, %.1f bytes/object, %.1f mallocs/object\n","B heap",h1-h0,(double)(h1-h0)/NO,(double)(g_calls-c0)/NO); }

    /* C: per-frame update (Move) over all objects, 100 frames */
    run("C Cscript_Move 50k x100",5,[&]{ for(int f=0;f<100;f++) for(int i=0;i<NO;i++){ Cscript_Move(objs[i]); } });

    /* D: pairwise collision checks */
    { volatile int hits=0;
      run("D Hit_Object 2000^2",5,[&]{ int h=0; for(int i=0;i<2000;i++) for(int j=0;j<2000;j++) h+=Object_Hit_Object(objs[i],objs[j]); hits=h; }); }

    /* E: var math through Cscript */
    { MT2D_VAR *pos=MT2D_Object_Create_Var_Pointer("Pos_X");
      MT2D_VAR *k=MT2D_Object_Create_Var_Int("K",1);
      MT2D_VAR *vars[2]={pos,k};
      run("E Cscript_VAR_ADD x5M",5,[&]{ for(int i=0;i<5000000;i++){ Cscript_VAR_ADD(objs[i%NO],vars);} });
      MT2D_VAR *a=MT2D_Object_Create_Var_Int("A",0), *b=MT2D_Object_Create_Var_Char("B",2), *c=MT2D_Object_Create_Var_Int("C",3);
      run("E2 ADD/SUB/GetInt x20M",5,[&]{ long s=0; for(int i=0;i<20000000;i++){ MT2D_Object_ADD(a,c); MT2D_Object_SUB(a,b); s+=MT2D_Object_VAR_GetInt(a);} VI(a)=(int)s; }); }

    /* F: clone vars (CreateObject path) */
    { MT2D_VAR *src=MT2D_Object_Create_Var_Int("S",5);
      run("F VAR_CLONE+free x2M",5,[&]{ for(int i=0;i<2000000;i++){ MT2D_VAR*c=MT2D_VAR_CLONE(src); MT2D_VAR_Free(c,1);} }); }

    /* G: delete */
    run("G Object_Delete 50k",1,[&]{ for(int i=0;i<NO;i++) Object_Delete(objs[i],false); });
    return 0;
}

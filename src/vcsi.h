/* vcsi.h */

/* Local configurations */
#include "vcsi-conf.h"

#ifdef WITH_THREADS
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef WITH_DYNAMIC_LOAD
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif /* HAVE_DLFCN_H */
#endif /* WITH_DYNAMIC_LOAD */
#include <regex.h>
#ifdef WITH_INFOSTD
#include <infostd.h>
#endif /* WITH_INFOSTD */

/* Main VCSI Object structure */
typedef enum vcsi_obj_type {
  NONE, FREED, CONS, BOOLEAN, CHAR, STRING, LNGNUM, FLTNUM, RATNUM, 
  SYMBOL, CLOSURE, MACRO, SYNTAX, PROC0, PROC1, PROC2, PROC3, PROC4, 
  PROC5, PROCENV, PROCOPT, PROCX, THUNK, PROMISED, PROMISEF, VECTOR, 
  PORTI, PORTO, PORTB, 
  CONTINUATION, VALUES, REGEXPN, HASH, USERDEF
#ifdef WITH_THREADS
  ,THREAD, MUTEX
#endif
} VCSI_OBJECT_TYPE;

typedef struct str_vcsi_object {
  /* the type */
  VCSI_OBJECT_TYPE type;

  /* Garbage collection flag */
  unsigned short gc;

  /* Protected variable */
  unsigned short protect;

  /* Type Union */
  union {
    struct {
      struct str_vcsi_object* car;
      struct str_vcsi_object* cdr;
    } cons;
    struct {
      unsigned char val;
    } boolean;
    struct {
      char c;
    } character;
    struct {
      char* value;
      int len;
    } string;
    struct {
      long value;
    } long_num;
    struct {
      float value;
    } float_num;
    struct {
      int numer;
      int denom;
    } rat_num;
    struct {
      struct str_vcsi_object* vcell;
      struct str_vcsi_object* scope;
      /* Protect certain symbols */
      char* name;
    } symbol;
    struct {
      struct str_vcsi_object* code;
      struct str_vcsi_object* env;
      /* Tracing */
      unsigned short trace;
      unsigned short depth;
    } closure;
    struct {
      struct str_vcsi_object* literals;
      struct str_vcsi_object* patterns;
      struct str_vcsi_object* env;
    } syntax;
    struct {
      char* name;
      struct str_vcsi_object* (*func)();
    } proc;
    struct {
      struct str_vcsi_object* exp;
      struct str_vcsi_object* env;
    } thunk;
    struct {
      struct str_vcsi_object** elem;
      int size;
    } vector;
    struct {
      char* path;
      FILE* fp;
    } port;
    struct {
      struct str_vcsi_continuation* cont;
    } continuation;
    struct {
      struct str_vcsi_object* vals;
    } values;
    struct {
      regex_t* rx_comp;
    } regexp;
#ifdef WITH_THREADS
    struct {
      struct str_vcsi_thread* thread_ptr;
    } thread;
    struct {
      pthread_mutex_t* mutex_ptr;
    } mutex;
#endif
    struct {
      struct str_vcsi_hash_tab** tab;
      unsigned int num_elem;
    } hash;
    struct {
      struct str_vcsi_user_def* ud_type;
      void* ud_value;
    } userdef;       
  } data;

} *VCSI_OBJECT;
#define SIZE_VCSI_OBJECT (sizeof(struct str_vcsi_object))

typedef struct str_vcsi_dynwind {
  VCSI_OBJECT root_env;
  VCSI_OBJECT* stack_start;
  VCSI_OBJECT* stack_end;
  VCSI_OBJECT pre_thunk;   
  VCSI_OBJECT post_thunk;
  sigjmp_buf error_jmp;
  struct str_vcsi_dynwind* next;
} *VCSI_DYNWIND;
#define SIZE_VCSI_DYNWIND (sizeof(struct str_vcsi_dynwind))

typedef enum vcsi_continuation_type {
  CALL_WITH_CC, CALL_WITH_VALUES
} VCSI_CONTINUATION_TYPE;
typedef struct str_vcsi_continuation {
  VCSI_CONTINUATION_TYPE type;

  struct str_vcsi_object* exp;
  struct str_vcsi_object* val;
  long* stack;
  long* stack_start;
  long len;
  sigjmp_buf* jmp;
  struct str_vcsi_dynwind* cont_wind;
} *VCSI_CONTINUATION;
#define SIZE_VCSI_CONTINUATION (sizeof(struct str_vcsi_continuation))

#ifdef WITH_THREADS
typedef enum vcsi_thread_status {
  VCSI_THREAD_NOT_STARTED, VCSI_THREAD_RUNNING, VCSI_THREAD_DONE
} VCSI_THREAD_STATUS;

typedef struct str_vcsi_thread {  
  pthread_t* thread;

  VCSI_THREAD_STATUS status;

  struct str_vcsi_context* vc;
   
  VCSI_OBJECT code;
  VCSI_OBJECT env;
  VCSI_OBJECT retval;
      
  struct str_vcsi_thread* next;
} *VCSI_THREAD;
#define SIZE_VCSI_THREAD (sizeof(struct str_vcsi_thread))
#endif

typedef struct str_vcsi_symbol {
   int ref;
   char* name;
   
   struct str_vcsi_symbol* next;
} *VCSI_SYMBOL;
#define SIZE_VCSI_SYMBOL (sizeof(struct str_vcsi_symbol))

typedef struct str_vcsi_symbol_table {
  VCSI_SYMBOL table;
  unsigned long count;

  unsigned long gensym_num;

#ifdef WITH_THREADS
  pthread_mutex_t* table_mutex;
#endif
} *VCSI_SYMBOL_TABLE;
#define SIZE_VCSI_SYMBOL_TABLE (sizeof(struct str_vcsi_symbol_table))

typedef enum vcsi_context_type {
  VCSI_CONTEXT_MASTER, VCSI_CONTEXT_CLONE
} VCSI_CONTEXT_TYPE;

typedef struct str_vcsi_context {
  VCSI_CONTEXT_TYPE type;
  
  /* The Root Wind */
  VCSI_DYNWIND root_wind;

  /* Global Environments */
  VCSI_OBJECT the_global_env;
  VCSI_OBJECT report_env;
  
  /* The Heap */
  VCSI_OBJECT heap, heap_end;
  long heap_size;
  long gc_alloced, gc_collected;

  /* Master list of all contexts */
  struct str_vcsi_context_list* context_list;

  /* Thread list */
#ifdef WITH_THREADS
  VCSI_THREAD thread_list;

  pthread_mutex_t* gc_critical_mutex;
  pthread_mutex_t* context_mutex;
  pthread_mutex_t* thread_mutex;
  pthread_mutex_t* user_def_mutex;
  pthread_mutex_t* error_mutex;
#endif

  /* The Symbol Table */
  VCSI_SYMBOL_TABLE symbol_table;

  /* Dynamic Modules */
  INFOSTD_HASH_STR dynl_hash;

  /* User defined types */
  INFOSTD_HASH_STR user_def_hash;
  unsigned int user_def_num;

  /* Special Parse Symbols */
  VCSI_OBJECT freeobj;
  VCSI_OBJECT true;
  VCSI_OBJECT false;
  VCSI_OBJECT errobj;
  VCSI_OBJECT elseobj;
  VCSI_OBJECT unbound;
  VCSI_OBJECT novalue;
  VCSI_OBJECT unquote;
  VCSI_OBJECT unquotespl;
  VCSI_OBJECT syntax_rules;
  VCSI_OBJECT elipsis;
  
  /* Special Eval Symbols */
  VCSI_OBJECT specialobj;
  VCSI_OBJECT ifobj;
  VCSI_OBJECT condobj;
  VCSI_OBJECT caseobj;
  VCSI_OBJECT andobj;
  VCSI_OBJECT orobj;
  VCSI_OBJECT letobj;
  VCSI_OBJECT letstarobj;
  VCSI_OBJECT letrecobj;
  VCSI_OBJECT beginobj;
  VCSI_OBJECT doobj;
  VCSI_OBJECT letsynobj;
  VCSI_OBJECT letrecsynobj;

  /* Ports */
  VCSI_OBJECT port_in;
  VCSI_OBJECT port_out;
  VCSI_OBJECT port_curr_in;
  VCSI_OBJECT port_curr_out;
  VCSI_OBJECT eofobj;

  /* REP variables */
  int do_quit;
  int got_error;
  int verbose;
  float secs;

  /* Strings */
  INFOSTD_DYN_STR input;
  INFOSTD_DYN_STR result;
  INFOSTD_DYN_STR res_extra;
  INFOSTD_DYN_STR tmpstr;

  /* Next Context for Clone inheritance*/
  struct str_vcsi_context* next;  
} *VCSI_CONTEXT;
#define SIZE_VCSI_CONTEXT (sizeof(struct str_vcsi_context))

/* A linked-list of contexts */
typedef struct str_vcsi_context_list {  
  struct str_vcsi_context* context;

  struct str_vcsi_context_list* next;
} *VCSI_CONTEXT_LIST;
#define SIZE_VCSI_CONTEXT_LIST (sizeof(struct str_vcsi_context_list))

/* Hashes */
typedef struct str_vcsi_hash_tab {
  VCSI_OBJECT key;
  VCSI_OBJECT info;
  
  struct str_vcsi_hash_tab* next;
} *VCSI_HASH_TAB;
#define SIZE_VCSI_HASH_TAB (sizeof(struct str_vcsi_hash_tab))
#define VCSI_HASH_SIZE 256

typedef struct str_vcsi_user_def {
  int sub_type;

  char* name;

  void (*ud_print)(VCSI_CONTEXT, INFOSTD_DYN_STR, VCSI_OBJECT);
  void (*ud_mark)(VCSI_CONTEXT, VCSI_OBJECT, int);
  void (*ud_free)(VCSI_CONTEXT, VCSI_OBJECT);

  VCSI_OBJECT ud_eval;
} *VCSI_USER_DEF;
#define SIZE_VCSI_USER_DEF (sizeof(struct str_vcsi_user_def))

/* Default Heap Size */
#define VCSI_HEAP_SIZE 100000

/* Memory Functions */
#define MALLOC malloc
#define FREE   free

/* Structure short cuts */
#define TYPE(x) (((x) == NULL) ? 0 : ((*(x)).type))
#define TYPEP(x,y) (TYPE(x) == (y))
#define TYPEN(x,y) (TYPE(x) != (y))

#define CAR(x) ((*x).data.cons.car)
#define CDR(x) ((*x).data.cons.cdr)
#define BOOL(x) ((*x).data.boolean.val)
#define SNAME(x) ((*x).data.symbol.name)
#define VCELL(x) ((*x).data.symbol.vcell)
#define SPROTECT(x) ((*x).protect)
#define SCOPE(x) ((*x).data.symbol.scope)
#define PROC(x) ((*x).data.proc.func)
#define PNAME(x) ((*x).data.proc.name)
#define CHAR(x) ((*x).data.character.c)
#define STR(x) ((*x).data.string.value)
#define SLEN(x) ((*x).data.string.len)
#define FLTN(x) ((*x).data.float_num.value)
#define LNGN(x) ((*x).data.long_num.value)
#define RNUM(x) ((*x).data.rat_num.numer)
#define RDENOM(x) ((*x).data.rat_num.denom)
#define CLSCODE(x) ((*x).data.closure.code)
#define CLSENV(x) ((*x).data.closure.env)
#define CLSTRACE(x) ((*x).data.closure.trace)
#define CLSDEPTH(x) ((*x).data.closure.depth)
#define LITERALS(x) ((*x).data.syntax.literals)
#define PATTERNS(x) ((*x).data.syntax.patterns)
#define SYNENV(x) ((*x).data.syntax.env)
#define THNKEXP(x) ((*x).data.thunk.exp)
#define THNKENV(x) ((*x).data.thunk.env)
#define VSIZE(x) ((*x).data.vector.size)
#define VELEM(x) ((*x).data.vector.elem)
#define PORTP(x) ((*x).data.port.path)
#define PORTFP(x) ((*x).data.port.fp)
#define CONTPTR(x) ((*x).data.continuation.cont)
#define CONTTYPE(x) ((*x).data.continuation.cont->type)
#define CONTEXP(x) ((*x).data.continuation.cont->exp)
#define CONTVAL(x) ((*x).data.continuation.cont->val)
#define CONTSTCK(x) ((*x).data.continuation.cont->stack)
#define CONTSTCKS(x) ((*x).data.continuation.cont->stack_start)
#define CONTLEN(x) ((*x).data.continuation.cont->len)
#define CONTJMP(x) ((*x).data.continuation.cont->jmp)
#define CONTWIND(x) ((*x).data.continuation.cont->cont_wind)
#define VVALS(x) ((*x).data.values.vals)
#define REGEXPNV(x)  ((*x).data.regexp.rx_comp)

#define HASHT(x)  ((*x).data.hash.tab)
#define HASHN(x)  ((*x).data.hash.num_elem)

#define UDTYPE(x)  ((*x).data.userdef.ud_type)
#define UDVAL(x)  ((*x).data.userdef.ud_value)


#ifdef WITH_THREADS
#define THREADPTR(x) ((*x).data.thread.thread_ptr)
#define MUTEXPTR(x) ((*x).data.mutex.mutex_ptr)
#endif

#define DYNROOT(x) ((*x).root_env)
#define DYNSTCK(x) ((*x).stack_start)
#define DYNSTCKE(x) ((*x).stack_end)
#define DYNERR(x) ((*x).error_jmp)
#define DYNPRE(x) ((*x).pre_thunk)
#define DYNPOST(x) ((*x).post_thunk)
#define DYNNEXT(x) ((*x).next)

#define EQ(x,y) ((x) == (y))
#define NEQ(x,y) ((x) != (y))

#include "net.h"
#include "thread.h"
#include "regexp.h"

#include "obj.h"
#include "cont.h"
#include "dynl.h"
#include "env.h"
#include "eval.h"

#include "sym.h"
#include "str.h"
#include "num.h"
#include "prim.h"
#include "vec.h"
#include "port.h"

#include "gc.h"
#include "lex.h"

#include "srfi.h"

extern char* vcsi_version;

int pmatchq(char* s);
int get_pmatch(VCSI_CONTEXT vc, FILE* f);

/* Verify the type of argument */
void check_arg_type(VCSI_CONTEXT vc,
		    VCSI_OBJECT x, 
		    VCSI_OBJECT_TYPE type, 
		    char* func_name);

/* Check that an argument is a String or Symbol
   and return a pointer to its char data */
char* check_arg_string_like(VCSI_CONTEXT vc,
			    VCSI_OBJECT x,
			    char* func_name);

/* The size of the free list */
long get_free_length(VCSI_CONTEXT vc);

/* Load a library file */
VCSI_OBJECT load_library_file(VCSI_CONTEXT vc, char* str);

/* Load a scheme file from a string filename*/
VCSI_OBJECT load_file_handle(VCSI_CONTEXT vc,
			     FILE* f);

/* Print an object into result, with option clearing */
char* print_obj(VCSI_CONTEXT vc, 
		INFOSTD_DYN_STR result, 
		VCSI_OBJECT item, 
		int clear);

/* Quit */
VCSI_OBJECT quit(VCSI_CONTEXT vc);

/* Initialization and utilization functions */
VCSI_CONTEXT vcsi_init(unsigned long hs);
VCSI_CONTEXT vcsi_clone_context(VCSI_CONTEXT vc);
void vcsi_context_free(VCSI_CONTEXT vc);
VCSI_CONTEXT vcsi_context_find_master(VCSI_CONTEXT vc);

char* rep(VCSI_CONTEXT vc, char* input);
void rep_loop(VCSI_CONTEXT vc);

/*
 * (define (fact n) (if (<= n 0) 1 (* n (fact (- n 1)))))
 * (define (facti a n) (if (= n 0) a (facti (* a n) (- n 1))))
 * (define (fib n) (if (> 2 n) n (+ (fib (- n 2)) (fib (- n 1)))))
 * (define (map f l) (if (null? l) '() (cons (f (car l)) (map f (cdr l)))))
 * (define (rev x) (if (null? x) x (append (rev (cdr x))  (list (car x)))))
 * (define (revi ls sl) (if (null? ls) sl (revi (cdr ls) (cons (car ls) sl))))
 * (define (sum x) (if (null? x)  0 (+ (car x) (sum (cdr x)))))
 * (define (sumi x y) (if (null? x) y (sumi (cdr x) (+ (car x) y))))
 * (define (sumc x y) (cond ((null? x) y) (else (sumi (cdr x) (+ (car x) y)))))
 */

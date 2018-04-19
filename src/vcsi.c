#include "vcsi.h"

/* Global Variables */
char* vcsi_version = "3.0.5";
char* vcsi_build_date = __DATE__ ;
char* vcsi_build_time = __TIME__ ;

int pmatchq(char* s) {
  int paren = 0;
  int i,l;

  l = strlen(s);

  if(l == 0)
    return 0;

  for(i=0;i<l;i++) {
    if(s[i] == ';') {
      s[i] = 0;
      break;
    } else if(s[i] == '\\') /* escaped characters */
      i++;
    else if(s[i] == '(')
      paren++;
    else if(s[i] == ')') {
      if(paren > 0)
	paren--;
      else
	break;
    } else if(s[i] == '"')
      for(i++;s[i] != '"' && i<l;i++) {
	if(s[i] == '\\') /* escaped characters */
	  i++;
	/* no semicolon handling in strings */
	/*else if(s[i] == ';') {
	  s[i] = 0;
	  i = l;
	  break;
	  }*/
      }

  }

  return (paren == 0) ? 1 : 0;
}

int get_pmatch(VCSI_CONTEXT vc, FILE* f) {
  int c = 0;
  int in_paren = -1;
  int in_str = 0;
  int in_com = 0;
  int esc = 0;
  int last_c = -1;

  infostd_dyn_str_clear(vc->input);

  if(vc->do_quit)
    return -1;

  for(c=fgetc(f);in_paren != 0;c=fgetc(f)) {
    if(c == -1) {
      if(vc->input->length)
	return 0;
      return EOF;
    }

    if(c == 10) {
      in_com = 0;

      if(vc->input->length > 0) {
	if(in_paren <= 0)
	  break;
	c = ' ';
      } else
	continue;
    }

    if(c == ';' && !in_str)
      in_com = 1;

    if(in_com)
      continue;

    if(c == ' ' && vc->input->length != 0 && !in_str && in_paren == -1)
      return 0;

    else if(c == '\"' && !esc)
      in_str = !in_str;
    else if(c == '\\')
      esc = 1;
    else if(c == '(' && !in_str && !esc)
      (in_paren == -1) ? in_paren = 1 :in_paren++;
    else if(c == ')' && !in_str && !esc) {
      if(in_paren > 0)
	in_paren--;
      else
	return 0;
    } else if(iscntrl(c) && !in_str)
      c = ' ';
    if(esc && c != '\\')
      esc = 0;

    if(in_str || !(c == ' ' && last_c == ' ')) /* ignore multiple spaces */
      infostd_dyn_str_addchar(vc->input,c);
    last_c = c;
  }
  return 0;
}

void print_vcsi_object_size(VCSI_CONTEXT vc) {

  printf("VCSI_OBJECT: %d\n",SIZE_VCSI_OBJECT);
  printf("VCSI_OBJECT TYPE: %d\n",sizeof(vc->true->type));
  printf("VCSI_OBJECT DATA: %d\n",sizeof(vc->true->data));
  printf("CONS: %d\n",sizeof(vc->true->data.cons));
  printf("BOOLEAN: %d\n",sizeof(vc->true->data.boolean));
  printf("CHAR: %d\n",sizeof(vc->true->data.character));
  printf("STRING: %d\n",sizeof(vc->true->data.string));
  printf("LONG: %d\n",sizeof(vc->true->data.long_num));
  printf("FLOAT: %d\n",sizeof(vc->true->data.float_num));
  printf("RAT: %d\n",sizeof(vc->true->data.rat_num));
  printf("SYMBOL: %d\n",sizeof(vc->true->data.symbol));
  printf("CLOSURE: %d\n",sizeof(vc->true->data.closure));
  printf("SYNTAX: %d\n",sizeof(vc->true->data.syntax));
  printf("PROC: %d\n",sizeof(vc->true->data.proc));
  printf("THUNK: %d\n",sizeof(vc->true->data.thunk));
  printf("VECTOR: %d\n",sizeof(vc->true->data.vector));
  printf("PORT: %d\n",sizeof(vc->true->data.port));
  printf("CONT: %d\n",sizeof(vc->true->data.continuation));
  printf("VAL: %d\n",sizeof(vc->true->data.values));
  printf("REGEXPN: %d\n",sizeof(vc->true->data.regexp));
#ifdef WITH_THREADS
  printf("THREAD: %d\n",sizeof(vc->true->data.thread));
  printf("MUTEX: %d\n",sizeof(vc->true->data.mutex));
#endif
  printf("HASH: %d\n",sizeof(vc->true->data.hash));
  printf("USERDEF: %d\n",sizeof(vc->true->data.userdef));
}

/* Print an Object and all embedded Objects */
char* print_obj(VCSI_CONTEXT vc,
		INFOSTD_DYN_STR result,
		VCSI_OBJECT item,
		int clear) {

  VCSI_OBJECT tmp;
  unsigned long l;
  int i;

  infostd_dyn_str_clear(vc->res_extra);

  if(clear)
    infostd_dyn_str_clear(result);

  if(vc->got_error)
    item = vc->errobj;

  switch TYPE(item) {
  case 0:
    infostd_dyn_str_strcat(result,"()");
    break;
  case BOOLEAN:
    if(BOOL(item))
      infostd_dyn_str_strcat(result,"#t");
    else
      infostd_dyn_str_strcat(result,"#f");
    break;
  case LNGNUM:
    infostd_dyn_str_printf(vc->res_extra,"%ld",LNGN(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case FLTNUM:
    if(FLTN(item) == (long)(FLTN(item)))
      infostd_dyn_str_printf(vc->res_extra,"%g.",FLTN(item));
    else /* %#g */
      infostd_dyn_str_printf(vc->res_extra,"%g",FLTN(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case RATNUM:
    infostd_dyn_str_printf(vc->res_extra,"%d/%d",RNUM(item),RDENOM(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case CHAR:
    if(CHAR(item) == ' ')
      infostd_dyn_str_printf(vc->res_extra,"#\\Space");
    else if(CHAR(item) == '\n')
      infostd_dyn_str_printf(vc->res_extra,"#\\Linefeed");
    else
      infostd_dyn_str_printf(vc->res_extra,"#\\%c",CHAR(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case STRING:
    infostd_dyn_str_addchar(vc->res_extra,'\"');
    for(l=0;l<SLEN(item);l++) {
      if(STR(item)[l] == '\"' || STR(item)[l] == '\\')
	infostd_dyn_str_addchar(vc->res_extra,'\\');
      infostd_dyn_str_addchar(vc->res_extra,STR(item)[l]);
    }
    infostd_dyn_str_addchar(vc->res_extra,'\"');
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case SYMBOL:
    if(item == vc->specialobj)
      infostd_dyn_str_printf(vc->res_extra,"#[%s]",SNAME(item));
    else if(SCOPE(item) && 0) /* show different scoped vars */
      infostd_dyn_str_printf(vc->res_extra,"&%s",SNAME(item));
    else
      infostd_dyn_str_printf(vc->res_extra,"%s",SNAME(item));

    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case PROC0:
  case PROC1:
  case PROC2:
  case PROC3:
  case PROCENV:
  case PROCOPT:
  case PROCX:
    infostd_dyn_str_printf(vc->res_extra,"#[primitive-procedure %s]",
			   item->data.proc.name);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case CLOSURE:
    infostd_dyn_str_strcat(result,"#[closure ");
    if(vc->verbose) {
      print_obj(vc,result,CAR(CLSCODE(item)),0);
      infostd_dyn_str_strcat(result," ");
      print_obj(vc,result,CAR(CDR(CLSCODE(item))),0);
    } else {
      infostd_dyn_str_printf(vc->res_extra,"%p",item);
      infostd_dyn_str_strcat(result,vc->res_extra->buff);
    }
    infostd_dyn_str_strcat(result,"]");
    break;
  case MACRO:
    infostd_dyn_str_strcat(result,"#[macro ");
    if(vc->verbose) {
      print_obj(vc,result,CAR(CLSCODE(item)),0);
      infostd_dyn_str_strcat(result," ");
      print_obj(vc,result,CAR(CDR(CLSCODE(item))),0);
    } else {
      infostd_dyn_str_printf(vc->res_extra,"%p",item);
      infostd_dyn_str_strcat(result,vc->res_extra->buff);
    }
    infostd_dyn_str_strcat(result,"]");
    break;
  case SYNTAX:
    infostd_dyn_str_strcat(result,"#[syntax ");
    if(vc->verbose) {
      print_obj(vc,result,LITERALS(item),0);
      infostd_dyn_str_strcat(result," ");
      print_obj(vc,result,PATTERNS(item),0);
    } else {
      infostd_dyn_str_printf(vc->res_extra,"%p",item);
      infostd_dyn_str_strcat(result,vc->res_extra->buff);
    }
    infostd_dyn_str_strcat(result,"]");
    break;
  case PROMISED:
  case PROMISEF:
    infostd_dyn_str_printf(vc->res_extra,"#[promise %p]",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case PORTI:
    infostd_dyn_str_printf(vc->res_extra,"#[input-port for %s]",PORTP(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case PORTO:
    infostd_dyn_str_printf(vc->res_extra,"#[output-port for %s]",
			   PORTP(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case PORTB:
    infostd_dyn_str_printf(vc->res_extra,"#[i/o-port for %s]",PORTP(item));
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case VECTOR:
    infostd_dyn_str_strcat(result,"#(");
    for(i=0;i<VSIZE(item);i++) {
      if(i != 0 && i != VSIZE(item))
	infostd_dyn_str_strcat(result," ");
      print_obj(vc,result, VELEM(item)[i],0);
    }
    infostd_dyn_str_strcat(result,")");
    break;
  case CONTINUATION:
    infostd_dyn_str_printf(vc->res_extra,"#[continuation %p]",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case VALUES:
    infostd_dyn_str_printf(vc->res_extra,"#[values %p ",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    print_obj(vc,result,VVALS(item),0);
    infostd_dyn_str_strcat(result,"]");
    break;
#ifdef WITH_THREADS
  case THREAD:
    infostd_dyn_str_printf(vc->res_extra,"#[thread %p (%s)]",item,
			   (THREADPTR(item)->status == VCSI_THREAD_NOT_STARTED) ?
			   "not started" : (THREADPTR(item)->status == VCSI_THREAD_RUNNING)
			   ? "running" : "done");
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case MUTEX:
    infostd_dyn_str_printf(vc->res_extra,"#[mutex %p]",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
#endif
  case REGEXPN:
    infostd_dyn_str_printf(vc->res_extra,"#[regexp %p]",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case CONS:
    infostd_dyn_str_strcat(result,"(");
    for(tmp=item;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
      print_obj(vc,result, CAR(tmp),0);
      if(CDR(tmp) != NULL)
	infostd_dyn_str_strcat(result," ");
    }
    if(TYPEN(tmp,0)) {
      infostd_dyn_str_strcat(result,". ");
      print_obj(vc,result, tmp,0);
    }
    infostd_dyn_str_strcat(result,")");
    break;
  case HASH:
    infostd_dyn_str_printf(vc->res_extra,"#[hash %p]",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  case USERDEF:
    if(UDTYPE(item) == NULL) {
      infostd_dyn_str_printf(vc->res_extra,"#[user-defined %p]",item);
      infostd_dyn_str_strcat(result,vc->res_extra->buff);
    } else {
      if(UDTYPE(item)->ud_print)
	UDTYPE(item)->ud_print(vc,result,item);
      else {
	infostd_dyn_str_printf(vc->res_extra,"#[user-defined %s]",
			       UDTYPE(item)->name);
	infostd_dyn_str_strcat(result,vc->res_extra->buff);
      }
    }
    break;
  default:
    infostd_dyn_str_printf(vc->res_extra,"#[unknown-type %p]",item);
    infostd_dyn_str_strcat(result,vc->res_extra->buff);
    break;
  }
  return result->buff;
}

VCSI_OBJECT load_file(VCSI_CONTEXT vc,
		      VCSI_OBJECT x) {
  FILE* fh;
  VCSI_OBJECT res;

  if(!TYPEP(x,STRING))
    return error(vc,"object passed to load is the wrong type",x);
  if(SLEN(x) == 0)
    return error(vc,"no such file",NULL);

  infostd_dyn_str_strcpy(vc->tmpstr, STR(x));
  fh = fopen(vc->tmpstr->buff,"r");

  if(fh == NULL) {
    infostd_dyn_str_strcat(vc->tmpstr,".scm");
    fh = fopen(vc->tmpstr->buff,"r");
    if(fh == NULL)
      return error(vc,"can't open file", x);
  }

  res = load_file_handle(vc, fh);
  fclose(fh);

  return res;
}

VCSI_OBJECT load_file_handle(VCSI_CONTEXT vc,
			     FILE* f) {
  VCSI_OBJECT tmp = NULL;
  VCSI_OBJECT s2;

  /* Clone the VC */
  vc = vcsi_clone_context(vc);
  DYNSTCKE(vc->root_wind) = &s2;

  while(get_pmatch(vc,f) != EOF) {
    vc->got_error = 0;

    if(!vc->input->length)
      continue;

    if(!sigsetjmp(DYNERR(vc->root_wind),1)) {
      tmp = eval(vc,parse_text(vc,vc->input->buff),DYNROOT(vc->root_wind));
    }

    infostd_dyn_str_clear(vc->input);

    if(errorq(vc,tmp))
      return vc->errobj;
  }

  vcsi_context_free(vc);

  return tmp;
}

VCSI_OBJECT load_library_file(VCSI_CONTEXT vc,
			      char* str) {
  FILE* f;
  VCSI_OBJECT tmp = NULL;
  VCSI_OBJECT s2;

#ifdef VCSI_LIB_DIR
  infostd_dyn_str_strcpy(vc->tmpstr,VCSI_LIB_DIR);
  infostd_dyn_str_strcat(vc->tmpstr,str);
#else
  infostd_dyn_str_strcpy(vc->tmpstr,"./lib/");
  infostd_dyn_str_strcat(vc->tmpstr,str);
#endif

  f = fopen(vc->tmpstr->buff,"r");

  if(f == NULL) {
    fprintf(stderr,"Unable to open library file %s\n",vc->tmpstr->buff);
    exit(0);
  }

  /* Clone the VC */
  vc = vcsi_clone_context(vc);
  DYNSTCKE(vc->root_wind) = &s2;

  while(get_pmatch(vc,f) != EOF) {
    vc->got_error = 0;

    if(!vc->input->length)
      continue;

    if(!sigsetjmp(DYNERR(vc->root_wind),1)) {
      tmp = eval(vc,parse_text(vc,vc->input->buff),DYNROOT(vc->root_wind));
    }

    infostd_dyn_str_clear(vc->input);

    if(errorq(vc,tmp)) {
      fprintf(stderr,"Error parsing library file %s\n",vc->tmpstr->buff);
      exit(0);
    }
  }

  fclose(f);

  vcsi_context_free(vc);

  return tmp;
}

void check_arg_type(VCSI_CONTEXT vc,
		    VCSI_OBJECT x,
		    VCSI_OBJECT_TYPE type,
		    char* func_name) {
  char buf[256];

  if(!TYPEP(x,type)) {
    snprintf(buf,256,"object passed to %s is the wrong type",func_name);
    error(vc,buf,x);
  }
}

char* check_arg_string_like(VCSI_CONTEXT vc,
			    VCSI_OBJECT x,
			    char* func_name) {

  char buf[256];

  if(!TYPEP(x,STRING) && !TYPEP(x,SYMBOL)) {
    snprintf(buf,256,"object passed to %s is the wrong type",func_name);
    error(vc,buf,x);
  }
  return (TYPEP(x,SYMBOL)) ? SNAME(x) : STR(x);
}

long get_free_length(VCSI_CONTEXT vc) {
  VCSI_OBJECT x;
  int count = 1;

  /* Use the master context */
  vc=vcsi_context_find_master(vc);

  for(x=vc->freeobj;TYPEP(x,FREED);x=CDR(x))
    count++;
  return count;
}

VCSI_OBJECT quit(VCSI_CONTEXT vc) {
  fprintf(PORTFP(vc->port_curr_out),"EOF\n");

  /* Kill the whole context */
  vcsi_context_free(vc);

  /* Exit */
  exit(1);

  return NULL;
}

VCSI_OBJECT verbosity(VCSI_CONTEXT vc,
		      VCSI_OBJECT x) {

  VCSI_OBJECT lvl;

  if(x != vc->novalue) {
    lvl = x;
    check_arg_type(vc,lvl,LNGNUM,"verbosity");

    vc->verbose = LNGN(lvl);

    if(vc->verbose < 0)
      vc->verbose = 0;
    else if(vc->verbose > 2)
      vc->verbose = 2;

  } else {
    if(vc->verbose == 0)
      vc->verbose = 1;
    else if(vc->verbose == 1)
      vc->verbose = 2;
    else
      vc->verbose = 0;
  }

  return make_long(vc,vc->verbose);
}

VCSI_OBJECT do_gc(VCSI_CONTEXT vc) {

  VCSI_OBJECT stck_end, *stck_old;

  /* save new stack pointers */
  stck_old = DYNSTCKE(vc->root_wind);
  DYNSTCKE(vc->root_wind) = &stck_end;

  /* Use the master context */
  gc_collect(vcsi_context_find_master(vc));

  /* restore the old stack pointers */
  DYNSTCKE(vc->root_wind) = stck_old;

  return vc->true;
}

VCSI_OBJECT gc_stats(VCSI_CONTEXT vc) {
  VCSI_OBJECT tmp, tmp1, tmp2, tmp3, tmp4, x, y, z;

  /* Use the master context */
  vc = vcsi_context_find_master(vc);

  x = make_long(vc,0);
  y = make_long(vc,0);
  z = make_long(vc,0);


  tmp1 = cons(vc,make_symbol(vc,"cells-allocated"),x);

  tmp2 = cons(vc,make_symbol(vc,"cells-collected"),y);

  tmp3 = cons(vc,make_symbol(vc,"cells-available"),z);

  tmp4 = cons(vc,make_symbol(vc,"heap-size"),make_long(vc,vc->heap_size));

  tmp = cons(vc,tmp1,cons(vc,tmp2,cons(vc,tmp3,cons(vc,tmp4,NULL))));

  /* So that it actually reports accurate values */
  LNGN(x) = vc->gc_alloced;
  LNGN(y) = vc->gc_collected;
  LNGN(z) = get_free_length(vc);

  return tmp;
}


/* VCSI_CONTEXT functions */
VCSI_CONTEXT vcsi_init(unsigned long hs) {
  VCSI_OBJECT tmp;

  VCSI_CONTEXT vc;

  /* Create the context */
  vc = (VCSI_CONTEXT)MALLOC(SIZE_VCSI_CONTEXT);
  memset(vc,0,SIZE_VCSI_CONTEXT);

  /* Set the type */
  vc->type = VCSI_CONTEXT_MASTER;

  /* No sub-contexts */
  vc->next = NULL;

  /* Create Input and Output buffers */
  vc->input = infostd_dyn_str_init(1024);
  vc->result = infostd_dyn_str_init(1024);
  vc->res_extra = infostd_dyn_str_init(100);
  vc->tmpstr = infostd_dyn_str_init(100);

  /* Create Hashes for User Defined Types and Dynamic Modules */
  vc->user_def_hash = infostd_hash_str_init(100);
  vc->dynl_hash = infostd_hash_str_init(100);

  /* Create the Symbol Table */
  vc->symbol_table = symbol_table_init();

#ifdef WITH_THREADS
  /* Create the mutexes */
  vc->gc_critical_mutex=(pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
  vc->context_mutex=(pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
  vc->thread_mutex=(pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
  vc->user_def_mutex=(pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
  vc->error_mutex=(pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
  vc->symbol_table->table_mutex=
    (pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));

  pthread_mutex_init(vc->gc_critical_mutex,NULL);
  pthread_mutex_init(vc->context_mutex,NULL);
  pthread_mutex_init(vc->thread_mutex,NULL);
  pthread_mutex_init(vc->user_def_mutex,NULL);
  pthread_mutex_init(vc->error_mutex,NULL);
  pthread_mutex_init(vc->symbol_table->table_mutex,NULL);
#endif

  /* Create the Heap */
  vc->heap_size = hs;
  if(vc->heap_size==0)
    vc->heap_size = VCSI_HEAP_SIZE;

  vc->heap = malloc(SIZE_VCSI_OBJECT * vc->heap_size);
  if(!vc->heap) {
    fprintf(stderr,"Unable to create VCSI Heap!\n");
    exit(1);
  }
  memset(vc->heap,0,SIZE_VCSI_OBJECT * vc->heap_size);

  /* Set the first element of the heap to be a reference
   * to the current location */
  vc->heap_end = vc->heap + (vc->heap_size-1);
  vc->heap = vc->heap;

  /* Setup the list of free objects */
  for(tmp=vc->heap;tmp<vc->heap_end;tmp++) {
    tmp->type = FREED;
    CDR(tmp) = tmp+1;
  }
  vc->heap_end->type = FREED;
  CDR(vc->heap_end) = NULL;
  vc->freeobj = vc->heap;

  /* Now setup our root of the wind chain */
  vc->root_wind = (VCSI_DYNWIND)MALLOC(SIZE_VCSI_DYNWIND);
  memset(vc->root_wind,0,SIZE_VCSI_DYNWIND);

  /* Set the global env to be the first element, then setup primitives */
  vc->the_global_env = alloc_obj(vc,CONS);
  CAR(vc->the_global_env) = NULL;
  CDR(vc->the_global_env) = NULL;
  DYNROOT(vc->root_wind) = vc->the_global_env;

  /* Create Permanent Booleans */
  vc->true = MALLOC(SIZE_VCSI_OBJECT);
  memset(vc->true,0,SIZE_VCSI_OBJECT);
  vc->true->type = BOOLEAN;
  BOOL(vc->true)=1;

  vc->false = MALLOC(SIZE_VCSI_OBJECT);
  memset(vc->false,0,SIZE_VCSI_OBJECT);
  vc->false->type = BOOLEAN;
  BOOL(vc->false)=0;

  /* Create Protected Objects */
  vc->elseobj = make_protected("else");
  vc->unbound = make_protected("#[unbound symbol]");
  vc->novalue = make_protected("#[unspecified]");
  vc->unquote = make_protected("unquote");
  vc->unquotespl = make_protected("unquote-splicing");
  vc->specialobj = make_protected("special form");
  vc->syntax_rules = make_protected("syntax-rules");
  vc->elipsis = make_protected("...");

  /* Initialize all core functions */
  set_int_proc(vc,"eval",PROC2,do_eval);
  set_int_proc(vc,"scheme-report-environment",PROC1,get_rep_env);
  set_int_proc(vc,"null-environment",PROC1,get_rep_env);
  set_int_proc(vc,"interaction-environment",PROCENV,get_int_env);

  set_int_proc(vc,"cons",PROC2,cons);
  set_int_proc(vc,"car",PROC1,car);
  set_int_proc(vc,"cdr",PROC1,cdr);
  set_int_proc(vc,"cdar",PROC1,cdar);
  set_int_proc(vc,"cddr",PROC1,cddr);
  set_int_proc(vc,"cadr",PROC1,cadr);
  set_int_proc(vc,"caar",PROC1,caar);
  set_int_proc(vc,"caaar",PROC1,caaar);
  set_int_proc(vc,"caadr",PROC1,caadr);
  set_int_proc(vc,"cadar",PROC1,cadar);
  set_int_proc(vc,"cdaar",PROC1,cdaar);
  set_int_proc(vc,"cddar",PROC1,cddar);
  set_int_proc(vc,"caddr",PROC1,caddr);
  set_int_proc(vc,"cdadr",PROC1,cdadr);
  set_int_proc(vc,"cdddr",PROC1,cdddr);
  set_int_proc(vc,"cddddr",PROC1,cddddr);
  set_int_proc(vc,"cadddr",PROC1,cadddr);
  set_int_proc(vc,"cddadr",PROC1,cddddr);
  set_int_proc(vc,"cadadr",PROC1,cadadr);
  set_int_proc(vc,"cdaddr",PROC1,cdaddr);
  set_int_proc(vc,"caaddr",PROC1,caaddr);
  set_int_proc(vc,"cdddar",PROC1,cdddar);
  set_int_proc(vc,"caddar",PROC1,caddar);
  set_int_proc(vc,"cddaar",PROC1,cddaar);
  set_int_proc(vc,"cadaar",PROC1,cadaar);
  set_int_proc(vc,"cdadar",PROC1,cdadar);
  set_int_proc(vc,"caadar",PROC1,caadar);
  set_int_proc(vc,"cdaadr",PROC1,cdaadr);
  set_int_proc(vc,"caaadr",PROC1,caaadr);
  set_int_proc(vc,"cdaaar",PROC1,cdaaar);
  set_int_proc(vc,"caaaar",PROC1,caaaar);

  set_int_proc(vc,"list",PROCOPT,eval_list);
  set_int_proc(vc,"list-ref",PROC2,vcsi_list_ref);
  set_int_proc(vc,"list-tail",PROC2,vcsi_list_tail);

  set_int_proc(vc,"define",PROCENV,eval_define);
  set_int_proc(vc,"lambda",PROCENV,eval_lambda);
  set_int_proc(vc,"quote",PROCENV,eval_quoted);
  set_int_proc(vc,"quasiquote",PROCENV,eval_quasiquote);

  set_int_proc(vc,"define-macro",PROCENV,eval_define_macro);
  set_int_proc(vc,"define-syntax",PROCENV,eval_define_syntax);
  vc->letsynobj = make_special(vc,"let-syntax");
  vc->letrecsynobj = make_special(vc,"letrec-syntax");

  vc->beginobj = make_special(vc,"begin");
  vc->doobj = make_special(vc,"do");

  set_int_proc(vc,"error",PROCENV,do_error);

  vc->letobj = make_special(vc,"let");
  vc->letstarobj = make_special(vc,"let*");
  vc->letrecobj = make_special(vc,"letrec");

  vc->caseobj = make_special(vc,"case");
  set_int_proc(vc,"map",PROCENV,eval_map);
  set_int_proc(vc,"for-each",PROCENV,eval_foreach);
  set_int_proc(vc,"apply",PROCENV,eval_apply);

  set_int_proc(vc,"delay",PROCENV,eval_delay);
  set_int_proc(vc,"force",PROC1,eval_force);


  vc->ifobj = make_special(vc,"if");
  vc->condobj = make_special(vc,"cond");
  vc->andobj = make_special(vc,"and");
  vc->orobj = make_special(vc,"or");
  set_int_proc(vc,"not",PROC1,eval_not);

  set_int_proc(vc,"memq",PROC2,memq);
  set_int_proc(vc,"memv",PROC2,memq);
  set_int_proc(vc,"member",PROC2,member);

  set_int_proc(vc,"equal?",PROC2,equal);
  set_int_proc(vc,"eq?",PROC2,eqv);
  set_int_proc(vc,"eqv?",PROC2,eqv);

  set_int_proc(vc,"assq",PROC2,assq);
  set_int_proc(vc,"assv",PROC2,assq);
  set_int_proc(vc,"assoc",PROC2,assoc);

  set_int_proc(vc,"cmp",PROC2,obj_compare);

  set_int_proc(vc,"pair?",PROC1,pairq);
  set_int_proc(vc,"null?",PROC1,nullq);
  set_int_proc(vc,"boolean?",PROC1,booleanq);
  set_int_proc(vc,"procedure?",PROC1,procq);
  set_int_proc(vc,"list?",PROC1,listq);

  set_int_proc(vc,"set!",PROCENV,eval_set);
  set_int_proc(vc,"set-car!",PROCENV,eval_setcar);
  set_int_proc(vc,"set-cdr!",PROCENV,eval_setcdr);

  set_int_proc(vc,"length",PROC1,vcsi_list_length);
  set_int_proc(vc,"reverse",PROC1,vcsi_list_reverse);
  set_int_proc(vc,"append",PROC2,vcsi_list_append);

  set_int_proc(vc,"load",PROC1,load_file);

  vc->errobj = make_internal(vc,"errobj");
  vc->got_error = 0;

  cont_init(vc);
  sym_init(vc);
  str_init(vc);
  num_init(vc);
  port_init(vc);
  vec_init(vc);

  /* Grab the scheme report environment */
  vc->report_env = CAR(DYNROOT(vc->root_wind));
  vc->report_env = cons(vc,vc->report_env,NULL);

  /* Add custom functions */
  set_int_proc(vc,"random",PROCOPT,randobj);
  set_int_proc(vc,"time",PROC0,get_time);
  set_int_proc(vc,"localtime",PROC1,get_localtime);
  set_int_proc(vc,"localtime->list",PROC1,get_localtime_list);
  set_int_proc(vc,"sleep",PROC1,do_sleep);
  set_int_proc(vc,"usleep",PROC1,do_usleep);
  set_int_proc(vc,"quit",PROC0,quit);
  set_int_proc(vc,"exit",PROC0,quit);
  set_int_proc(vc,"verbose",PROCX,verbosity);
  set_int_proc(vc,"gc",PROC0,do_gc);
  set_int_proc(vc,"gc-stats",PROC0,gc_stats);
  set_int_proc(vc,"trace",PROC1,clstrace);
  set_int_proc(vc,"untrace",PROC1,clsuntrace);

  set_int_proc(vc,"sort",PROC1,obj_sort);

  /* Hashing functions */
  set_int_proc(vc,"make-hash",PROC0,make_hash);
  set_int_proc(vc,"hash-add",PROC3,hash_add);
  set_int_proc(vc,"hash-set!",PROC3,hash_add);
  set_int_proc(vc,"hash-lookup",PROC2,hash_lookup);
  set_int_proc(vc,"hash-delete",PROC2,hash_delete);
  set_int_proc(vc,"hash-keys",PROC1,hash_keys);

  /* Load custom libraries */
#ifdef WITH_DYNAMIC_LOAD
  dynl_init(vc);
#endif

  net_init(vc);
  regexp_init(vc);
  str_ext_init(vc);

#ifdef WITH_THREADS
  thread_init(vc);
#endif

  /* Load Scheme Request for Implementation Libraries */
  srfi_init(vc);

  /* Seed the random number generator */
  srand(time(NULL));

  /* Protect Syntactic Variables */
  protect_var(vc,"quote");
  protect_var(vc,"lambda");
  protect_var(vc,"if");
  protect_var(vc,"set!");
  protect_var(vc,"begin");
  protect_var(vc,"cond");
  protect_var(vc,"and");
  protect_var(vc,"or");
  protect_var(vc,"case");
  protect_var(vc,"let");
  protect_var(vc,"let*");
  protect_var(vc,"letrec");
  protect_var(vc,"do");
  protect_var(vc,"delay");
  protect_var(vc,"quasiquote");
  protect_var(vc,"define");

  /* Collect any Garbage Created in Initialization */
  gc_collect(vc);

  /* Return the context */
  return vc;
}

VCSI_CONTEXT vcsi_clone_context(VCSI_CONTEXT vc) {
  VCSI_CONTEXT_LIST tmp;
  VCSI_CONTEXT vn;

  if(!vc)
    return NULL;

  /* Create the new context */
  vn = (VCSI_CONTEXT)MALLOC(SIZE_VCSI_CONTEXT);
  memset(vn,0,SIZE_VCSI_CONTEXT);

  /* Copy the old Context */
  memcpy(vn,vc,SIZE_VCSI_CONTEXT);

  /* Set the type */
  vn->type = VCSI_CONTEXT_CLONE;

  /* Create Context Chain */
  vn->next = vc;

  /* Create a new Dynamic Wind */
  vn->root_wind = (VCSI_DYNWIND)MALLOC(SIZE_VCSI_DYNWIND);
  memset(vn->root_wind,0,SIZE_VCSI_DYNWIND);

  /* Clone the old Wind */
  DYNROOT(vn->root_wind) = DYNROOT(vc->root_wind);
  DYNPRE(vn->root_wind) = DYNPRE(vc->root_wind);
  DYNPOST(vn->root_wind) = DYNPOST(vc->root_wind);
  /* Save the start of the stack */
  DYNSTCK(vn->root_wind) = DYNSTCK(vc->root_wind);

  /* Create private Input and Output buffers */
  vc->input = infostd_dyn_str_init(1024);
  vc->result = infostd_dyn_str_init(1024);
  vc->res_extra = infostd_dyn_str_init(100);
  vc->tmpstr = infostd_dyn_str_init(100);

  /* Reset variables */
  vn->do_quit=0;
  vn->got_error=0;

  /* Add it to the Context List */
  tmp = (VCSI_CONTEXT_LIST)MALLOC(SIZE_VCSI_CONTEXT_LIST);
  memset(tmp,0,SIZE_VCSI_CONTEXT_LIST);

  tmp->context = vn;

#ifdef WITH_THREADS
  pthread_mutex_lock(vc->context_mutex);
#endif
  tmp->next = vc->context_list;
  vc->context_list = tmp;
#ifdef WITH_THREADS
  pthread_mutex_unlock(vc->context_mutex);
#endif

  /* Return the new context */
  return vn;
}

void vcsi_context_free(VCSI_CONTEXT vc) {
  VCSI_CONTEXT vm;
  VCSI_CONTEXT_LIST ltmp, lprev;
#ifdef WITH_THREADS
  VCSI_THREAD tmp;
#endif

  if(!vc)
    return;

  if(vc->type == VCSI_CONTEXT_MASTER) {

    /* Free Buffers */
    infostd_dyn_str_free(vc->input);
    infostd_dyn_str_free(vc->result);
    infostd_dyn_str_free(vc->res_extra);
    infostd_dyn_str_free(vc->tmpstr);

    /* Free the Symbol Table */
    symbol_table_free(vc->symbol_table);

    /* Free Hashes */
    infostd_hash_str_free(vc->user_def_hash);
    infostd_hash_str_free(vc->dynl_hash);

#ifdef WITH_THREADS
    /* Free all the threads? */
    while(vc->thread_list) {
      tmp = vc->thread_list;
      vc->thread_list = tmp->next;

      /* Kill the thread if it is running */
      if(tmp->status != VCSI_THREAD_DONE)
	pthread_kill(*tmp->thread,SIGKILL);

      /* Free the thread */
      thread_free(tmp);
    }

    /* Free the Mutexes */
    FREE(vc->gc_critical_mutex);
    FREE(vc->thread_mutex);
    FREE(vc->user_def_mutex);
    FREE(vc->error_mutex);
#endif

    /* Free the heap */
    FREE(vc->heap);

    /* Free the Wind */
    FREE(vc->root_wind);

    /* Free Protected Objects */
    free_protected(vc->elseobj);
    free_protected(vc->unbound);
    free_protected(vc->novalue);
    free_protected(vc->unquote);
    free_protected(vc->unquotespl);
    free_protected(vc->specialobj);
    free_protected(vc->syntax_rules);
    free_protected(vc->elipsis);

  } else if(vc->type == VCSI_CONTEXT_CLONE) {
    /* Free the Wind */
    FREE(vc->root_wind);

    /* Free Buffers */
    infostd_dyn_str_free(vc->input);
    infostd_dyn_str_free(vc->result);
    infostd_dyn_str_free(vc->res_extra);
    infostd_dyn_str_free(vc->tmpstr);

    /* Remove it from the context list */
    vm = vcsi_context_find_master(vc);
#ifdef WITH_THREADS
    pthread_mutex_lock(vc->context_mutex);
#endif
    if(vm && vm->context_list) {
      ltmp = vm->context_list;
      lprev = ltmp;

      if(vc == vm->context_list->context) {
	ltmp = vm->context_list;
	vm->context_list = vm->context_list->next;
	FREE(ltmp);
      } else {
	while(ltmp) {
	  if(ltmp->context == vc) {
	    lprev->next = ltmp->next;
	    FREE(ltmp);
	    break;
	  }
	  lprev = ltmp;
	  ltmp = ltmp->next;
	}
      }
    }
#ifdef WITH_THREADS
    pthread_mutex_unlock(vc->context_mutex);
#endif

  }

  /* Free the Context */
  FREE(vc);
}

VCSI_CONTEXT vcsi_context_find_master(VCSI_CONTEXT vc) {
  VCSI_CONTEXT tmp;

  for(tmp=vc;tmp;tmp=tmp->next){

    if(tmp->type==VCSI_CONTEXT_MASTER)
      return tmp;

  }
  /* If the master can't be located, return the current context */
  return vc;
}

char* rep(VCSI_CONTEXT vc,
	  char* text) {

  char* tmp;
  struct timeval t1, t2;
  struct timezone tz;

  VCSI_OBJECT pos; /* stack pointer */

  tz.tz_minuteswest = 0;
  tz.tz_dsttime = 0;
  vc->got_error = 0;
  gettimeofday(&t1,&tz);
  if(!sigsetjmp(DYNERR(vc->root_wind),1)) {
    /* Set our stack pinter */
    DYNSTCK(vc->root_wind) = &pos;
    pos = parse_text(vc,text);
    pos = eval(vc,pos,DYNROOT(vc->root_wind));
    tmp = print_obj(vc,vc->result, pos,1);
  } else {
    tmp = print_obj(vc,vc->result,vc->errobj,1);
  }

  gettimeofday(&t2,&tz);

  vc->secs = (t2.tv_sec - t1.tv_sec) + (float)(t2.tv_usec - t1.tv_usec) /
    (float)1000000;

  if(vc->verbose > 1) {
    infostd_dyn_str_printf(vc->tmpstr,"%s\n;; Evaluation took %fs -- GC: %ld cells allocated, %ld cells collected, %ld available.",vc->result->buff,vc->secs,vc->gc_alloced,vc->gc_collected,get_free_length(vc));
    infostd_dyn_str_strcpy(vc->result,vc->tmpstr->buff);
  }

  return tmp;
}

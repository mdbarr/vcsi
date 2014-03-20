#include "vcsi.h"

VCSI_OBJECT errorq(VCSI_CONTEXT vc,
		   VCSI_OBJECT obj) {

  return eq(vc,obj,vc->errobj);
}

VCSI_OBJECT pairq(VCSI_CONTEXT vc,
		  VCSI_OBJECT obj) {

  if(TYPEP(obj,CONS))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT nullq(VCSI_CONTEXT vc,
		  VCSI_OBJECT obj) {
  if(obj == NULL)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT booleanq(VCSI_CONTEXT vc,
		     VCSI_OBJECT obj) {
  if(TYPEP(obj,BOOLEAN))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT procq(VCSI_CONTEXT vc,
		  VCSI_OBJECT obj) {

  if(TYPEP(obj,PROC0) || TYPEP(obj,PROC1) || TYPEP(obj,PROC2) ||
     TYPEP(obj,PROC3) || TYPEP(obj,PROC4) || TYPEP(obj,PROC5) || 
     TYPEP(obj,PROCENV) || TYPEP(obj,PROCOPT) || TYPEP(obj,CLOSURE) ||
     TYPEP(obj,CONTINUATION))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT listq(VCSI_CONTEXT vc,
		  VCSI_OBJECT obj) {

  VCSI_OBJECT tmp;
  for(tmp=obj;TYPEP(tmp,CONS);tmp=CDR(tmp));
  if(tmp == NULL)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT eq(VCSI_CONTEXT vc,
	       VCSI_OBJECT x, 
	       VCSI_OBJECT y) {

  if(EQ(x,y))
    return vc->true;
  if(TYPE(x) != TYPE(y))
    return NULL;
  if(TYPEP(x,CONS))
    return NULL;
  if(TYPEP(x,BOOLEAN))
    if(BOOL(x) == BOOL(y))
      return vc->true;
  if(TYPEP(x,STRING))
    if(!strcmp(STR(x),STR(y)))
      return vc->true;
  if(TYPEP(x,LNGNUM))
    if(LNGN(x) == LNGN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM))
    if(FLTN(x) == FLTN(y))
      return vc->true;
  if(TYPEP(x,RATNUM))
    if((double)RNUM(x)/RDENOM(x) == (double)RNUM(y)/RDENOM(y))
      return vc->true;
  if(TYPEP(x,SYMBOL)) {
    if(!strcasecmp(SNAME(x),SNAME(y)))
      return vc->true;
  }
  return NULL;
}

VCSI_OBJECT r_equal(VCSI_CONTEXT vc,
		    VCSI_OBJECT x, 
		    VCSI_OBJECT y) {

  VCSI_OBJECT tmpx, tmpy;
  int i;
      
  if(TYPE(x) != TYPE(y))
    return NULL;
   
  if(TYPEP(x,CONS)) {
    i = get_length(x);
    if(i != get_length(y))
      return NULL;
	
    for(tmpx=x,tmpy=y;TYPEP(tmpx,CONS);tmpx=CDR(tmpx),tmpy=CDR(tmpy)) {
      if(!r_equal(vc,CAR(tmpx),CAR(tmpy)))
	return NULL;
    }
    return vc->true;
  } else if(TYPEP(x,VECTOR)) {
    if(VSIZE(x) != VSIZE(y))
      return NULL;
	
    for(i=0;i<VSIZE(x);i++) {
      if(!r_equal(vc,VELEM(x)[i],VELEM(y)[i]))
	return NULL;
    }
    return vc->true;
  } else   
    return eq(vc,x,y);
}

VCSI_OBJECT equal(VCSI_CONTEXT vc,
		  VCSI_OBJECT x, 
		  VCSI_OBJECT y) {

  if(r_equal(vc,x,y) != NULL)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT memq(VCSI_CONTEXT vc,
		 VCSI_OBJECT x, 
		 VCSI_OBJECT y) {

  VCSI_OBJECT tmp = NULL;
  for(tmp=y;TYPEP(tmp,CONS);tmp=CDR(tmp))
    if(eq(vc,x,CAR(tmp)))
      return tmp;
  return vc->false;
}

VCSI_OBJECT eqv(VCSI_CONTEXT vc,
		VCSI_OBJECT x, 
		VCSI_OBJECT y) {
  if(eq(vc,x,y))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT member(VCSI_CONTEXT vc,
		   VCSI_OBJECT x, 
		   VCSI_OBJECT y) {
  VCSI_OBJECT tmp = NULL;
  for(tmp=y;TYPEP(tmp,CONS);tmp=CDR(tmp))
    if(r_equal(vc,x,CAR(tmp)))
      return tmp;
  return vc->false;
}

VCSI_OBJECT assq(VCSI_CONTEXT vc,
		 VCSI_OBJECT x, 
		 VCSI_OBJECT y) {
  VCSI_OBJECT tmp = NULL;
  for(tmp=y;TYPEP(tmp,CONS);tmp=CDR(tmp))
    {
      if(!TYPEP(CAR(tmp),CONS))
	return error(vc,"object passed to assq is the wrong type",CAR(tmp));
      if(eq(vc,x,CAR(CAR(tmp))))
	return CAR(tmp);
    }    
  return vc->false;
}

VCSI_OBJECT assoc(VCSI_CONTEXT vc,
		  VCSI_OBJECT x, 
		  VCSI_OBJECT y) {

  VCSI_OBJECT tmp = NULL;
  for(tmp=y;TYPEP(tmp,CONS);tmp=CDR(tmp))
    {
      if(!TYPEP(CAR(tmp),CONS))
	return error(vc,"object passed to assoc is the wrong type",CAR(tmp));
      if(r_equal(vc,x,CAR(CAR(tmp))))
	return CAR(tmp);
    }
  return vc->false;
}

int obj_compare_i(VCSI_OBJECT x, 
		  VCSI_OBJECT y) {
  
  if(TYPE(x) == TYPE(y)) {

    if(TYPEP(x,STRING))
      return strcmp(STR(x),STR(y));
    else if(TYPEP(x,SYMBOL))
      return strcmp(SNAME(x),SNAME(y));
    else if(TYPEP(x,CHAR)) {
      if(CHAR(x) == CHAR(y))
	return 0;
      else if(CHAR(x) > CHAR(y))
	return 1;
      else
	return -1;
    } else if(TYPEP(x,LNGNUM)) {
      if(LNGN(x) == LNGN(y))
	return 0;
      else if(LNGN(x) > LNGN(y))
	return 1;
      else
	return -1;
    } else if(TYPEP(x,FLTNUM)) {
      if(FLTN(x) == FLTN(y))
	return 0;
      else if(FLTN(x) > FLTN(y))
	return 1;
      else
	return -1;
    } else if(TYPEP(x,RATNUM)) {
      if((double)RNUM(x)/RDENOM(x) == (double)RNUM(y)/RDENOM(y))
	return 0;
      else if((double)RNUM(x)/RDENOM(x) > (double)RNUM(y)/RDENOM(y))
	return 1;
      else
	return -1;
    }

    return 0;
  } else if(x->type > y->type)
    return 1;

  return -1;
}

VCSI_OBJECT obj_compare(VCSI_CONTEXT vc,
			VCSI_OBJECT x,
			VCSI_OBJECT y) {

  return make_long(vc,obj_compare_i(x,y));
}

/* Functions to allocate and create new objects of a specific type */
VCSI_OBJECT alloc_obj(VCSI_CONTEXT vc,
		      VCSI_OBJECT_TYPE type) {
  VCSI_CONTEXT vm;
  VCSI_OBJECT new_obj;    

  /* stack pointers */
  VCSI_OBJECT stck_end, *stck_old;

  /* save new stack pointers */
  stck_old = DYNSTCKE(vc->root_wind);
  DYNSTCKE(vc->root_wind) = &stck_end;

  /* Use the master context */
  vm = vcsi_context_find_master(vc);
  
  if(vm->freeobj == NULL) {
#ifdef DEBUG
    fprintf(stderr,"Garbaging Collecting: %d/%d",vm->gc_alloced,vm->gc_collected);
#endif

    gc_collect(vm);

#ifdef DEBUG
    fprintf(stderr,"/%d\n",get_free_length(vc));
#endif
    if(vm->freeobj == NULL) {
      fprintf(stderr,"Unable to Garbage Collect. Exiting...\n");
      exit(1);
    }
  }
#ifdef WITH_THREADS
  /* Lock the heap */
  pthread_mutex_lock(vm->gc_critical_mutex);
#endif

  vm->gc_alloced++;
  new_obj = vm->freeobj;
  vm->freeobj = CDR(vm->freeobj);

  /* Clear the object */
  memset(new_obj,0,SIZE_VCSI_OBJECT);

  /* Set the type */
  new_obj->type = type;

#ifdef WITH_THREADS
  /* Unlock the heap */
  pthread_mutex_unlock(vm->gc_critical_mutex);
#endif

  /* restore the old stack pointers */
  DYNSTCKE(vc->root_wind) = stck_old;

  return new_obj;
}

void* dmalloc(int size) {

  return malloc(size);
}

void dfree(void* ptr) {

  free(ptr);
}

VCSI_OBJECT protect_var(VCSI_CONTEXT vc,
			char* var) {
			
  VCSI_OBJECT tmp;

  tmp = make_symbol(vc,var);
  tmp = envlookup(vc,tmp,vc->the_global_env);

  if(TYPEP(tmp,SYMBOL))
    SPROTECT(tmp)=1;
  return tmp;
}

VCSI_OBJECT make_protected(char* str) {

  VCSI_OBJECT tmp;

  tmp = MALLOC(SIZE_VCSI_OBJECT);
  memset(tmp,0,SIZE_VCSI_OBJECT);

  tmp->type = SYMBOL;
  SPROTECT(tmp) = 1;

  SNAME(tmp) = strdup(str);
  VCELL(tmp) = tmp;

  return tmp;
}

void free_protected(VCSI_OBJECT x) {
  
  if(TYPEP(x,SYMBOL))
    FREE(SNAME(x));

  FREE(x);
}

VCSI_OBJECT make_boolean(VCSI_CONTEXT vc,
			 unsigned short val) {

 VCSI_OBJECT tmp = alloc_obj(vc,BOOLEAN);
 BOOL(tmp) = val;

 return tmp;
}

VCSI_OBJECT make_proc(VCSI_CONTEXT vc,
		      char* name, 
		      VCSI_OBJECT_TYPE type, 
		      VCSI_OBJECT (*proc)()) {

  VCSI_OBJECT tmp = alloc_obj(vc,type);
  tmp->data.proc.name = name;
  tmp->data.proc.func = proc;
  return tmp;
}


VCSI_OBJECT make_internal(VCSI_CONTEXT vc,
			  char *name) {
  VCSI_OBJECT tmp;
  tmp = symbol_cons(vc,name,NULL);
  CAR(vc->the_global_env) = cons(vc,tmp, CAR(vc->the_global_env));
  return tmp;
}

VCSI_OBJECT make_special(VCSI_CONTEXT vc,
			 char *name) {
  VCSI_OBJECT tmp;
  tmp = symbol_cons(vc,name,NULL);
  CAR(vc->the_global_env) = cons(vc,tmp, CAR(vc->the_global_env));

  setvar(vc,tmp,vc->specialobj,NULL);

  return tmp;
}

VCSI_OBJECT make_closure(VCSI_CONTEXT vc,
			 VCSI_OBJECT code, 
			 VCSI_OBJECT env) {

  VCSI_OBJECT tmp = alloc_obj(vc,CLOSURE);
  CLSCODE(tmp) = code;
  CLSENV(tmp) = env;
  return tmp;
}

VCSI_OBJECT make_lambda(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT code) {
  VCSI_OBJECT tmp;
  for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp))
    if(!TYPEP(CAR(tmp),SYMBOL))
      return error(vc,"improperly formed lambda arguments",args);
  return cons(vc,make_symbol(vc,"lambda"),
	      vcsi_list_append(vc,cons(vc,args,NULL),code));
}

VCSI_OBJECT make_thunk(VCSI_CONTEXT vc,
		       VCSI_OBJECT exp, 
		       VCSI_OBJECT env) {
  VCSI_OBJECT tmp = alloc_obj(vc,THUNK);
  THNKEXP(tmp) = exp;
  THNKENV(tmp) = env;
  return tmp;
}

VCSI_OBJECT make_quoted(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {
  return cons(vc,make_symbol(vc,"quote"),cons(vc,x,NULL));
}

VCSI_OBJECT make_list(VCSI_CONTEXT vc, 
		      int count, ...) {
  
  VCSI_OBJECT tmp = NULL;
  int i;
  va_list ap;

  va_start(ap,count);
  for(i=0;i<count;i++)
    tmp=cons(vc,va_arg(ap,VCSI_OBJECT),tmp);
  va_end(ap);
  
  return vcsi_list_reverse(vc,tmp);
}

/* Syntax functions */
VCSI_OBJECT make_syntax(VCSI_CONTEXT vc,
			VCSI_OBJECT pattern,
			VCSI_OBJECT env) {

  VCSI_OBJECT tmp;
  
  check_arg_type(vc,pattern,CONS,"syntax-rules");
  if(!eq(vc,car(vc,pattern),vc->syntax_rules))
    error(vc,"object passed to make-syntax is of wrong type",pattern);

  pattern = cdr(vc,pattern);

  tmp=alloc_obj(vc,SYNTAX);
  LITERALS(tmp) = car(vc,pattern);
  if(LITERALS(tmp) && !TYPEP(LITERALS(tmp),CONS))
    return error(vc,"invalid literals to syntax-rules",LITERALS(tmp));

  /* sanity check ... */
  PATTERNS(tmp) = cdr(vc,pattern);
  check_pattern_validity(vc,PATTERNS(tmp));

  SYNENV(tmp) = env;

  return tmp;
}

VCSI_OBJECT check_pattern_validity(VCSI_CONTEXT vc,
				   VCSI_OBJECT pattern) {

  VCSI_OBJECT tmp, ptmp, stmp, ttmp;
  check_arg_type(vc,pattern,CONS,"syntax-rules");
  for(tmp=pattern;TYPEP(tmp,CONS);tmp=cdr(vc,tmp)) {
    check_arg_type(vc,tmp,CONS,"syntax-rules");

    ptmp = car(vc,tmp);
    
    if(get_length(ptmp) != 2)
      return error(vc,"invalid pattern to syntax-rules",ptmp);

    /* check the pattern */
    stmp = car(vc,tmp);
    if(TYPEP(stmp,VECTOR))
      stmp = vector_list(vc,stmp);

    if(TYPEP(stmp,CONS)) {
      for(ttmp=stmp;TYPEP(ttmp,CONS);ttmp=cdr(vc,ttmp)){
	if(eq(vc,car(vc,ttmp),vc->elipsis) && cdr(vc,ttmp))
	  return error(vc,"bad elipsis usage in syntax-rules",stmp);
      }
    }
    
    /* check the template */
    stmp = cadar(vc,tmp);
    if(TYPEP(stmp,VECTOR))
      stmp = vector_list(vc,stmp);

    if(TYPEP(stmp,CONS)) {
      for(ttmp=stmp;TYPEP(ttmp,CONS);ttmp=cdr(vc,ttmp)){
	if(eq(vc,car(vc,ttmp),vc->elipsis) && cdr(vc,ttmp))
	  return error(vc,"bad elipsis usage in syntax-rules",stmp);
      }
    }

  }
  return vc->true;
}


VCSI_OBJECT find_pattern_match(VCSI_CONTEXT vc,
			       VCSI_OBJECT syntax,
			       VCSI_OBJECT pattern,
			       VCSI_OBJECT newenv) {

  VCSI_OBJECT tmp, pat, template, bindings;

  /* syntax: (sass) ((_ ?a sass ?b) (+ ?a ?b)) */
  /* pattern: (foo 1 sass 2) */

  check_arg_type(vc,syntax,SYNTAX,"syntax-rules");

  tmp = pat = template = bindings = NULL;

  for(tmp=PATTERNS(syntax),bindings=NULL;
      TYPEP(tmp,CONS) && !bindings;
      tmp=cdr(vc,tmp)) {

    pat = caar(vc,tmp);
    template = cdar(vc,tmp);
    bindings = list_pattern_match(vc,pat,pattern,LITERALS(syntax));
  }
  
  if(bindings) {
    for(tmp=bindings;TYPEP(tmp,CONS);tmp=cdr(vc,tmp))
      syntax_set_scope(vc,cdar(vc,tmp),newenv);
  
    return transform_template(vc,syntax,template,bindings,newenv);
  }
  
  return NULL;
}

VCSI_OBJECT list_pattern_match(VCSI_CONTEXT vc,
			       VCSI_OBJECT syntax,
			       VCSI_OBJECT pattern,
			       VCSI_OBJECT literals) {

  VCSI_OBJECT tmp, stmp, ptmp, scar, pcar, bindings;
  int vector=0;

  tmp = stmp = ptmp = scar = pcar = bindings = NULL;

  if(TYPEP(syntax,VECTOR)) {
    if(TYPEP(pattern,VECTOR)) {
      syntax = vector_list(vc,syntax);
      pattern = vector_list(vc,pattern);
      vector=1;
    } else
      return NULL;
  }

  for(stmp=syntax,ptmp=pattern;TYPEP(stmp,CONS)&&TYPEP(ptmp,CONS);
      stmp=cdr(vc,stmp),ptmp=cdr(vc,ptmp)) {

    scar = car(vc,stmp);
    pcar = car(vc,ptmp);

    if(TYPEP(scar,CONS)) {      
      tmp = list_pattern_match(vc,scar,pcar,literals);
      if(tmp)
	bindings = vcsi_list_append(vc,tmp,bindings);
      else
	return NULL;
    } else if(TYPEP(scar,VECTOR)) {
      if(!TYPEP(pcar,VECTOR))
	return NULL;

      tmp = list_pattern_match(vc,vector_list(vc,scar),
			       vector_list(vc,pcar),literals);

      if(tmp)
	bindings = vcsi_list_append(vc,tmp,bindings);
      else
	return NULL;
    } else if(!eq(vc,member(vc,scar,literals),vc->false)) {
      if(!eq(vc,scar,pcar))
	return NULL;
      tmp = cons(vc,scar,pcar);
      bindings = cons(vc,tmp,bindings);
    } else if(eq(vc,scar,vc->elipsis)) {
      tmp = cons(vc,scar,ptmp);
      ptmp=stmp;
      bindings = cons(vc,tmp,bindings);
    } else if(TYPEP(scar,SYMBOL)) {
      tmp = cons(vc,scar,pcar);
      bindings = cons(vc,tmp,bindings);
    } else {
      if(!eq(vc,scar,pcar))
	return NULL;
    }
  }
  
  if(TYPEP(stmp,CONS)) {
    if(!eq(vc,member(vc,vc->elipsis,stmp),vc->false)) {
      for(;TYPEP(stmp,CONS);stmp=cdr(vc,stmp)) {
	scar = car(vc,stmp);
	tmp = cons(vc,scar,vc->unbound);
	bindings = cons(vc,tmp,bindings);
      }
    }
  }

  if(!stmp && !ptmp) {
    return bindings;
  }
  return NULL;
}

VCSI_OBJECT transform_template(VCSI_CONTEXT vc,
			       VCSI_OBJECT syntax,
			       VCSI_OBJECT template,			      
			       VCSI_OBJECT bindings,
			       VCSI_OBJECT newenv) {

  VCSI_OBJECT exp, tmp, ttmp, btmp, etmp;
  int skip, found, vector;

  exp = tmp = ttmp = btmp = etmp = NULL;  
  vector=0;

  if(get_length(template)==1 && TYPEP(car(vc,template),VECTOR)) {
    template = vector_list(vc,car(vc,template));
    vector=1;
  }

  if(!TYPEP(template,CONS))
    return template;

  for(tmp=template;TYPEP(tmp,CONS);tmp=cdr(vc,tmp)) {
    skip = found = 0;

    ttmp = car(vc,tmp);
    
    if(TYPEP(ttmp,CONS)) {
      ttmp = transform_template(vc,syntax,ttmp,bindings,newenv);
      found=1;
    } else {
      for(btmp=bindings;TYPEP(btmp,CONS);btmp=cdr(vc,btmp)) {
	if(eq(vc,caar(vc,btmp),ttmp)) {
	  if(!eq(vc,ttmp,vc->elipsis)) {
	    ttmp = cdar(vc,btmp);
	    found=1;
	  } else {
	    for(etmp=cdar(vc,btmp);TYPEP(etmp,CONS);etmp=cdr(vc,etmp)) {
	      exp = cons(vc,car(vc,etmp),exp);
	    }
	    skip=1;
	  }
	  break;
	}
      }
    }
    if(eq(vc,ttmp,vc->unbound))
      skip=1;

    if(!skip) {
	
      exp = cons(vc,ttmp,exp);  
    }
  }
  if(vector)
    return list_vector(vc,vcsi_list_reverse(vc,exp));

  return vcsi_list_reverse(vc,exp);
}

VCSI_OBJECT syntax_set_scope(VCSI_CONTEXT vc,
			     VCSI_OBJECT x,
			     VCSI_OBJECT env) {
  
  VCSI_OBJECT tmp;
  if(TYPEP(x,SYMBOL) && !SCOPE(x))
    SCOPE(x) = env;
  else if(TYPEP(x,CONS)) {
    for(tmp=x;TYPEP(tmp,CONS);tmp=cdr(vc,tmp))
      syntax_set_scope(vc,car(vc,tmp),env);
    if(tmp)
      syntax_set_scope(vc,tmp,env);
  } 

  return x;
}

/* Hashes */
VCSI_OBJECT make_hash(VCSI_CONTEXT vc) {

  VCSI_OBJECT tmp;
  
  tmp = alloc_obj(vc,HASH);

  HASHT(tmp) = (VCSI_HASH_TAB*)malloc(VCSI_HASH_SIZE * SIZE_VCSI_HASH_TAB);
  memset(HASHT(tmp),0,VCSI_HASH_SIZE * SIZE_VCSI_HASH_TAB);

  HASHN(tmp) = 0;

  return tmp;
}

void free_hash(VCSI_CONTEXT vc,
	       VCSI_OBJECT h) {

  VCSI_HASH_TAB tmp,next;
  int i;

  if(HASHT(h)) {
    for(i=0;i<VCSI_HASH_SIZE;i++) {
      for(tmp=HASHT(h)[i];tmp;) {
	next = tmp->next;
	free_tab_hash(tmp);
	tmp = next;
      }
    }
    free(HASHT(h));
  }
}

VCSI_HASH_TAB init_tab_hash(VCSI_OBJECT key,
			    VCSI_OBJECT info) {

  VCSI_HASH_TAB vht = (VCSI_HASH_TAB)malloc(SIZE_VCSI_HASH_TAB);
  memset(vht,0,SIZE_VCSI_HASH_TAB);

  vht->key = key;
  vht->info = info;

  return vht;
}

void free_tab_hash(VCSI_HASH_TAB vht) {

  free(vht);
}

unsigned int hash_hash(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  unsigned int i = 0;
  char* ln;
  
  switch (x->type) {
  case STRING:
    for(ln=STR(x),i=0;*ln!='\0';ln++)
      i = ((i * 128) + (unsigned long)(*ln)) % VCSI_HASH_SIZE;
    break;
  case SYMBOL:
    for(ln=SNAME(x),i=0;*ln!='\0';ln++)
      i = ((i * 128) + (unsigned long)(*ln)) % VCSI_HASH_SIZE;
    break;
  case CHAR:
    i = (unsigned int)CHAR(x);
    break;
  case LNGNUM:
    i = (LNGN(x)*62342347) % VCSI_HASH_SIZE;
    break;
  default:
    i = (((int)x->type)*5)%VCSI_HASH_SIZE;
    break;
  }
  return i;
}

VCSI_OBJECT hash_lookup_i(VCSI_CONTEXT vc,
			  VCSI_OBJECT h,
			  VCSI_OBJECT key) {

  unsigned int pos;
  VCSI_HASH_TAB l;

  if(!HASHN(h))
    return NULL;
  
  pos = hash_hash(vc,key);

  for(l=HASHT(h)[pos];l;){
    if(eq(vc,l->key,key))
      break;
    else
      l=l->next;
  }

  if(l) {
    return l->info; 
  }
  return NULL;
}

VCSI_OBJECT hash_lookup(VCSI_CONTEXT vc,
			VCSI_OBJECT h,
			VCSI_OBJECT key) {
 
  VCSI_OBJECT ret;
  
  check_arg_type(vc,h,HASH,"hash-lookup");

  if((ret = hash_lookup_i(vc,h,key)))
    return ret;
  return vc->false;
}

VCSI_OBJECT hash_set(VCSI_CONTEXT vc,
		     VCSI_OBJECT h,
		     VCSI_OBJECT key,
		     VCSI_OBJECT info) {

  VCSI_HASH_TAB l;
  int pos;

  check_arg_type(vc,h,HASH,"hash-set!");

  if(!HASHN(h))
    return NULL;
  
  pos = hash_hash(vc,key);

  for(l=HASHT(h)[pos];l;){
    if(eq(vc,l->key,key))
      break;
    else
      l=l->next;
  }

  if(l) {
    l->info = info;    
    return l->info; 
  }

  return NULL;
}

VCSI_OBJECT hash_add(VCSI_CONTEXT vc,
		     VCSI_OBJECT h,
		     VCSI_OBJECT key,
		     VCSI_OBJECT info) {

  VCSI_HASH_TAB tmp, l;
  VCSI_OBJECT id;
  unsigned int pos;

  check_arg_type(vc,h,HASH,"hash-add");
  
  pos = hash_hash(vc,key);

  /* Implicit set! on adds */
  id = hash_set(vc,h,key,info);
  if(id)
    return vc->true;
  
  l = init_tab_hash(key,info);
  if(HASHT(h)[pos]) {
    for(tmp=HASHT(h)[pos];tmp&&tmp->next;tmp=tmp->next);
    tmp->next = l;
  } else {
    HASHT(h)[pos] = l;
  }
  HASHN(h)++;

  return vc->true;
}

VCSI_OBJECT hash_delete(VCSI_CONTEXT vc,
			VCSI_OBJECT h,
			VCSI_OBJECT key) {

  VCSI_HASH_TAB l,p;
  unsigned int pos;
  
  check_arg_type(vc,h,HASH,"hash-delete");

  if(!HASHN(h))
    return vc->false;

  pos = hash_hash(vc,key);
  
  for(l=HASHT(h)[pos],p=NULL;l;p=l,l=l->next) {
    if(eq(vc,l->key,key))
      break;
  }

  if(!l)
    return vc->false;
  
  if(p)
    p->next=l->next;

  if(l == HASHT(h)[pos])
    HASHT(h)[pos] = l->next;

  free_tab_hash(l);

  HASHN(h)--;
  
  return vc->true;
}

VCSI_OBJECT hash_keys(VCSI_CONTEXT vc,
		      VCSI_OBJECT h) {

  VCSI_HASH_TAB tmp,next;
  VCSI_OBJECT ret = NULL;
  int i;

  check_arg_type(vc,h,HASH,"hash-keys");

  if(HASHT(h)) {
    for(i=0;i<VCSI_HASH_SIZE;i++) {
      for(tmp=HASHT(h)[i];tmp;) {
	next = tmp->next;
	ret = cons(vc,tmp->key,ret);
	tmp = next;
      }
    }
  }
  return vcsi_list_reverse(vc,ret);
}

void hash_mark(VCSI_CONTEXT vc,
	       VCSI_OBJECT h,
	       int flag) {

  VCSI_HASH_TAB tmp,next;
  int i;

  if(HASHT(h)) {
    for(i=0;i<VCSI_HASH_SIZE;i++) {
      for(tmp=HASHT(h)[i];tmp;) {
	next = tmp->next;
	gc_mark(vc,tmp->key,flag);
	gc_mark(vc,tmp->info,flag);
	tmp = next;
      }
    }
  }
}

/* User Defined Object Functions */
int same_type_user_def(VCSI_CONTEXT vc,
		       VCSI_OBJECT obj, 
		       VCSI_USER_DEF type) {

  if(TYPE(obj) == USERDEF && UDTYPE(obj) != NULL) {
    if(UDTYPE(obj)->sub_type == type->sub_type)
      return 1;
  }

  return 0;
}

VCSI_USER_DEF lookup_user_def(VCSI_CONTEXT vc,
			      char* name) {

  VCSI_USER_DEF tmp;

  if((unsigned long)(tmp=(VCSI_USER_DEF)infostd_hash_str_info(vc->user_def_hash
							      ,name))
     !=ULONG_MAX)
    return tmp;
  return NULL;
}
  

VCSI_USER_DEF make_user_def_type(VCSI_CONTEXT vc,
				 char* name,
				 void (*udp)(),
				 void (*udm)(),
				 void (*udf)()) {
  VCSI_USER_DEF tmp = NULL;
  
  tmp = lookup_user_def(vc,name);
  if(tmp)
    return tmp;

  tmp = (VCSI_USER_DEF)MALLOC(SIZE_VCSI_USER_DEF);
  memset(tmp,0,SIZE_VCSI_USER_DEF);

#ifdef WITH_THREADS
  pthread_mutex_lock(vc->user_def_mutex);
#endif

  tmp->sub_type = vc->user_def_num++;

  tmp->name=name;

  tmp->ud_print = udp;  
  tmp->ud_mark = udm;
  tmp->ud_free = udf;

  tmp->ud_eval = NULL;

  infostd_hash_str_add(vc->user_def_hash,name,(unsigned long)tmp);

#ifdef WITH_THREADS
  pthread_mutex_unlock(vc->user_def_mutex);
#endif

  return tmp;
}

VCSI_OBJECT make_user_def(VCSI_CONTEXT vc,
			  void* val, 
			  VCSI_USER_DEF ut) {

  VCSI_OBJECT tmp = alloc_obj(vc,USERDEF);
  
  UDTYPE(tmp) = ut;
  UDVAL(tmp) = val;

  return tmp;
}

int user_def_set_eval(VCSI_CONTEXT vc,
		      VCSI_OBJECT obj,
		      VCSI_OBJECT_TYPE type, 
		      VCSI_OBJECT (*proc)()) {
  
  if(!(TYPEP(obj,USERDEF) && UDTYPE(obj)))
    return 0;
  
  UDTYPE(obj)->ud_eval = make_proc(vc,UDTYPE(obj)->name,type,proc);

  return 1;
}

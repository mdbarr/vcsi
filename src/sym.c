#include "vcsi.h"

VCSI_SYMBOL_TABLE symbol_table_init(void) {
  VCSI_SYMBOL_TABLE tmp;

  tmp = (VCSI_SYMBOL_TABLE)MALLOC(SIZE_VCSI_SYMBOL_TABLE);
  memset(tmp,0,SIZE_VCSI_SYMBOL_TABLE);

  return tmp;
}

void symbol_table_free(VCSI_SYMBOL_TABLE st) {

  VCSI_SYMBOL tmp, curr;

  tmp = st->table;

  while(tmp) {
    curr = tmp;
    tmp=curr->next;

    FREE(curr->name);
    FREE(curr);
  }

#ifdef WITH_THREADS
  if(st->table_mutex)
    FREE(st->table_mutex);
#endif

  FREE(st);
}

VCSI_SYMBOL sym_create(char* name) {
  VCSI_SYMBOL tmp;

  tmp = (VCSI_SYMBOL)MALLOC(SIZE_VCSI_SYMBOL);
   
  tmp->name = strdup(name);
  tmp->ref = 0;
   
  tmp->next = NULL;
   
  return tmp;
}

void sym_insert(VCSI_SYMBOL_TABLE st, 
		VCSI_SYMBOL item) {

#ifdef WITH_THREADS
  pthread_mutex_lock(st->table_mutex);
#endif

  if(st->table == NULL) {
    st->table = item;
  } else {
    item->next = st->table;
    st->table = item;
  }
  st->count++;

#ifdef WITH_THREADS
  pthread_mutex_unlock(st->table_mutex);
#endif

}

void sym_remove(VCSI_SYMBOL_TABLE st, 
		VCSI_SYMBOL item) {

  VCSI_SYMBOL node, pnode;
   
#ifdef WITH_THREADS
  pthread_mutex_lock(st->table_mutex);
#endif

  node = st->table;
  pnode = node;
  if(item == st->table) {
    st->table = item->next;
  } else {
    while(node) {
      if(node == item) {
	pnode->next = node->next;
      }
      pnode = node;
      node = node->next;
    }
  }
  st->count--;

#ifdef WITH_THREADS
  pthread_mutex_unlock(st->table_mutex);
#endif
}

void sym_print(VCSI_SYMBOL_TABLE st) {

  VCSI_SYMBOL tmp;
  tmp = st->table;

  while(tmp) {
    printf("Sym: %s\n",tmp->name);
    fflush(stdout);
    tmp=tmp->next;
  }
}
   
VCSI_SYMBOL sym_lookup(VCSI_SYMBOL_TABLE st,
		       char* name) {

  int i;
  VCSI_SYMBOL tmp;

  for(tmp=st->table,i=0;tmp!=NULL && i<st->count;tmp=tmp->next,i++) {
    if(!strcasecmp(name,tmp->name))
      return tmp;
  }
  return NULL;
}

char* sym_add(VCSI_SYMBOL_TABLE st,
	      char* name) {

  VCSI_SYMBOL tmp;
  if((tmp = sym_lookup(st,name)) != NULL) {
    tmp->ref++;
    return tmp->name;
  } else {
    tmp = sym_create(name);
    sym_insert(st,tmp);
	
    tmp->ref++;
    return tmp->name;
  }
}

void sym_free(VCSI_SYMBOL_TABLE st,
	      char* name) {

  VCSI_SYMBOL tmp;

  if((tmp = sym_lookup(st,name)) != NULL) {
    tmp->ref--;
    if(tmp->ref == 0) {
      sym_remove(st,tmp);
      FREE(tmp->name);
      FREE(tmp);
    }
  }
  else
    FREE(name);
}

int sym_r5rs_valid(char* name) {
  char* ident_init = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?^_~";
  char* ident_subs = "0123456789+-.@";
  unsigned long i, len;
  unsigned char c;

  len = strlen(name);

  if(!len)
    return 0;

  for(i=0;i<len;i++) {
    c = name[i];
    if(c < 32 || c > 127)
      return 0;
  }

  /* Peculiar Identifier */
  if(!strcmp(name,"+") || !strcmp(name,"-") || !strcmp(name,"..."))
    return 1;
  
  /* Identifier */
  if(!strchr(ident_init,name[0]))
    return 0;
  
  for(i=1;i<len;i++) {
    if(!strchr(ident_init,name[i]) && !strchr(ident_subs,name[i]))
      return 0;
  }

  return 1;
}

void sym_init(VCSI_CONTEXT vc) {
  set_int_proc(vc,"symbol?",PROC1,symbolq);
  set_int_proc(vc,"symbol->string",PROC1,symbol_to_string);
  set_int_proc(vc,"gensym",PROCX,make_gensym);
}

VCSI_OBJECT symbolq(VCSI_CONTEXT vc, 
		    VCSI_OBJECT obj) {
  if(TYPEP(obj,SYMBOL))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT make_symbol(VCSI_CONTEXT vc, 
			char* str) {
  VCSI_OBJECT tmp;

  if(!sym_r5rs_valid(str)) {
    error(vc,"invalid symbol",make_string(vc,str));
  }

  tmp = alloc_obj(vc,SYMBOL);
  SNAME(tmp) = sym_add(vc->symbol_table,str);
  VCELL(tmp) = vc->unbound;
  SCOPE(tmp) = NULL;

  return tmp;
}

VCSI_OBJECT symbol_cons(VCSI_CONTEXT vc, 
			char *name, 
			VCSI_OBJECT vcell) {

  VCSI_OBJECT tmp;

  if(!sym_r5rs_valid(name)) {
    error(vc,"invalid symbol",make_string(vc,name));
  }

  tmp = alloc_obj(vc,SYMBOL);
  SNAME(tmp) = sym_add(vc->symbol_table,name);
  VCELL(tmp) = vcell;
  return tmp;
}

VCSI_OBJECT copy_symbol(VCSI_CONTEXT vc, 
			VCSI_OBJECT obj) {

  VCSI_OBJECT tmp = alloc_obj(vc,SYMBOL);
  SNAME(tmp) = sym_add(vc->symbol_table,SNAME(obj));
  VCELL(tmp) = NULL;
  SPROTECT(tmp)=0;
  return tmp;
   
}

VCSI_OBJECT symbol_to_string(VCSI_CONTEXT vc, 
			     VCSI_OBJECT obj) {

  check_arg_type(vc,obj,SYMBOL,"symbol->string");

  return make_string(vc,SNAME(obj));
}

VCSI_OBJECT make_gensym(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {

  VCSI_SYMBOL_TABLE st;
  char gensym[256];
  int num;

  if(x != vc->novalue)
    check_arg_type(vc,x,STRING,"gensym");
  
  st = vc->symbol_table;

#ifdef WITH_THREADS
  pthread_mutex_lock(st->table_mutex);
#endif
  num = st->gensym_num;
  st->gensym_num++;

#ifdef WITH_THREADS
  pthread_mutex_unlock(st->table_mutex);
#endif
  
  if(x != vc->novalue)
    snprintf(gensym,256,"%s%d",STR(x),num);
  else
    snprintf(gensym,256,"%%%%gensym%d",num);
    
  return make_symbol(vc,gensym);   
}

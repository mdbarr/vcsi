#include "vcsi.h"
/* Essential functions likes cons, car cdr etc */

VCSI_OBJECT cons(VCSI_CONTEXT vc,
		 VCSI_OBJECT x, 
		 VCSI_OBJECT y) {

  VCSI_OBJECT new_obj = alloc_obj(vc,CONS);
  CAR(new_obj) = x;
  CDR(new_obj) = y;
  return new_obj;
}

VCSI_OBJECT car(VCSI_CONTEXT vc,
		VCSI_OBJECT x) {

  if(TYPEP(x,CONS))
    return CAR(x);
  return error(vc,"non-pair to car", x);
}

VCSI_OBJECT cdr(VCSI_CONTEXT vc,
		VCSI_OBJECT x) {
  if(TYPEP(x,CONS))
    return CDR(x);
  return error(vc,"non-pair to cdr", x);
}

VCSI_OBJECT cdar(VCSI_CONTEXT vc,
		 VCSI_OBJECT x) {
  return cdr(vc,car(vc,x));
}

VCSI_OBJECT cadr(VCSI_CONTEXT vc,
		 VCSI_OBJECT x) {
  return car(vc,cdr(vc,x));
}

VCSI_OBJECT caar(VCSI_CONTEXT vc,
		 VCSI_OBJECT x) {
  return car(vc,car(vc,x));
}

VCSI_OBJECT cddr(VCSI_CONTEXT vc,
		 VCSI_OBJECT x) {
  return cdr(vc,cdr(vc,x));
}

VCSI_OBJECT caaar(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return car(vc,caar(vc,x));
}

VCSI_OBJECT caadr(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return car(vc,cadr(vc,x));
}

VCSI_OBJECT cadar(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return car(vc,cdar(vc,x));
}

VCSI_OBJECT cdaar(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return cdr(vc,caar(vc,x));
}

VCSI_OBJECT cddar(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return cdr(vc,cdar(vc,x));
}

VCSI_OBJECT caddr(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return car(vc,cddr(vc,x));
}

VCSI_OBJECT cdadr(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return cdr(vc,cadr(vc,x));
}

VCSI_OBJECT cdddr(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) {
  return cdr(vc,cddr(vc,x));
}

VCSI_OBJECT cddddr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,cdddr(vc,x));
}

VCSI_OBJECT cadddr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,cdddr(vc,x));
}

VCSI_OBJECT cddadr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,cdadr(vc,x));
}

VCSI_OBJECT cadadr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,cdadr(vc,x));
}

VCSI_OBJECT cdaddr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,caddr(vc,x));
}

VCSI_OBJECT caaddr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,caddr(vc,x));
}

VCSI_OBJECT cdddar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,cddar(vc,x));
}

VCSI_OBJECT caddar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,cddar(vc,x));
}

VCSI_OBJECT cddaar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,cdaar(vc,x));
}

VCSI_OBJECT cadaar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,cdaar(vc,x));
}

VCSI_OBJECT cdadar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,cadar(vc,x));
}

VCSI_OBJECT caadar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,cadar(vc,x));
}

VCSI_OBJECT cdaadr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,caadr(vc,x));
}

VCSI_OBJECT caaadr(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,caadr(vc,x));
}

VCSI_OBJECT cdaaar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return cdr(vc,caaar(vc,x));
}

VCSI_OBJECT caaaar(VCSI_CONTEXT vc,
		   VCSI_OBJECT x) {
  return car(vc,caaar(vc,x));
}

VCSI_OBJECT error(VCSI_CONTEXT vc,
		  char* text, 
		  VCSI_OBJECT obj) {
#ifdef DEBUG
  printf("Error: %s - %s\n",text,print_obj(vc,vc->tmpstr,obj,1));
#endif

#ifdef WITH_THREADS
  pthread_mutex_lock(vc->error_mutex);
#endif

  vc->errobj->type = SYMBOL;
  VCELL(vc->errobj) = cons(vc,make_string(vc,text),cons(vc,obj,NULL));
  vc->got_error = 1;

#ifdef WITH_THREADS
  pthread_mutex_unlock(vc->error_mutex);
#endif
   
  siglongjmp(DYNERR(vc->root_wind),1);
  return vc->errobj;
}

VCSI_OBJECT get_int_env(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT env) {
  if(get_length(args) != 0)
    return error(vc,"invalid number or arguments to interaction-environment",args);
  return env;
}

VCSI_OBJECT get_rep_env(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {
  if(!eq(vc,x,make_long(vc,5)))
    return error(vc,"invalid report environment version",x);
   
  return vc->report_env;
}

VCSI_OBJECT get_time(VCSI_CONTEXT vc) {
  time_t clock;

  time(&clock);
   
  return make_long(vc,clock);
}

VCSI_OBJECT get_localtime(VCSI_CONTEXT vc,
			  VCSI_OBJECT x) {

  char buf[100];
  struct tm* lt;
  time_t tt;

  check_arg_type(vc,x,LNGNUM,"localtime");
  
  tt = LNGN(x);
  lt = localtime(&tt);
  
  sprintf(buf,"%d/%d/%d %d:%2.2d:%2.2d",lt->tm_mon+1,lt->tm_mday,
	  lt->tm_year+1900,lt->tm_hour,lt->tm_min,lt->tm_sec);

  return make_string(vc,buf);
}

VCSI_OBJECT get_localtime_list(VCSI_CONTEXT vc,
			       VCSI_OBJECT x) {

  struct tm* lt;
  time_t tt;

  check_arg_type(vc,x,LNGNUM,"localtime->list");
  
  tt = LNGN(x);
  lt = localtime(&tt);
  
  return make_list(vc,9,make_long(vc,lt->tm_sec),
		   make_long(vc,lt->tm_min),
		   make_long(vc,lt->tm_hour),
		   make_long(vc,lt->tm_mday),
		   make_long(vc,lt->tm_mon),
		   make_long(vc,lt->tm_year+1900),
		   make_long(vc,lt->tm_wday),
		   make_long(vc,lt->tm_yday),
		   make_long(vc,lt->tm_isdst));
}

int get_length(VCSI_OBJECT x) {
  int count = 0;

  VCSI_OBJECT tmp;
  if(x == NULL)
    return count;
  if(!TYPEP(x,CONS))
    return -1;
  for(tmp=x;TYPEP(x,CONS);x=CDR(x))
    count++;
  return count;
}

VCSI_OBJECT vcsi_list_length(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {
  int c;

  if(eq(vc,listq(vc,x),vc->false))
    error(vc,"object passed to length is the wrong type",x);
  
  if((c = get_length(x)) == -1)
    error(vc,"object passed to length is the wrong type",x);

  return make_long(vc,c);
}

VCSI_OBJECT vcsi_list_reverse(VCSI_CONTEXT vc,
			 VCSI_OBJECT list) {
  VCSI_OBJECT tmp, tmp2 = NULL;
  for(tmp=list;TYPEP(tmp,CONS);tmp=CDR(tmp))
    tmp2=cons(vc,CAR(tmp),tmp2);
  return tmp2;
}


VCSI_OBJECT vcsi_list_append(VCSI_CONTEXT vc,
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {

  if(!TYPEP(x,CONS) && x != NULL)
    return error(vc,"object passed to append is the wrong type",x);
  if(!x)
    return y;
  return cons(vc,CAR(x),vcsi_list_append(vc,CDR(x),y));
}

VCSI_OBJECT vcsi_list_ref(VCSI_CONTEXT vc,
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y) {
  VCSI_OBJECT tmp;
  int i,j;
  if(!TYPEP(x,CONS) && x != NULL)
    return error(vc,"object passed to list-ref is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    return error(vc,"object passed to list-ref is the wrong type",y);
   
  j = get_length(x);
  if(LNGN(y) >= j)
    return error(vc,"subscript out of range",y);
  for(tmp=x,i=0;TYPEP(tmp,CONS) && i < j && i < LNGN(y);tmp=CDR(tmp),i++);
  return CAR(tmp);
}

VCSI_OBJECT vcsi_list_ref_i(VCSI_CONTEXT vc,
		       VCSI_OBJECT x, 
		       int y) {
  VCSI_OBJECT tmp;
  int i,j;
  if(!TYPEP(x,CONS) && x != NULL)
    return error(vc,"object passed to list-ref is the wrong type",x);
   
  j = get_length(x);
  if(y >= j)
    return error(vc,"subscript out of range",cons(vc,x,make_long(vc,y)));
   
  for(tmp=x,i=0;TYPEP(tmp,CONS) && i < j && i < y;tmp=CDR(tmp),i++);
  return CAR(tmp);
}

VCSI_OBJECT vcsi_list_tail(VCSI_CONTEXT vc,
		      VCSI_OBJECT x, 
		      VCSI_OBJECT y) {
  VCSI_OBJECT tmp;
  int i=0,j=0;
  if(!TYPEP(x,CONS) && x != NULL)
    return error(vc,"object passed to list-tail is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    return error(vc,"object passed to list-tail is the wrong type",y);
   
  j = get_length(x);
  if(LNGN(y) >= j)
    return error(vc,"subscript out of range",y);
   
  for(tmp=x,i=0;TYPEP(tmp,CONS) && i < j && i != LNGN(y);tmp=CDR(tmp),i++);
  return tmp;
}

VCSI_OBJECT do_error(VCSI_CONTEXT vc,
		     VCSI_OBJECT args, 
		     VCSI_OBJECT env) {

  VCSI_OBJECT x, y, z;
  if(get_length(args) < 2)
    return error(vc,"invalid number of arguments to error",args);
  
  x = car(vc,args);
  y = cadr(vc,args);

  if(!TYPEP(x,STRING) && TYPEP(y,STRING)) {
    z = x;
    x = y;
    y = z;
  }

  check_arg_type(vc,x,STRING,"error");

  return error(vc,STR(x),y);
}
   
VCSI_OBJECT randobj(VCSI_CONTEXT vc,
		    VCSI_OBJECT args, 
		    int length) {
  VCSI_OBJECT tmp;
  int r_seed;
      
  if(length <= 0)
    return error(vc,"random requires at least one argument",args);
   
  if(length == 1) {
    tmp = CAR(args);
    if(TYPEP(tmp,LNGNUM)) {
      r_seed = (int)((float)LNGN(tmp)*rand()/(RAND_MAX));
      return make_long(vc,r_seed);
    }
    else
      return tmp;
  } else {
    r_seed = (int)((float)length*rand()/(RAND_MAX));
    return vcsi_list_ref_i(vc,args,r_seed);
  }
}

VCSI_OBJECT clstrace(VCSI_CONTEXT vc,
		  VCSI_OBJECT x) { 
  
  if(!TYPEP(x,CLOSURE))
    error(vc,"object passed to trace is incorrect type",x);
  CLSTRACE(x)=1;
  return vc->true;
}

VCSI_OBJECT clsuntrace(VCSI_CONTEXT vc,
		    VCSI_OBJECT x) { 
  
  if(!TYPEP(x,CLOSURE))
    error(vc,"object passed to untrace is incorrect type",x);
  CLSTRACE(x)=0;
  return vc->true;
}

VCSI_OBJECT do_sleep(VCSI_CONTEXT vc,
		     VCSI_OBJECT x) {

  check_arg_type(vc,x,LNGNUM,"sleep");
  if(LNGN(x) < 0)
    error(vc,"argument to sleep out of range",x);

  sleep(LNGN(x));

  return vc->true;
}

VCSI_OBJECT do_usleep(VCSI_CONTEXT vc,
		      VCSI_OBJECT x) {

#ifndef HAVE_USLEEP
  struct timeval tv;  
#endif

  check_arg_type(vc,x,LNGNUM,"usleep");
  if(LNGN(x) < 0)
    error(vc,"argument to usleep out of range",x);
#ifdef HAVE_USLEEP
  usleep(LNGN(x));
#else
  tv.tv_sec = 0;
  tv.tv_usec = LNGN(x);
  select(0,NULL,NULL,NULL,&tv);
#endif

  return vc->true;
}

int obj_compare_void(void* a, void* b) {
  return obj_compare_i((VCSI_OBJECT)a,(VCSI_OBJECT)b);
}

VCSI_OBJECT obj_sort(VCSI_CONTEXT vc,
		    VCSI_OBJECT x) {
  
  VCSI_OBJECT tmp, res;
  INFOSTD_PHEAP iph;
  unsigned long len;

  check_arg_type(vc,x,CONS,"sort");
  len = get_length(x);

  if(!len)
    return x;
  
  iph = infostd_pheap_init(INFOSTD_HEAP_MIN, obj_compare_void,len+1);
  
  for(tmp=x;TYPEP(tmp,CONS);tmp=cdr(vc,tmp))
    infostd_pheap_push(iph,(void*)car(vc,tmp));
  
  res = NULL;

  while((tmp=(VCSI_OBJECT)infostd_pheap_pop(iph))!=(void*)ULONG_MAX)
    res = cons(vc,tmp,res);

  infostd_pheap_free(iph);

  return vcsi_list_reverse(vc,res);
}


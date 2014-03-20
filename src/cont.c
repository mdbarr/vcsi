#include "vcsi.h"

void cont_init(VCSI_CONTEXT vc) {

  set_int_proc(vc,"call-with-current-continuation",PROCENV,call_w_cc);
  set_int_proc(vc,"call/cc",PROCENV,call_w_cc);
  set_int_proc(vc,"values",PROCOPT,make_values);
  set_int_proc(vc,"call-with-values",PROC2,call_with_values);
  set_int_proc(vc,"dynamic-wind",PROCENV,dynamic_wind);
}

VCSI_OBJECT make_continuation(VCSI_CONTEXT vc, 
			      VCSI_OBJECT tmp,
			      VCSI_OBJECT exp) {

  long *src, *dst;
  long len,i;
  VCSI_OBJECT sp;
   
  CONTPTR(tmp) = (VCSI_CONTINUATION)MALLOC(SIZE_VCSI_CONTINUATION);
  CONTJMP(tmp) = (sigjmp_buf*)MALLOC(sizeof(sigjmp_buf));
   
  if(DYNSTCK(vc->root_wind) > &sp) {
    src = (long*)&sp;
    len = DYNSTCK(vc->root_wind) - &sp;
  } else {
    src = (long*)DYNSTCK(vc->root_wind);
    len = &sp - DYNSTCK(vc->root_wind);
  }

  CONTSTCKS(tmp) = src;
  CONTLEN(tmp) = len;
  CONTSTCK(tmp) = (long*) MALLOC(sizeof(long)*len);
  dst = CONTSTCK(tmp);
   
  for(i=0;i<len;i++) {
#ifdef DEBUG
    printf("Mapping %ld: %p to %p -- %p\n",i,src,dst,*src);
#endif
    *dst++ = *src++;
  }
   
  CONTEXP(tmp) = exp;
  CONTWIND(tmp) = vc->root_wind;
  return tmp;
}

int within_dynamic_extent(VCSI_DYNWIND context,
			  VCSI_DYNWIND extent) {

  for(;extent;extent=DYNNEXT(extent)) {
    if(context==extent)
      return 1;
  }
  return 0;
}

VCSI_OBJECT call_w_cc(VCSI_CONTEXT vc, 
			   VCSI_OBJECT args, 
			   VCSI_OBJECT env) {

  VCSI_OBJECT tmp, exp;
  VCSI_DYNWIND curr, old_wind;
   
  if(get_length(args) != 1)
    return error(vc,"invalid number of arguments to call/cc",args);
   
  tmp = alloc_obj(vc,CONTINUATION);
   
  make_continuation(vc,tmp,CAR(args));
  CONTTYPE(tmp) = CALL_WITH_CC; /* So we know how it was created */

#ifdef DEBUG
  printf("Using %p at %p\n",tmp,&tmp);
#endif

  if(sigsetjmp(*CONTJMP(tmp),1)) {
    /* Re-entering this context from another */
    old_wind = vc->root_wind;

    if(CONTWIND(tmp) != vc->root_wind) {
      vc->root_wind = CONTWIND(tmp);

      /* sync the error jump */
      memcpy(DYNERR(vc->root_wind),DYNERR(old_wind),sizeof(sigjmp_buf));
    }

	
    for(exp=NULL,curr=vc->root_wind;curr;curr=DYNNEXT(curr)) { 
      if(!within_dynamic_extent(curr,old_wind) && DYNPRE(curr)) 
	exp = cons(vc,cons(vc,DYNPRE(curr),DYNROOT(curr)),exp); 
    } 
    for(;TYPEP(exp,CONS);exp=CDR(exp)) 
      eval(vc,CAR(CAR(exp)),CDR(CAR(exp)));
      
    /* eval the return only after we have properly re-entered this context */
    return eval(vc,CAR(CONTVAL(tmp)),CDR(CONTVAL(tmp)));
  }
  else
    return eval(vc,cons(vc,CAR(args),cons(vc,tmp,NULL)),env);
}

VCSI_OBJECT cont_grow(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  VCSI_OBJECT filler[100];
  VCSI_OBJECT tmp;
   
  tmp = x;
  filler[0] = NULL;
   
  return cont_throw(vc,tmp);
}
   
VCSI_OBJECT cont_throw(VCSI_CONTEXT vc, 
		       VCSI_OBJECT tmp) {

  long *src, *dst;
  long len,i;
   
  VCSI_DYNWIND curr;
  VCSI_OBJECT exp;
   
  VCSI_OBJECT sp;
   
  dst = CONTSTCKS(tmp);
  len = CONTLEN(tmp);
  src = CONTSTCK(tmp);
   
  if((long*)&sp < (dst+len) && (long*)&sp > dst)
    return cont_grow(vc,tmp);
   
  for(i=0;i<len;i++) {
#ifdef DEBUG
    printf("Mapping %ld: %p to %p -- %p\n",i,src,dst,*src);
#endif
    *dst++ = *src++;
  }
#ifdef DEBUG
  printf("Using %p at %p\n",tmp,&tmp);    
#endif

  /* leaving this context and entering another */
  for(exp=NULL,curr=vc->root_wind;curr;curr=DYNNEXT(curr)) {
    if(within_dynamic_extent(curr,CONTWIND(tmp)) && DYNPOST(curr))
      exp = cons(vc,cons(vc,DYNPOST(curr),DYNROOT(curr)),exp);
  }
  for(;TYPEP(exp,CONS);exp=CDR(exp))
    eval(vc,CAR(CAR(exp)),CDR(CAR(exp)));
   
  siglongjmp(*CONTJMP(tmp),1);
  return vc->novalue;
}

VCSI_OBJECT make_values(VCSI_CONTEXT vc,
			VCSI_OBJECT args,
			int length) {

  VCSI_OBJECT tmp;

  if(length == 1)
    return car(vc,args);

  tmp = alloc_obj(vc,VALUES);
  VVALS(tmp) = args;
  
  return tmp;
}

VCSI_OBJECT call_with_values(VCSI_CONTEXT vc,
			     VCSI_OBJECT producer,
			     VCSI_OBJECT consumer) {
  VCSI_OBJECT produced, args;

  produced = eval(vc,cons(vc,producer,NULL),DYNROOT(vc->root_wind));
  if(TYPEP(produced,VALUES)) 
    args = VVALS(produced);
  else 
    args = cons(vc,produced,NULL);
    
  return eval(vc,cons(vc,consumer,args),DYNROOT(vc->root_wind));
}

VCSI_OBJECT dynamic_wind(VCSI_CONTEXT vc, 
			 VCSI_OBJECT args, 
			 VCSI_OBJECT env) {

  VCSI_DYNWIND new_wind, old_wind;
  VCSI_OBJECT x, y, z;
  VCSI_OBJECT val;
      
  if(get_length(args) != 3)
    return error(vc,"invalid number of arguments to dynamic-wind",args);
   
  x = cons(vc,CAR(args),NULL);
  y = cons(vc,CAR(CDR(args)),NULL);
  z = cons(vc,CAR(CDR(CDR(args))),NULL);
   
  old_wind = vc->root_wind;
  new_wind = (VCSI_DYNWIND)MALLOC(SIZE_VCSI_DYNWIND);
  memset(new_wind,0,SIZE_VCSI_DYNWIND);

  DYNROOT(new_wind) = env;
  memcpy(DYNERR(new_wind),DYNERR(old_wind),sizeof(sigjmp_buf));
  DYNSTCK(new_wind) = DYNSTCK(old_wind);
  DYNPRE(new_wind) = x;
  DYNPOST(new_wind) = z;
  DYNNEXT(new_wind) = old_wind;
   
  vc->root_wind = new_wind;
   
  eval(vc,x,env);
  val = eval(vc,y,env);
  eval(vc,z,env);
   
  vc->root_wind = old_wind;
  return val;
}

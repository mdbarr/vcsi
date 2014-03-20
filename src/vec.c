#include "vcsi.h"

void vec_init(VCSI_CONTEXT vc) {

  set_int_proc(vc,"make-vector",PROCOPT,make_vector);
  set_int_proc(vc,"vector",PROCOPT,vector);
  set_int_proc(vc,"vector?",PROC1,vectorq);
  set_int_proc(vc,"vector-length",PROC1,vector_len);
  set_int_proc(vc,"vector-ref",PROC2,vector_ref);
  set_int_proc(vc,"vector-set!",PROC3,vector_set);
  set_int_proc(vc,"list->vector",PROC1,list_vector);
  set_int_proc(vc,"vector->list",PROC1,vector_list);
  set_int_proc(vc,"vector-fill",PROC2,vector_fill);
}

VCSI_OBJECT new_vector(VCSI_CONTEXT vc, 
		       int size, 
		       VCSI_OBJECT init) {

  VCSI_OBJECT tmp;
  int i;
   
  tmp = alloc_obj(vc,VECTOR);
   
  VSIZE(tmp) = size;
  VELEM(tmp) = MALLOC(SIZE_VCSI_OBJECT*size);
   
  for(i=0;i<VSIZE(tmp);i++)
    VELEM(tmp)[i] = init;
   
  return tmp;
}
   
VCSI_OBJECT make_vector(VCSI_CONTEXT vc, 
			VCSI_OBJECT args, 
			int length) {

  VCSI_OBJECT x, y = NULL;

  if(length < 1 || length > 2)
    return error(vc,"invalid number of arguments to make-vector",args);
   
  x = car(vc,args);

  if(!TYPEP(x,LNGNUM))
    return error(vc,"object passed to make-vector is the wrong type",x);
   
  if(length == 2) {
    y = cadr(vc,args);
  } 
  return new_vector(vc,LNGN(x),y);
}

VCSI_OBJECT vector(VCSI_CONTEXT vc, 
		   VCSI_OBJECT args, 
		   int length) {

  VCSI_OBJECT tmp, vec;
  int i;

  vec = new_vector(vc,length,NULL);
   
  for(tmp=args,i=0;TYPEP(tmp,CONS);tmp=CDR(tmp),i++)
    VELEM(vec)[i] = car(vc,tmp);
  return vec;
}

VCSI_OBJECT vectorq(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(TYPEP(x,VECTOR))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT vector_len(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  if(!TYPEP(x,VECTOR))
    return error(vc,"object passed to vector-length is the wrong type",x);
  return make_long(vc,VSIZE(x));
}

VCSI_OBJECT vector_ref(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y) {

  if(!TYPEP(x,VECTOR))
    return error(vc,"object passed to vector-ref is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    return error(vc,"object passed to vector-ref is the wrong type",y);
  if(LNGN(y) >= VSIZE(x))
    return error(vc,"subscript out of range in vector-ref",y);
  return VELEM(x)[LNGN(y)]; 
}

VCSI_OBJECT vector_set(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y, 
		       VCSI_OBJECT z) {

  if(!TYPEP(x,VECTOR))
    return error(vc,"object passed to vector-set! is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    return error(vc,"object passed to vector-set! is the wrong type",y);
  if(LNGN(y) >= VSIZE(x))
    return error(vc,"subscript out of range in vector-set!",y);
   
  VELEM(x)[LNGN(y)] = z;
   
  return vc->novalue;
}

VCSI_OBJECT list_vector(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  VCSI_OBJECT tmp, vec;
  int i;
   
  if(!TYPEP(x,CONS) && x != NULL)
    return error(vc,"object passed to list->vector is the wrong type",x);
   
  i = get_length(x);
    
  vec = new_vector(vc,i,NULL);
   
  for(tmp=x,i=0;TYPEP(tmp,CONS);tmp=CDR(tmp),i++)
    VELEM(vec)[i] = CAR(tmp);
   
  return vec;
}

VCSI_OBJECT vector_list(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  VCSI_OBJECT tmp = NULL;
  int i;
   
  if(!TYPEP(x,VECTOR))
    return error(vc,"object passed to vector->list is the wrong type",x);
   
  for(i=0;i<VSIZE(x);i++)
    tmp = cons(vc,VELEM(x)[i],tmp);
   
  return vcsi_list_reverse(vc,tmp);
}

VCSI_OBJECT vector_fill(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {
  int i;
   
  if(!TYPEP(x,VECTOR))
    return error(vc,"object passed to vector-fill is the wrong type",x);
   
  for(i=0;i<VSIZE(x);i++)
    VELEM(x)[i] = y;
   
  return vc->novalue;
}

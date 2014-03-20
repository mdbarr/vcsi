#include "vcsi.h"
/* Functions concerning the setting and retreival of objects in environments */
VCSI_OBJECT setvar(VCSI_CONTEXT vc, 
		   VCSI_OBJECT var,
		   VCSI_OBJECT val,
		   VCSI_OBJECT env) {

  VCSI_OBJECT tmp;
  if(TYPEN(var,SYMBOL))
    return error(vc,"non-symbol to set",var);

  tmp = envlookup(vc,var,env);

  if(tmp == NULL)
    tmp = var;

  if(TYPEP(tmp,SYMBOL) && SPROTECT(tmp) == 1)
    error(vc,"redefinition of syntactic keyword",tmp);

  return (VCELL(tmp) = val);
}

VCSI_OBJECT setcar(VCSI_CONTEXT vc, 
		   VCSI_OBJECT var, 
		   VCSI_OBJECT val, 
		   VCSI_OBJECT env) {

  VCSI_OBJECT tmp;
  if(TYPEN(var,CONS))
    return error(vc,"non-pair to set-car",var);

  tmp = envlookup(vc,var,env);

  if(tmp == NULL)
    tmp=var;

  return (CAR(tmp) = val);
}

VCSI_OBJECT setcdr(VCSI_CONTEXT vc, 
		   VCSI_OBJECT var, 
		   VCSI_OBJECT val, 
		   VCSI_OBJECT env) {

  VCSI_OBJECT tmp;
  if(TYPEN(var,CONS))
    return error(vc,"non-pair to set-cdr",var);

  tmp = envlookup(vc,var,env);

  if(tmp == NULL)
    tmp = var;

  return (CDR(tmp) = val);
}

VCSI_OBJECT set_int_proc(VCSI_CONTEXT vc, 
			 char* name, 
			 VCSI_OBJECT_TYPE type, 
			 VCSI_OBJECT (*proc)()) {
  
  return setvar(vc,make_internal(vc,name),make_proc(vc,name,type,proc),NULL);
}

VCSI_OBJECT envlookup(VCSI_CONTEXT vc, 
		      VCSI_OBJECT var, 
		      VCSI_OBJECT env) {

  VCSI_OBJECT enclosing, frame, tmp;

  /* Override the scope if necassary */
  if(SCOPE(var))
    env=SCOPE(var);

  for(enclosing=env;TYPEP(enclosing,CONS);enclosing=CDR(enclosing)) {
    for(frame=CAR(enclosing);TYPEP(frame,CONS);frame=CDR(frame)) {
      tmp = CAR(frame);
      if(eq(vc,tmp,var))
	return tmp;
    }
  }
  return NULL;
}

VCSI_OBJECT envlookup_str(VCSI_CONTEXT vc, 
			  char* var,
			  VCSI_OBJECT env) {

  VCSI_OBJECT enclosing, frame, tmp;

  for(enclosing=env;TYPEP(enclosing,CONS);enclosing=CDR(enclosing)) {
    for(frame=CAR(enclosing);TYPEP(frame,CONS);frame=CDR(frame)) {
      tmp = CAR(frame);
      if(TYPEP(tmp,SYMBOL)) {
	if(!strcasecmp(SNAME(tmp),var))
	  return tmp;
      }
    }
  }
  return NULL;
}

VCSI_OBJECT extend_env(VCSI_CONTEXT vc, 
		       char* func_name,
		       VCSI_OBJECT vars,
		       VCSI_OBJECT vals, 
		       VCSI_OBJECT env) {

  VCSI_OBJECT frame, var, val, tmp;
  int numargs, dotflag;
  char errbuf[128];

  for(numargs=0,dotflag=0,tmp=vars;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    numargs++;
  }
  if(tmp)
    dotflag=1;

  if(dotflag || TYPEP(vars,SYMBOL)) {
    if(get_length(vals) < numargs - 1) {
      snprintf(errbuf,127,"invalid number of arguments to %s(%d+)",
	       func_name, numargs);
      return error(vc,errbuf,vcsi_list_length(vc,vals));
    }
  } else {
    if(get_length(vals) != numargs) {
      snprintf(errbuf,127,"invalid number of arguments to %s(%d)",
	       func_name, numargs);
      return error(vc,errbuf,vcsi_list_length(vc,vals));
    }
  }

  frame = NULL;

  if(TYPEP(vars,SYMBOL)) {
    tmp = copy_symbol(vc,vars);
    VCELL(tmp) = vals;
    return cons(vc,cons(vc,tmp,NULL),env);
  }

  for(var=vars,val=vals;TYPEP(var,CONS);var=CDR(var)) {
    if(!TYPEP(CAR(var),SYMBOL))
      error(vc,"non-symbol binding",var);
    if(!TYPEP(val,CONS))
      error(vc,"null values to bindings",var);

    tmp = copy_symbol(vc,CAR(var));
    VCELL(tmp) = CAR(val);
    frame = cons(vc,tmp,frame);
     
    if(TYPEP(val,CONS))
      val = cdr(vc,val);
    else
      val = NULL;
  }

  if(var) { /* dotted list */
    tmp = copy_symbol(vc,var);
    VCELL(tmp) = val;
    frame = cons(vc,tmp,frame);
  }

  return cons(vc,frame,env);
}

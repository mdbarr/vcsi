#include "vcsi.h"
/* The Real Evaluator */
VCSI_OBJECT ceval(VCSI_CONTEXT vc,
		  VCSI_OBJECT exp, 
		  VCSI_OBJECT env, 
		  int arg_eval) {
  /* error buffer for better reporting*/
  char errbuf[128];
  char* func_name;

  /* temp, return value, and trace */
  VCSI_OBJECT tmp, sym, ret;
  /* procedures */
  VCSI_OBJECT arg1, arg2, arg3, arg4, arg5;
  /* let and loop constructs */
  VCSI_OBJECT vars, vals, body, binding;
  /* looping constructs */
  VCSI_OBJECT setv, test, steps;

  /* the tail context */
  VCSI_OBJECT tail_context = NULL;

  /* the stack pointers */
  VCSI_OBJECT stck_end, *stck_old;

  unsigned short i, tail_count=0;
  
#ifdef DEBUG
  if(vc->verbose) {
    printf("Eval: %s\n",print_obj(vc,vc->result,exp,1));
    printf("Env: %s\n",print_obj(vc,vc->result,env,1));
  }
#endif

  /* save new stack pointers */
  stck_old = DYNSTCKE(vc->root_wind);
  DYNSTCKE(vc->root_wind) = &stck_end;

  /* main evaluation loop */
 eval_loop:
  /* save number of tail calls */
  tail_count++;

  /* force evaluation of args on tail calls */
  if(tail_count > 1)
    arg_eval=1;

  /* saftey nullifier */
  tmp = sym = ret = NULL;
  arg1 = arg2 = arg3 = arg4 = arg5 = NULL;
  vars = vals = body = binding = NULL;
  setv = test = steps = NULL;
  
  /* update the tail context */
  switch TYPE(exp) {
  case 0:
    ret=NULL;
    goto eval_end;

    break;
  case SYMBOL:

    tmp = envlookup(vc,exp,env);
    if(tmp == NULL) {
      return error(vc,"undefined symbol", exp);
    }
    tmp = VCELL(tmp);

    if(eq(vc,tmp,vc->unbound))
      return error(vc,"unbound symbol", exp);

    ret=tmp;
    goto eval_end;

    break;
  case CONS:
    tmp = CAR(exp);
    /* Ok, we have a cons, lets figure out what the car is... */
    switch TYPE(tmp) {
    case SYMBOL:
      sym = tmp;

      tmp = envlookup(vc,tmp,env);
      if(tmp == NULL)
	return error(vc,"undefined symbol", CAR(exp));
      tmp = VCELL(tmp);
      /* Special symbol handler - for pseudo function handling */
      if(tmp == vc->specialobj) {
	exp = cdr(vc,exp);

	if(eq(vc,sym,vc->ifobj)) /* if handler */
	  goto eval_if;
	else if(eq(vc,sym,vc->condobj)) /* cond handler */
	  goto eval_cond;
	else if(eq(vc,sym,vc->caseobj)) /* case handler */
	  goto eval_case;
	else if(eq(vc,sym,vc->andobj)) /* and handler */
	  goto eval_and;
	else if(eq(vc,sym,vc->orobj)) /* or handler */
	  goto eval_or;
	else if(eq(vc,sym,vc->letobj)) /* let handler */
	  goto eval_let;
	else if(eq(vc,sym,vc->letstarobj)) /* let* handler */
	  goto eval_letstar;
	else if(eq(vc,sym,vc->letrecobj)) /* letrec handler */
	  goto eval_letrec;
	else if(eq(vc,sym,vc->beginobj)) /* begin handler */
	  goto eval_sequence;
	else if(eq(vc,sym,vc->doobj)) /* do handler */
	  goto eval_do;
	else if(eq(vc,sym,vc->letsynobj)) /* let-syntax handler */
	  goto eval_let_syntax;
	else if(eq(vc,sym,vc->letrecsynobj)) /* letrec-syntax handler */
	  goto eval_letrec_syntax;
	    
	return error(vc, "undefined special form", CAR(exp));
      }
	  
      if(eq(vc,tmp,vc->unbound))
	return error(vc, "unbound symbol", CAR(exp));

      break;

    case CONS:
      tmp = eval(vc,tmp,env);
      break;
    }

    if(sym)
      func_name = SNAME(sym);
    else {
      snprintf(errbuf,127,"%p",tmp);
      func_name = errbuf;
    }

    /* Now that we have done something with the car, 
       lets see if we need to execute a function... */
    switch TYPE(tmp) {
    case CLOSURE:
      /* if there is a previous context, decrement it's count
	 as the new context is replacing it on the stack
	 in the same space. Meaning the previous context
	 has completed. */    
      if(tail_context) 
	if(CLSTRACE(tail_context))
	  CLSDEPTH(tail_context)--;

      tail_context = tmp;
	  
      arg1 = CDR(exp);
      if(arg_eval)
	arg1 = eval_args(vc,arg1,env);
	  
      if(CLSTRACE(tail_context)) {
	CLSDEPTH(tail_context)++;

	for(i=1;i<CLSDEPTH(tail_context);i++)
	  fprintf(PORTFP(vc->port_curr_out),"|  ");

	print_obj(vc,vc->tmpstr,arg1,1);
	vc->tmpstr->buff[0] = ' ';

	fprintf(PORTFP(vc->port_curr_out),"[%s %s]\n",
		SNAME(sym),vc->tmpstr->buff);
      }
	  
      exp=CDR(CLSCODE(tmp)); /* tail call */
      env = extend_env(vc,func_name,CAR(CLSCODE(tmp)),arg1,CLSENV(tmp));
      goto eval_sequence;

      break;

    case MACRO:
      arg1 = CDR(exp);
	  	  
      exp=CDR(CLSCODE(tmp));
      env = extend_env(vc,func_name,CAR(CLSCODE(tmp)),arg1,CLSENV(tmp));
      
      /* substitute the pattern in */
      exp = eval_args(vc,exp,env);

      /* then eval the new pattern */

      goto eval_sequence;

      break;

    case SYNTAX:
      exp = find_pattern_match(vc,tmp,exp,env);
      if(!exp)
	return error(vc,"invalid pattern to syntax",tmp);

      env = SYNENV(tmp);

      if(TYPEP(exp,CONS))
	goto eval_sequence;
      else if(TYPEP(exp,VECTOR)) {
	ret=exp;
	goto eval_end;
      }

      goto eval_loop;

      break;

    case PROC0:
      if(get_length(cdr(vc,exp)) != 0) {
	snprintf(errbuf,127,"invalid number of arguments to %s(0)",PNAME(tmp));
	return error(vc, errbuf,CDR(exp));
      }
      ret=PROC(tmp)(vc);
      goto eval_end;

      break;
	  
    case PROC1:
      if(get_length(cdr(vc,exp)) != 1) {
	snprintf(errbuf,127,"invalid number of arguments to %s(1)",PNAME(tmp));
	return error(vc, errbuf, CDR(exp));
      }
      arg1 = cadr(vc,exp);

      if(arg_eval)
	arg1 = eval(vc,arg1,env);

      ret=PROC(tmp)(vc,arg1);
      goto eval_end;

      break;

    case PROC2:
      if(get_length(cdr(vc,exp)) != 2) {
	snprintf(errbuf,127,"invalid number of arguments to %s(2)",PNAME(tmp));
	return error(vc, errbuf, CDR(exp));
      }

      arg1 = cadr(vc,exp);
      arg2 = caddr(vc,exp);

      if(arg_eval) {
	arg1 = eval(vc,arg1,env);
	arg2 = eval(vc,arg2,env);
      }

      ret=PROC(tmp)(vc,arg1,arg2);
      goto eval_end;

      break;

    case PROC3:
      if(get_length(cdr(vc,exp)) != 3) {
	snprintf(errbuf,127,"invalid number of arguments to %s(3)",PNAME(tmp));
	return error(vc, errbuf, CDR(exp));
      }
	    
      arg1 = cadr(vc,exp);
      arg2 = caddr(vc,exp);
      arg3 = cadddr(vc,exp);

      if(arg_eval) {
	arg1 = eval(vc,arg1,env);
	arg2 = eval(vc,arg2,env);
	arg3 = eval(vc,arg3,env);
      }


      ret=PROC(tmp)(vc,arg1,arg2,arg3);
      goto eval_end;

      break;

    case PROC4:
      if(get_length(cdr(vc,exp)) != 4) {
	snprintf(errbuf,127,"invalid number of arguments to %s(4)",PNAME(tmp));
	return error(vc, errbuf, CDR(exp));
      }
      arg1 = cadr(vc,exp);
      arg2 = caddr(vc,exp);
      arg3 = cadddr(vc,exp);
      arg4 = cadddr(vc,cdr(vc,exp));

      if(arg_eval) {
	arg1 = eval(vc,arg1,env);
	arg2 = eval(vc,arg2,env);
	arg3 = eval(vc,arg3,env);
	arg4 = eval(vc,arg4,env);
      }


      ret=PROC(tmp)(vc,arg1,arg2,arg3,arg4);
      goto eval_end;

      break;

    case PROC5:
      if(get_length(cdr(vc,exp)) != 5) {
	snprintf(errbuf,127,"invalid number of arguments to %s(5)",PNAME(tmp));
	return error(vc, errbuf, CDR(exp));
      }

      arg1 = cadr(vc,exp);
      arg2 = caddr(vc,exp);
      arg3 = cadddr(vc,exp);
      arg4 = cadddr(vc,cdr(vc,exp));
      arg5 = cadddr(vc,cddr(vc,exp));

      if(arg_eval) {
	arg1 = eval(vc,arg1,env);
	arg2 = eval(vc,arg2,env);
	arg3 = eval(vc,arg3,env);
	arg4 = eval(vc,arg4,env);
	arg5 = eval(vc,arg5,env);
      }

      ret=PROC(tmp)(vc,arg1,arg2,arg3,arg4,arg5);
      goto eval_end;

      break;

    case PROCENV:
      arg1 = cdr(vc,exp);

      ret=PROC(tmp)(vc,arg1,env);
      goto eval_end;
      
      break;

    case PROCOPT:
      if(TYPEP(exp,CONS)) {
	arg1 = eval_args(vc,cdr(vc,exp),env);
      } else
	arg1 = NULL;
      
      ret=PROC(tmp)(vc,arg1,get_length(arg1));
      goto eval_end;
      
      break;		    
      
    case PROCX:
      switch(get_length(cdr(vc,exp))) {	
      case 0:
	ret=PROC(tmp)(vc,vc->novalue);      
	goto eval_end;
	break;
      case 1:
	arg1 = cadr(vc,exp);	
	if(arg_eval)
	  arg1 = eval(vc,arg1,env);	
	ret=PROC(tmp)(vc,arg1);
	goto eval_end;
      default:	 
	snprintf(errbuf,127,"invalid number of arguments to %s(1)",PNAME(tmp));
	return error(vc, errbuf, CDR(exp));
	break;
      }      
      break;

    case CONTINUATION:
      if(CONTTYPE(tmp) == CALL_WITH_CC && get_length(cdr(vc,exp)) != 1)
	return error(vc, "invalid number of arguments to call/cc",exp);
      CONTVAL(tmp) = cons(vc,cadr(vc,exp),env);
      cont_throw(vc,tmp);
      break;
    case USERDEF:
      if(UDTYPE(tmp) && UDTYPE(tmp)->ud_eval) {
	exp = cons(vc,UDTYPE(tmp)->ud_eval,cons(vc,tmp,cdr(vc,exp)));
	goto eval_loop;
      } else
	return error(vc, "object not applicable", tmp);
      break;
    default:
      return error(vc, "object not applicable", tmp);
    }
  case VECTOR:
    return error(vc,"unquoted vector constant",exp);
    break;
  default:
    ret=exp;
    goto eval_end;
  }

  /* tail body */
 eval_sequence:  
  for(tmp=exp;TYPEP(tmp,CONS) && TYPEP(CDR(tmp),CONS);tmp=CDR(tmp))
    ret = eval(vc,CAR(tmp),env);
  if(TYPEP(tmp,CONS))
    exp = CAR(tmp); /* tail call */
  else
    exp = vc->true; /* unspecified */

  goto eval_loop;

  /* if handler */
 eval_if:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of args to if",exp);
  if(!eq(vc,eval(vc,CAR(exp),env),vc->false)) {

    exp = CAR(CDR(exp)); /* tail call */
    goto eval_loop;

  } else if(get_length(exp) == 3) {
    exp = CAR(CDR(CDR(exp))); /* tail call */
    goto eval_loop;
  }

  ret=vc->false;
  goto eval_end;
	
  /* cond handler */
 eval_cond:
  if(get_length(exp) < 1)
    return error(vc, "invalid number of args to cond",exp);
  for(tmp=exp;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(eq(vc,tmp,vc->false)) {
      ret=vc->false;
      goto eval_end;
    }

    if(!TYPEP(tmp,CONS) && !TYPEP(CAR(tmp),CONS))
      continue;

    if(eq(vc,CAR(CAR(tmp)),vc->elseobj)) {
      exp = CDR(CAR(tmp));

      if(exp)
	goto eval_sequence;

      return error(vc, "bad else clause",CAR(tmp));
	
    }

    /* evaluate the test */
    ret = eval(vc,CAR(CAR(tmp)),env);

    if(!eq(vc,ret,vc->false)) {
      exp = CDR(CAR(tmp)); 

      if(exp)
	goto eval_sequence;

      goto eval_end;
    }

  }

  ret=vc->false;
  goto eval_end;

  /* case handler */
 eval_case:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of agruments to case",exp);
   
  arg1 = eval(vc,CAR(exp),env);
  for(exp=CDR(exp);TYPEP(exp,CONS);exp=CDR(exp)) {
    tmp=CAR(exp);
      
    for(arg2=CAR(tmp);TYPEP(arg2,CONS);arg2=CDR(arg2)) {
      if(eq(vc,arg1,CAR(arg2))) {
	exp=CDR(tmp);
	goto eval_sequence;
      }
    }
    if(eq(vc,vc->elseobj,CAR(tmp))) {
      exp=CDR(tmp);
      goto eval_sequence;
    }
  }

  ret=vc->novalue;
  goto eval_end;

  /* and handler */
 eval_and:
  for(tmp=exp;TYPEP(tmp,CONS) && TYPEP(CDR(tmp),CONS);tmp=CDR(tmp)) {
    ret = eval(vc,CAR(tmp),env);
    if(eq(vc,ret,vc->false))
      break;
  }
  
  if(TYPEP(tmp,CONS)) { 
    exp = CAR(tmp); /* tail call */
    goto eval_loop;
  }
  goto eval_end;
  
  /* or handler */
 eval_or:
  for(tmp=exp;TYPEP(tmp,CONS) && TYPEP(CDR(tmp),CONS);tmp=CDR(tmp)) {
    ret = eval(vc,CAR(tmp),env);
    if(!eq(vc,ret,vc->false))
      goto eval_end;
  }
  if(TYPEP(tmp,CONS)) { 
    exp = CAR(tmp); /* tail call */
    goto eval_loop;
  }
  goto eval_end;

  /* let handler */
 eval_let:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of args to let",exp);
   
  /* named let syntax */
  if(TYPEP(CAR(exp),SYMBOL)) {
    binding = CAR(exp);
    exp = CDR(exp);
  }

  body = CDR(exp);
  for(tmp=CAR(exp);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(!TYPEP(CAR(tmp),CONS))
      error(vc,"bad bindings to let",tmp);

    if(!TYPEP(CAR(CAR(tmp)),SYMBOL))
      error(vc,"non-symbol binding to let",CAR(CAR(tmp)));

    vars = cons(vc,CAR(CAR(tmp)),vars);
    vals = cons(vc,CAR(CDR(CAR(tmp))),vals);
  }
  vars = vcsi_list_reverse(vc,vars);
  vals = eval_args(vc,vcsi_list_reverse(vc,vals),env);
  
  if(binding) {
    /* build named binding */
    tmp = make_closure(vc,vcsi_list_append(vc,cons(vc,vars,NULL),body),NULL);

    vars = cons(vc,binding,vars);
    vals = cons(vc,tmp,vals);
  } 
  env = extend_env(vc,"let",vars,vals,env);
  if(binding) /* adjust the right environment binding */
    CLSENV(tmp)=env;
  exp = body;
  goto eval_sequence;
  

  /* let* handler */
 eval_letstar:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of args to let",exp);
   
  body = CDR(exp);
  for(tmp=CAR(exp);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(!TYPEP(CAR(tmp),CONS))
      error(vc,"bad bindings to let*",tmp);

    if(!TYPEP(CAR(CAR(tmp)),SYMBOL))
      error(vc,"non-symbol binding to let",CAR(CAR(tmp)));

    vars = cons(vc,CAR(CAR(tmp)),vars);
    vals = cons(vc,CAR(CDR(CAR(tmp))),vals);
  }
  vars = vcsi_list_reverse(vc,vars);
  vals = vcsi_list_reverse(vc,vals);

  /* eval left to right */
  for(;TYPEP(vars,CONS)&&TYPEP(vals,CONS);vars=CDR(vars),vals=CDR(vals))
    env = extend_env(vc,"let*",cons(vc,CAR(vars),NULL),cons(vc,eval(vc,CAR(vals),env),NULL),env);

  exp = body;
  goto eval_sequence;

  /* letrec handler */
 eval_letrec:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of args to let",exp);
   
  body = CDR(exp);
  for(tmp=CAR(exp);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(!TYPEP(CAR(tmp),CONS))
      error(vc,"bad bindings to letrec",tmp);

    if(!TYPEP(CAR(CAR(tmp)),SYMBOL))
      error(vc,"non-symbol binding to let",CAR(CAR(tmp)));

    vars = cons(vc,CAR(CAR(tmp)),vars);
    vals = cons(vc,CAR(CDR(CAR(tmp))),vals);
  }
  vars = vcsi_list_reverse(vc,vars);
  vals = vcsi_list_reverse(vc,vals);

  /* bind with unspecified */
  env = extend_env(vc,"letrec",vars,vals,env);

  /* then eval */
  for(;TYPEP(vars,CONS)&&TYPEP(vals,CONS);vars=CDR(vars),vals=CDR(vals))
    setvar(vc,CAR(vars),eval(vc,CAR(vals),env),env);

  exp = body;
  goto eval_sequence;

  /* let-syntax handler */
 eval_let_syntax:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of args to let",exp);
   
  /* named let syntax */
  if(TYPEP(CAR(exp),SYMBOL)) {
    binding = CAR(exp);
    exp = CDR(exp);
  }

  body = CDR(exp);
  for(tmp=CAR(exp);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(!TYPEP(CAR(tmp),CONS))
      error(vc,"bad bindings to let-syntax",tmp);

    if(!TYPEP(CAR(CAR(tmp)),SYMBOL))
      error(vc,"non-symbol binding to let",CAR(CAR(tmp)));

    vars = cons(vc,CAR(CAR(tmp)),vars);
    vals = cons(vc,make_syntax(vc,CAR(CDR(CAR(tmp))),env),vals);
  }
  vars = vcsi_list_reverse(vc,vars);
  vals = vcsi_list_reverse(vc,vals);
  
  if(binding) {
    /* build named binding */
    tmp = make_closure(vc,vcsi_list_append(vc,cons(vc,vars,NULL),body),NULL);

    vars = cons(vc,binding,vars);
    vals = cons(vc,tmp,vals);
  } 
  env = extend_env(vc,"let-syntax",vars,vals,env);
  if(binding) /* adjust the right environment binding */
    CLSENV(tmp)=env;
  exp = body;
  goto eval_sequence;

  /* letrec-syntax handler */
 eval_letrec_syntax:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of args to let",exp);
   
  body = CDR(exp);
  for(tmp=CAR(exp);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(!TYPEP(CAR(tmp),CONS))
      error(vc,"bad bindings to letrec",tmp);

    if(!TYPEP(CAR(CAR(tmp)),SYMBOL))
      error(vc,"non-symbol binding to let",CAR(CAR(tmp)));

    vars = cons(vc,CAR(CAR(tmp)),vars);
    vals = cons(vc,CAR(CDR(CAR(tmp))),vals);
  }
  vars = vcsi_list_reverse(vc,vars);
  vals = vcsi_list_reverse(vc,vals);

  /* bind with unspecified */
  env = extend_env(vc,"letrex-syntax",vars,vals,env);

  /* then eval */
  for(;TYPEP(vars,CONS)&&TYPEP(vals,CONS);vars=CDR(vars),vals=CDR(vals))
    setvar(vc,CAR(vars),make_syntax(vc,CAR(vals),env),env);

  exp = body;
  goto eval_sequence;


 eval_do:
  if(get_length(exp) < 2)
    return error(vc, "invalid number of arguments to do",exp);

  setv = make_symbol(vc,"set!");
   
  for(tmp=CAR(exp);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(get_length(CAR(tmp)) < 2)
      return error(vc, "invalid number of arguments to do",CAR(tmp));
    vars = cons(vc,CAR(CAR(tmp)),vars);
    vals = cons(vc,eval(vc,CAR(CDR(CAR(tmp))),env),vals);
    if(get_length(CAR(tmp)) == 3) {
      arg1 = CAR(CDR(CDR(CAR(tmp))));
      arg1 = cons(vc,setv,cons(vc,CAR(vars),cons(vc,arg1,NULL)));
      steps = cons(vc,arg1,steps);
    }
    if(get_length(CAR(tmp)) > 3)
      return error(vc, "invalid number of arguments to do",CAR(tmp));
  }

  tmp = CAR(CDR(exp));
  if(get_length(tmp) < 2)
    return error(vc, "invalid number or arguments to do",tmp);
  test = CAR(tmp);
  body = CDR(tmp);
   
  exp = CDR(CDR(exp));

  env = extend_env(vc,"do",vars,vals,env);
   
  while(eq(vc,eval(vc,test,env),vc->false)) {
    for(tmp=exp;TYPEP(tmp,CONS);tmp=CDR(tmp))
      eval(vc,CAR(tmp),env);
    for(tmp=steps;TYPEP(tmp,CONS);tmp=CDR(tmp))
      eval(vc,CAR(tmp),env);
  }
  
  exp = body;
  goto eval_sequence;

  /* end context handler */
 eval_end:
  if(tail_context) {
    if(CLSTRACE(tail_context)) {      
      for(i=1;i<CLSDEPTH(tail_context);i++)
	fprintf(PORTFP(vc->port_curr_out),"|  ");
      fprintf(PORTFP(vc->port_curr_out),"%s\n",print_obj(vc,vc->tmpstr,ret,1));
      CLSDEPTH(tail_context)--;
    }
  }

  /* restore the old stack pointers */
  DYNSTCKE(vc->root_wind) = stck_old;
    
  /* final catch-all */
  return ret;
}

VCSI_OBJECT eval_args(VCSI_CONTEXT vc,
		      VCSI_OBJECT args, 
		      VCSI_OBJECT env) {
  VCSI_OBJECT tmp, newargs = NULL;
   
  for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    newargs = cons(vc,eval(vc,CAR(tmp),env),newargs);
  }
  return vcsi_list_reverse(vc,newargs);
}

VCSI_OBJECT do_eval(VCSI_CONTEXT vc,
		    VCSI_OBJECT exp, 
		    VCSI_OBJECT env) {

  if(env != vc->report_env && !TYPEP(env,CONS))
    return error(vc, "invalid environment specifier to eval",env);
  return eval(vc,exp,env);
}

VCSI_OBJECT eval_lambda(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT env) {

  return make_closure(vc,args,env);
}

VCSI_OBJECT eval_define(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT env) {

  VCSI_OBJECT tmp, var, val;
   
  if(get_length(args) < 2)
    return error(vc, "invalid number of args to define",args);

  if(TYPEP(CAR(args),SYMBOL)) {
    var = CAR(args);
    val = CAR(CDR(args));

  } else if(TYPEP(CAR(args),CONS)) {
    var = CAR(CAR(args));
    if(!TYPEP(var,SYMBOL))
      return error(vc, "cannot define a non-symbol",var);
    val = eval(vc,make_lambda(vc,CDR(CAR(args)),CDR(args)),env);
  } else
    return error(vc, "cannot define a non-symbol", CAR(args));
  
  if(TYPEP(val,SYMBOL)) {
    tmp = envlookup(vc,val,env);
    if(tmp != NULL)
      val = VCELL(tmp);
    else
      return error(vc, "unbound symbol",tmp);
  } else
    val = eval(vc,val,env);
  /* define's scope is limited to the first frame of its environment */   
  tmp = envlookup(vc,var,cons(vc,CAR(env),NULL));
  if(tmp == NULL) {
    /* so that we don't pass procs, a variable that will get redefined
     * if we re-call that function that internally defines it
     */
    tmp = copy_symbol(vc,var);

    /* add the new symbol to the environment */
    CAR(env) = cons(vc,tmp,CAR(env));
  }
  /* actually define the val to the var */
  setvar(vc,tmp,val,cons(vc,CAR(env),NULL));
  /* return the name of what we are defining... much cleaner then returning
   * the value...
   */
  return tmp;
}

VCSI_OBJECT quote_transform(VCSI_CONTEXT vc,
			    VCSI_OBJECT x) {
  VCSI_OBJECT tmp;
  for(tmp=x;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(TYPEP(CAR(tmp),CONS)) {
      CAR(tmp) = quote_transform(vc,CAR(tmp));
    } 
  }
  return x;
}

VCSI_OBJECT eval_quoted(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT env) {

  if(get_length(args) < 1)
    return error(vc, "invalid number of args to quote",args);
  return quote_transform(vc,CAR(args));
}

VCSI_OBJECT eval_quasi_loop(VCSI_CONTEXT vc,
			    VCSI_OBJECT args, 
			    VCSI_OBJECT env) {

  VCSI_OBJECT x, tmp, res = NULL;

  if(!TYPEP(args,CONS))
    return args;
  for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(TYPEP(CAR(tmp),CONS)) {
      x = CAR(CAR(tmp));
      if(eq(vc,x,vc->unquote)) {
	x = CDR(CAR(tmp));
	if(TYPEP(x,CONS))
	  x = CAR(x);
	res = vcsi_list_append(vc,res,cons(vc,eval(vc,x,env),NULL));
	continue;
      } else if(eq(vc,x,vc->unquotespl)) {
	x = CDR(CAR(tmp));
	if(TYPEP(x,CONS))
	  x = CAR(x);
	x = eval(vc,x,env);
	if(eq(vc,listq(vc,x),vc->false))
	  return error(vc, "non-list to unquote-splicing",x);
	res = vcsi_list_append(vc,res,x);
	continue;
      } else
	res = vcsi_list_append(vc,res,cons(vc,eval_quasi_loop(vc,CAR(tmp),env),NULL));
    } else 
      res = vcsi_list_append(vc,res,cons(vc,CAR(tmp),NULL));
  }

  return res;
}

VCSI_OBJECT eval_quasiquote(VCSI_CONTEXT vc,
			    VCSI_OBJECT args, 
			    VCSI_OBJECT env) {

  VCSI_OBJECT tmp, res = NULL;
  int v = 0;
  
  if(!TYPEP(args,CONS))
    return args;
 
  if(TYPEP(CAR(args),VECTOR)) {
    v = 1;
    tmp = vector_list(vc,CAR(args));
  } else
    tmp = CAR(args);
   
  res = eval_quasi_loop(vc,tmp,env);

  if(v)
    return list_vector(vc,res);
  return res;
}

VCSI_OBJECT eval_define_macro(VCSI_CONTEXT vc,
			      VCSI_OBJECT args, 
			      VCSI_OBJECT env) {
  
  VCSI_OBJECT tmp, var, val;
   
  if(get_length(args) < 2)
    return error(vc, "invalid number of args to define",args);

  if(TYPEP(CAR(args),SYMBOL)) {
    var = CAR(args);
    val = CAR(CDR(args));

  } else if(TYPEP(CAR(args),CONS)) {
    var = CAR(CAR(args));
    if(!TYPEP(var,SYMBOL))
      return error(vc, "can not define a non-symbol",var);
    val = eval(vc,make_lambda(vc,CDR(CAR(args)),CDR(args)),env);
  } else
    return error(vc, "can not define a non-symbol", CAR(args));
  
  if(TYPEP(val,SYMBOL)) {
    tmp = envlookup(vc,val,env);
    if(tmp != NULL)
      val = VCELL(tmp);
    else
      return error(vc, "unbound symbol",tmp);
  } else
    val = eval(vc,val,env);

  if(!TYPEP(val,CLOSURE))
    return error(vc,"bad arguments",val);

  /* convert to a macro */
  val->type = MACRO;

  tmp = envlookup(vc,var,cons(vc,CAR(env),NULL));
  if(tmp == NULL) {
    tmp = copy_symbol(vc,var);
    CAR(env) = cons(vc,tmp,CAR(env));
  }

  setvar(vc,tmp,val,cons(vc,CAR(env),NULL));
  return tmp;
}

VCSI_OBJECT eval_define_syntax(VCSI_CONTEXT vc,
			       VCSI_OBJECT args, 
			       VCSI_OBJECT env) {
  
  VCSI_OBJECT var, syntax, tmp;
   
  if(get_length(args) != 2)
    return error(vc, "invalid number of args to define-syntax",args);

  var = car(vc,args);
  syntax = cadr(vc,args);

  check_arg_type(vc,var,SYMBOL,"define-syntax");

  syntax = make_syntax(vc,syntax,env);
  check_arg_type(vc,syntax,SYNTAX,"define-syntax");

  
  tmp = envlookup(vc,var,cons(vc,CAR(env),NULL));
  if(tmp == NULL) {
    tmp = copy_symbol(vc,var);
    CAR(env) = cons(vc,tmp,CAR(env));
  }

  setvar(vc,tmp,syntax,cons(vc,CAR(env),NULL));
  return tmp;
}
  

VCSI_OBJECT eval_set(VCSI_CONTEXT vc,
		     VCSI_OBJECT args, 
		     VCSI_OBJECT env) {

  if(get_length(args) !=  2)
    return error(vc, "invalid number of args to set!",args);
  return setvar(vc,CAR(args),eval(vc,CAR(CDR(args)),env),env);
}

VCSI_OBJECT eval_setcar(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT env) {

  if(get_length(args) != 2)
    return error(vc, "invalid number of args to set-car!",args);
  args = eval_args(vc,args,env);
  return setcar(vc,CAR(args),CAR(CDR(args)),env);
}

VCSI_OBJECT eval_setcdr(VCSI_CONTEXT vc,
			VCSI_OBJECT args, 
			VCSI_OBJECT env) {

  if(get_length(args) != 2)
    return error(vc, "invalid number of args to set-cdr!",args);
  args = eval_args(vc,args,env);
  return setcdr(vc,CAR(args),CAR(CDR(args)),env);
}

VCSI_OBJECT eval_list(VCSI_CONTEXT vc,
		      VCSI_OBJECT args, 
		      int length) {

  return args;
}

VCSI_OBJECT eval_not(VCSI_CONTEXT vc,
		     VCSI_OBJECT x) {

  if(eq(vc,x,vc->false))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT eval_map(VCSI_CONTEXT vc,
		     VCSI_OBJECT args, 
		     VCSI_OBJECT env) {

  VCSI_OBJECT proc, exps, tmp, t_args, res, fin;
  int l,m,i,j;

  proc = exps = tmp = t_args = res = fin = NULL;

  if(get_length(args) < 2)
    return error(vc,"invalid number of args to map",args);

  proc = CAR(args);
   
  for(tmp=CDR(args);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    exps = cons(vc,eval(vc,CAR(tmp),env),exps);
  }
  /*exps = vcsi_list_reverse(vc,exps);*/
   
  l = get_length(exps);
  m = get_length(CAR(exps));
   
  for(i=0;i<m;i++) {
    t_args = NULL;
	
    for(j=0;j<l;j++) {
      tmp = vcsi_list_ref_i(vc,vcsi_list_ref_i(vc,exps,j),i);
      t_args = cons(vc,tmp,t_args);
    }
    /*t_args = vcsi_list_reverse(vc,t_args);*/
	
    res = eval_no_args(vc,cons(vc,proc,t_args),env);
    fin = cons(vc,res,fin);
  }
  return vcsi_list_reverse(vc,fin);
}

VCSI_OBJECT eval_foreach(VCSI_CONTEXT vc,
			 VCSI_OBJECT args, 
			 VCSI_OBJECT env) {

  VCSI_OBJECT proc, exps, tmp, t_args, res;
  int l, m, i, j;

  proc = exps = tmp = t_args = res = NULL;
   
  if(get_length(args) < 2)
    return error(vc, "invalid number of arguments to for-each",args);
   
  proc = CAR(args);
   
  for(tmp=CDR(args);TYPEP(tmp,CONS);tmp=CDR(tmp))
    exps = cons(vc,eval(vc,CAR(tmp),env),exps);
   
  if(!TYPEP(exps,CONS))
    return error(vc, "object passed to for-each is not a list",exps);
   
  if(!TYPEP(CAR(exps),CONS))
    return error(vc, "object passed to for-each is not a list",CAR(exps));
   
  l = get_length(exps);
  m = get_length(CAR(exps));
   
  for(i=0;i<m;i++) {
    t_args = NULL;
	
    for(j=0;j<l;j++) {
      tmp = vcsi_list_ref_i(vc,vcsi_list_ref_i(vc,exps,j),i);
      t_args = cons(vc,tmp,t_args);
    }
	
    res = eval_no_args(vc,cons(vc,proc,t_args),env);
  }
  return vc->novalue;
}

VCSI_OBJECT eval_delay(VCSI_CONTEXT vc,
		       VCSI_OBJECT args, 
		       VCSI_OBJECT env) {
  VCSI_OBJECT tmp;
   
  if(get_length(args) != 1)
    return error(vc, "invalid number of arguments to delay",args);
   
  tmp = make_thunk(vc,CAR(args),env);
  tmp->type = PROMISED;
  return tmp;
}

VCSI_OBJECT eval_force(VCSI_CONTEXT vc,
		       VCSI_OBJECT x) {

  VCSI_OBJECT tmp;

  if(!TYPEP(x,PROMISED) && !TYPEP(x,PROMISEF))
    return error(vc, "object passed to force is the wrong type",x);
   
  if(TYPEP(x,PROMISEF))
    return THNKEXP(x);
  else {
    tmp = eval(vc,THNKEXP(x),THNKENV(x));

    x->type = PROMISEF;
    THNKEXP(x) = tmp;
    THNKENV(x) = NULL;
    return THNKEXP(x);
  }
}

VCSI_OBJECT eval_apply(VCSI_CONTEXT vc,
		       VCSI_OBJECT args, 
		       VCSI_OBJECT env) {
  
  VCSI_OBJECT proc = NULL, exps = NULL, tmp = NULL;

  if(get_length(args) < 2)
    return error(vc, "invalid number of arguments to apply",args);
   
  proc = CAR(args);
  for(tmp=CDR(args);TYPEP(CDR(tmp),CONS);tmp=CDR(tmp))
    exps = cons(vc,eval(vc,CAR(tmp),env),exps);
   
  exps = vcsi_list_append(vc,vcsi_list_reverse(vc,exps),eval(vc,CAR(tmp),env));
   
  return eval_no_args(vc,cons(vc,proc,exps),env);
}

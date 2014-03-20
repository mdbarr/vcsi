#include <EXTERN.h>        
#include <perl.h>

#include <vcsi.h>

const char module_name[] = "perl";

static PerlInterpreter *perl_embed;
/* maybe a mutex */

/* Returns perl value as a scheme one */
VCSI_OBJECT perl_return(VCSI_CONTEXT vc,
			SV* val) {
  if(SvIOK(val))
    return make_long(vc,SvIV(val));
  else if(SvNOK(val))
    return make_float(vc,SvNV(val));
  else     
    return make_string(vc,SvPV_nolen(val));
}

SV* perl_sv(VCSI_CONTEXT vc,
	    VCSI_OBJECT x) {
  
  /* basic type handling */
  if(TYPEP(x,LNGNUM))
    return newSViv(LNGN(x));
  else if(TYPEP(x,FLTNUM))
    return newSVnv(FLTN(x));
  else if(TYPEP(x,STRING))
    return newSVpvn(STR(x),SLEN(x));
  
  /* everything else */
  return Nullsv;
}

/* Evalute perl code */
VCSI_OBJECT perl_eval(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {
  
  SV *val;
  check_arg_type(vc,x,STRING,"perl-eval");

  /*printf("Trying to eval:\n%s\n",STR(x));*/
  
  val = eval_pv(STR(x),TRUE);
  if(val)
    return perl_return(vc,val);
  return vc->false;
}

/* run a perl function */
VCSI_OBJECT perl_func(VCSI_CONTEXT vc,
		      VCSI_OBJECT args,
		      int length) {

  dSP ;
  VCSI_OBJECT name,tmp,ret;
  SV* val;
  unsigned long len;
  int i, count;
  int flags = G_EVAL;

  /* stack stuff */ 
  ENTER ;
  SAVETMPS ;

  PUSHMARK(SP) ;
 
  if(length < 1)
    return error(vc,"invalid number of arguments to perl-func",args);

  if(length == 1)
    flags = flags | G_NOARGS;

  name = car(vc,args);
  args = cdr(vc,args);

  /* loop putting the args on the stack */
  for(tmp=args;tmp!=NULL;tmp=cdr(vc,tmp))  
    XPUSHs(sv_2mortal(perl_sv(vc,car(vc,tmp))));
 
  /* stack stuff */
  PUTBACK ;

  /* call the function */
  count = call_pv(STR(name),flags);

  /*printf("COUNT: %d\n",count);*/

  /* refresh the stack */
  SPAGAIN ;

  /* POP out results */
  for(i=0,ret=NULL;i<count;i++) {
    val = POPs;
    ret = cons(vc,perl_return(vc,val),ret);
  }

  if(cdr(vc,ret) == NULL)
    ret = car(vc,ret);
  
  /* clean up the stack */

  PUTBACK ;
  FREETMPS ;
  LEAVE ;

  /* return */
  return ret;
}

/* read a perl scalar */
VCSI_OBJECT perl_sc_var(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {

  SV* val;
  char* s;

  check_arg_type(vc,x,STRING,"perl-sc-var");

  s = STR(x);
  if(*s == '$')
    s++;

  val = get_sv(s,FALSE);
  
  if(val)
    return perl_return(vc,val);
  return vc->false;
}

void module_init(VCSI_CONTEXT vc) {
  /* create the perl environment */
  char *embedding[] = {"","-e","0"};


  perl_embed = perl_alloc();
  perl_construct(perl_embed);
  perl_parse(perl_embed,NULL,3,embedding,NULL);

  /* register the proc */
  set_int_proc(vc,"perl-eval",PROC1,perl_eval);  
  set_int_proc(vc,"perl-func",PROCOPT,perl_func);
  set_int_proc(vc,"perl-sc-var",PROC1,perl_sc_var);

  /* if ever we destroyed */
  /*
    perl_destruct(perl_embed);
    perl_free(perl_embed);
  */
}

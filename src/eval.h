/* Various evaluation functions */

#define eval(vc,exp,env) ceval((vc),(exp),(env),1)
#define eval_no_args(vc,exp,env) ceval((vc),(exp),(env),0)

VCSI_OBJECT ceval(VCSI_CONTEXT vc,
		  VCSI_OBJECT exp, 
		  VCSI_OBJECT env, 
		  int arg_eval);

VCSI_OBJECT eval_args(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT do_eval(VCSI_CONTEXT vc, VCSI_OBJECT exp, VCSI_OBJECT env);

VCSI_OBJECT eval_lambda(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_define(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT quote_transform(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT eval_quoted(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_quasiquote(VCSI_CONTEXT vc, 
			    VCSI_OBJECT args, 
			    VCSI_OBJECT env);

VCSI_OBJECT eval_define_macro(VCSI_CONTEXT vc,
			      VCSI_OBJECT args, 
			      VCSI_OBJECT env);

VCSI_OBJECT eval_define_syntax(VCSI_CONTEXT vc,
			       VCSI_OBJECT args, 
			       VCSI_OBJECT env);

VCSI_OBJECT eval_set(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_setcar(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_setcdr(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_list(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT eval_map(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_foreach(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_not(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT eval_delay(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT eval_force(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT eval_apply(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);

#include <vcsi.h>
#include "env.h"

const char module_name[] = "env";

void module_init(VCSI_CONTEXT vc) { 
   set_int_proc(vc,"env-get",PROC1,env_get);
   set_int_proc(vc,"env-set!",PROC2,env_set);
   set_int_proc(vc,"env-unset!",PROC1,env_unset);
}

VCSI_OBJECT env_get(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {
   VCSI_OBJECT tmp = NULL;
   char* s = NULL;

   if(!TYPEP(x,STRING))
     return error(vc,"Invalid type to env-get",x);
      
   s = getenv(STR(x));
   if(s) 
     tmp = make_string(vc,s);

   return tmp;
}

VCSI_OBJECT env_set(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x, 
		    VCSI_OBJECT y) {
  
  if(!TYPEP(x,STRING))
    return error(vc,"Invalid type to env-set!",x);
  if(!TYPEP(y,STRING))
    return error(vc,"Invalid type to env-set!",y);

  setenv(STR(x),STR(y),1);

  return vc->true;
}

VCSI_OBJECT env_unset(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {
 
   if(!TYPEP(x,STRING))
     return error(vc,"Invalid type to env-get",x);
      
   unsetenv(STR(x));
   
   return vc->true;
}

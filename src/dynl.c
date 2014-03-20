#include "vcsi.h"

#ifdef WITH_DYNAMIC_LOAD

void dynl_init(VCSI_CONTEXT vc)  {
  set_int_proc(vc,"dynamic-load",PROC1,dyn_load);
  set_int_proc(vc,"dynamic-reload",PROC1,dyn_reload);
  set_int_proc(vc,"module-loaded?",PROC1,module_loaded);
  set_int_proc(vc,"module-list",PROC0,module_list);
}

VCSI_OBJECT dyn_load_opt(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x,
			 int reload) {

  void* tmp;
  void (*func)();
  char* mname;
   
  check_arg_type(vc,x,STRING,"dynamic-load");

  tmp = dlopen(STR(x),RTLD_LAZY);
  if(!tmp) {
    infostd_dyn_str_strcpy(vc->tmpstr,VCSI_MOD_DIR);
    infostd_dyn_str_strcat(vc->tmpstr,STR(x));
    if(!strstr(vc->tmpstr->buff,".so"))
      infostd_dyn_str_strcat(vc->tmpstr,".so");
	
    tmp = dlopen(vc->tmpstr->buff,RTLD_LAZY);
	
    if(!tmp)
      return error(vc,"file does not exist",x);
  }
  
  mname = (char*)dlsym(tmp,"module_name");

  func = (void (*) ()) dlsym(tmp,"module_init");
  if(!func)
    return error(vc,"can't initialize module",x);
  
  if(mname) {
    if(infostd_hash_str_info(vc->dynl_hash,mname)==ULONG_MAX) {
      infostd_hash_str_add(vc->dynl_hash,mname,1);
      func(vc);
    } else if(reload) /* Re-run the function if in reload mode */
      func(vc);
  } else
    func(vc);
   
  return vc->true;
}

VCSI_OBJECT dyn_load(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  return dyn_load_opt(vc,x,0);
}

VCSI_OBJECT dyn_reload(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  return dyn_load_opt(vc,x,1);
}

VCSI_OBJECT module_loaded(VCSI_CONTEXT vc,
			  VCSI_OBJECT x) {
  
  check_arg_type(vc,x,STRING,"module-loaded?");
  
  if(infostd_hash_str_info(vc->dynl_hash,STR(x))!=ULONG_MAX)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT module_list(VCSI_CONTEXT vc) {
  
  VCSI_OBJECT list = NULL;
  INFOSTD_HASH_STR_TAB l;
  unsigned long i;

  for(i=0;i<vc->dynl_hash->size;i++) {
    if(vc->dynl_hash->tab[i]) {
      for(l=vc->dynl_hash->tab[i];l;l=l->next) {
	list = cons(vc,make_string(vc,l->str),list);
      }
    }
  }
  
  return list;
}
#endif

#include <vcsi.h>

const char module_name[] = "infostd-speller";

VCSI_USER_DEF infostd_speller_mod_type;
#define ISM(x) ((INFOSTD_SPELLER)(*x).data.userdef.ud_value)

void infostd_speller_mod_print(VCSI_CONTEXT vc,
			       INFOSTD_DYN_STR res, 
			       VCSI_OBJECT obj) {
  
  infostd_dyn_str_printf(vc->res_extra,"#[speller (%ld words)]",
			 ISM(obj)->dictionary->num_elem);
  
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void infostd_speller_mod_mark(VCSI_CONTEXT vc,
	      VCSI_OBJECT obj, 
	      int flag) {
  
}

void infostd_speller_mod_free(VCSI_CONTEXT vc,
			      VCSI_OBJECT obj) {
  
  infostd_speller_free(ISM(obj));
}

VCSI_OBJECT infostd_speller_mod_init(VCSI_CONTEXT vc) {

  VCSI_OBJECT tmp;
  INFOSTD_SPELLER isp;

  isp = infostd_speller_init(100);

  tmp = make_user_def(vc,(void*)isp,infostd_speller_mod_type);

  return tmp;
}

VCSI_OBJECT infostd_speller_mod_q(VCSI_CONTEXT vc,
				  VCSI_OBJECT x) {

  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    return vc->false;
  return vc->true;  
}

VCSI_OBJECT infostd_speller_mod_read(VCSI_CONTEXT vc,
				     VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  INFOSTD_SPELLER isp;
  FILE* fp;

  check_arg_type(vc,x,STRING,"speller-read");

  fp = fopen(STR(x),"r");
  if(!fp)
    error(vc,"speller can't open file",x);

  isp = infostd_speller_read(fp);

  fclose(fp);

  tmp = make_user_def(vc,(void*)isp,infostd_speller_mod_type);

  return tmp;
}

VCSI_OBJECT infostd_speller_mod_config(VCSI_CONTEXT vc,
				       VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  INFOSTD_SPELLER isp;

  check_arg_type(vc,x,STRING,"speller-load-config");


  isp = infostd_speller_load_config(STR(x));
  if(!isp)
    error(vc,"speller can't load config file",x);

  tmp = make_user_def(vc,(void*)isp,infostd_speller_mod_type);

  return tmp;
}

VCSI_OBJECT infostd_speller_mod_check(VCSI_CONTEXT vc,
				      VCSI_OBJECT x,
				      VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    error(vc,"objected passed to speller-correct? is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"speller-correct?");
 
  if(infostd_speller_check(ISM(x),STR(y)))
    return vc->true;

  return vc->false;
}

VCSI_OBJECT infostd_speller_mod_sugg(VCSI_CONTEXT vc,
				      VCSI_OBJECT x,
				      VCSI_OBJECT y) {
  VCSI_OBJECT list = NULL;
  INFOSTD_DYN_STRS idss;
  unsigned long n;
  long i;
   
  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    error(vc,"objected passed to speller-suggestions is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"speller-suggestions");

  idss = infostd_dyn_strs_init(100);

  if((n=infostd_speller_suggestions(ISM(x),STR(y),idss))==0)
    return NULL;

  for(i=n-1;i>=0;i--)
    list = cons(vc,make_string(vc,infostd_dyn_strs_get(idss,i)),
		list);

  infostd_dyn_strs_free(idss);

  return list;
}

VCSI_OBJECT infostd_speller_mod_add_word(VCSI_CONTEXT vc,
					 VCSI_OBJECT x,
					 VCSI_OBJECT y) {
  
  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    error(vc,"objected passed to speller-add-word is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"speller-add-word");

  infostd_speller_add(ISM(x),STR(y));

  return vc->true;
}

VCSI_OBJECT infostd_speller_mod_add_ignore(VCSI_CONTEXT vc,
					   VCSI_OBJECT x,
					   VCSI_OBJECT y) {
  
  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    error(vc,"objected passed to speller-add-ignore is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"speller-add-ignore");

  infostd_speller_add_ignore(ISM(x),STR(y));

  return vc->true;
}

VCSI_OBJECT infostd_speller_mod_add_map(VCSI_CONTEXT vc,
					VCSI_OBJECT x,
					VCSI_OBJECT y,
					VCSI_OBJECT z) {
  
  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    error(vc,"objected passed to speller-add-mapping is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"speller-add-mapping");
  check_arg_type(vc,z,STRING,"speller-add-mapping");

  infostd_speller_add_mapping(ISM(x),STR(y),STR(z));

  return vc->true;
}

VCSI_OBJECT infostd_speller_mod_write(VCSI_CONTEXT vc,
				      VCSI_OBJECT x,
				      VCSI_OBJECT y) {

  FILE *fp;
  
  if(!same_type_user_def(vc,x,infostd_speller_mod_type))
    error(vc,"objected passed to speller-write is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"speller-write");

  fp = fopen(STR(y),"w");
  if(!fp)
    error(vc,"speller can't open file for writing.",y);  

  infostd_speller_write(ISM(x),fp);

  fclose(fp);

  return vc->true;
}


void module_init(VCSI_CONTEXT vc) { 

  infostd_speller_mod_type = make_user_def_type(vc, (char*)module_name,
						infostd_speller_mod_print,
						infostd_speller_mod_mark,
						infostd_speller_mod_free);
  
    set_int_proc(vc,"make-speller",PROC0,infostd_speller_mod_init);
    set_int_proc(vc,"speller?",PROC1,infostd_speller_mod_q);

    set_int_proc(vc,"speller-read",PROC1,infostd_speller_mod_read);
    set_int_proc(vc,"speller-load-config",PROC1,infostd_speller_mod_config);   
    
    set_int_proc(vc,"speller-correct?",PROC2,infostd_speller_mod_check);   
    set_int_proc(vc,"speller-suggestions",PROC2,infostd_speller_mod_sugg);

    set_int_proc(vc,"speller-add-word",PROC2,infostd_speller_mod_add_word);
    set_int_proc(vc,"speller-add-ignore",PROC2,infostd_speller_mod_add_ignore);
    set_int_proc(vc,"speller-add-mapping",PROC3,infostd_speller_mod_add_map);

    set_int_proc(vc,"speller-write",PROC2,infostd_speller_mod_write);
}

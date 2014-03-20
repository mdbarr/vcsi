#include <vcsi.h>

const char module_name[] = "infostd-stemmer";

VCSI_USER_DEF infostd_stemmer_mod_type;
#define ISM(x) ((INFOSTD_STEMMER)(*x).data.userdef.ud_value)

void infostd_stemmer_mod_print(VCSI_CONTEXT vc,
			       INFOSTD_DYN_STR res, 
			       VCSI_OBJECT obj) {
  
  infostd_dyn_str_printf(vc->res_extra,"#[stemmer (%ld words)]",
			 ISM(obj)->roots->num_elem);
  
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void infostd_stemmer_mod_mark(VCSI_CONTEXT vc,
			      VCSI_OBJECT obj, 
			      int flag) {
  
}

void infostd_stemmer_mod_free(VCSI_CONTEXT vc,
			      VCSI_OBJECT obj) {
  
  infostd_stemmer_free(ISM(obj));
}

VCSI_OBJECT infostd_stemmer_mod_init(VCSI_CONTEXT vc) {

  VCSI_OBJECT tmp;
  INFOSTD_STEMMER isp;

  isp = infostd_stemmer_init(100);

  tmp = make_user_def(vc,(void*)isp,infostd_stemmer_mod_type);

  return tmp;
}

VCSI_OBJECT infostd_stemmer_mod_q(VCSI_CONTEXT vc,
				  VCSI_OBJECT x) {

  if(!same_type_user_def(vc,x,infostd_stemmer_mod_type))
    return vc->false;
  return vc->true;  
}

VCSI_OBJECT infostd_stemmer_mod_read(VCSI_CONTEXT vc,
				     VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  INFOSTD_STEMMER isp;
  FILE* fp;

  check_arg_type(vc,x,STRING,"stemmer-read");

  fp = fopen(STR(x),"r");
  if(!fp)
    error(vc,"stemmer can't open file",x);

  isp = infostd_stemmer_read(fp);

  fclose(fp);

  tmp = make_user_def(vc,(void*)isp,infostd_stemmer_mod_type);

  return tmp;
}

VCSI_OBJECT infostd_stemmer_mod_root(VCSI_CONTEXT vc,
				     VCSI_OBJECT x,
				     VCSI_OBJECT y) {

  INFOSTD_DYN_STR ds;
  VCSI_OBJECT ret = y;

  if(!same_type_user_def(vc,x,infostd_stemmer_mod_type))
    error(vc,"objected passed to stemmer-correct? is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"stemmer-root");
 
  ds = infostd_dyn_str_init(100);

  if(infostd_stemmer_root(ISM(x),STR(y),ds)) {
    ret = make_string(vc,infostd_dyn_str_buff(ds));
  }

  infostd_dyn_str_free(ds);

  return ret;
}

VCSI_OBJECT infostd_stemmer_mod_forms(VCSI_CONTEXT vc,
				      VCSI_OBJECT x,
				      VCSI_OBJECT y) {

  INFOSTD_DYN_STR ds;
  VCSI_OBJECT ret = NULL;
  unsigned long i, len;
  char* buff;

  if(!same_type_user_def(vc,x,infostd_stemmer_mod_type))
    error(vc,"objected passed to stemmer-correct? is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"stemmer-root");
 
  ds = infostd_dyn_str_init(100);

  if(infostd_stemmer_forms(ISM(x),STR(y),ds)) {
    len = infostd_dyn_str_length(ds)+1;
    for(buff=ds->buff, i=0;i<len;i++) {
      if(ds->buff[i] == ' ' || ds->buff[i] == 0) {
	ds->buff[i] = 0;
	ret = cons(vc,make_string(vc,buff),ret);
	buff = ds->buff + (i+1);
      }
    }
    ret = vcsi_list_reverse(vc,ret);
  }

  infostd_dyn_str_free(ds);

  if(ret == NULL)
    ret = cons(vc,y,NULL);
  
  return ret;
}

VCSI_OBJECT infostd_stemmer_mod_write(VCSI_CONTEXT vc,
				      VCSI_OBJECT x,
				      VCSI_OBJECT y) {

  FILE *fp;
  
  if(!same_type_user_def(vc,x,infostd_stemmer_mod_type))
    error(vc,"objected passed to stemmer-write is of the wrong type",x);
  
  check_arg_type(vc,y,STRING,"stemmer-write");

  fp = fopen(STR(y),"w");
  if(!fp)
    error(vc,"stemmer can't open file for writing.",y);  

  infostd_stemmer_write(ISM(x),fp);

  fclose(fp);

  return vc->true;
}


void module_init(VCSI_CONTEXT vc) { 

  infostd_stemmer_mod_type = make_user_def_type(vc, (char*)module_name,
						infostd_stemmer_mod_print,
						infostd_stemmer_mod_mark,
						infostd_stemmer_mod_free);
  
    set_int_proc(vc,"make-stemmer",PROC0,infostd_stemmer_mod_init);
    set_int_proc(vc,"stemmer?",PROC1,infostd_stemmer_mod_q);

    set_int_proc(vc,"stemmer-read",PROC1,infostd_stemmer_mod_read);

    set_int_proc(vc,"stemmer-root",PROC2,infostd_stemmer_mod_root);
    set_int_proc(vc,"stemmer-forms",PROC2,infostd_stemmer_mod_forms);

    set_int_proc(vc,"stemmer-write",PROC2,infostd_stemmer_mod_write);
}

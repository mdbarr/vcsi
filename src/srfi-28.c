/* Scheme Request for Implementation 28 - Format String */
#include "vcsi.h"
void srfi_28_init(VCSI_CONTEXT vc) {
  set_int_proc(vc,"format",PROCOPT,srfi_28_format);
}

VCSI_OBJECT srfi_28_format(VCSI_CONTEXT vc,
			   VCSI_OBJECT args,
			   int length) {
  VCSI_OBJECT str, tmp;
  unsigned long i;
  int in_esc;
  char c[2];
  char* s;
  
  if(length < 1)
    return error(vc,"incorrect number of arguments to format",args);

  str = car(vc,args);
  check_arg_type(vc,str,STRING,"format");

  args = cdr(vc,args);

  infostd_dyn_str_clear(vc->tmpstr);

  for(i=0,in_esc=0;i<SLEN(str);i++) {
    if(in_esc) {
      if(STR(str)[i] == 'a' || STR(str)[i] == 's') {
	if(!TYPEP(args,CONS))
	  error(vc,"incomplete escape sequence",args);
	tmp = car(vc,args);
	args = cdr(vc,args);
	
	if(STR(str)[i] == 'a') {
	  if(TYPEP(tmp,STRING))
	    s = STR(tmp);
	  else if(TYPEP(tmp,CHAR)) {
	    c[0] = CHAR(tmp);
	    c[1] = '\0';
	    s = c;
	  } else
	    s = print_obj(vc,vc->result,tmp,1);
	} else 
	  s = print_obj(vc,vc->result,tmp,1);
	
	infostd_dyn_str_strcat(vc->tmpstr,s);
      } else if(STR(str)[i] == '%') {
	infostd_dyn_str_addchar(vc->tmpstr,'\n');
      } else if(STR(str)[i] == '~') {	
	infostd_dyn_str_addchar(vc->tmpstr,'~');
      } else
	return error(vc,"unrecognized escape sequence",str);
      
      in_esc = 0;
    } else if(STR(str)[i] == '~')
	in_esc = 1;
    else {
      infostd_dyn_str_addchar(vc->tmpstr,STR(str)[i]);
    }
  }
  
  return make_string(vc,vc->tmpstr->buff);
}

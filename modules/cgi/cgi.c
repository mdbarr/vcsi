#include <vcsi.h>
#include "cgi.h"

const char module_name[] = "cgi";

void module_init(VCSI_CONTEXT vc) {
  cgi_var = make_internal(vc,"cgi-variables");
  setvar(vc,cgi_var,NULL,NULL);
  set_int_proc(vc,"cgi-parse",PROC0,cgi_parse);
  set_int_proc(vc,"cgi-param",PROC1,cgi_param);
}   

VCSI_OBJECT cgi_parse(VCSI_CONTEXT vc) {
  VCSI_OBJECT tmp = NULL;
  char *s, *v, *qstr = NULL;
  char* buf;
  int c=0, l=0, count=0;
  
  /* QUERY_STRING */
  qstr = getenv("QUERY_STRING");
  if(qstr) {
    qstr=strdup(qstr);
    for(s=strtok(qstr,"&");s!=NULL;s=strtok(NULL,"&")) {

      if(strlen(s) < 2)
	continue;

      c = strcspn(s,"=");
      s[c]=0; c++;
      tmp = make_string(vc,s+c);
      tmp = cons(vc,make_string(vc,s),tmp);
      VCELL(cgi_var)=cons(vc,tmp,VCELL(cgi_var));
      tmp = NULL;
      count++;
    }
    free(qstr);
  }

  /* Content Length */
  s = getenv("CONTENT_LENGTH");
  if(s)
    l = atol(s);

  if(l > 0) {
    buf = (char*)malloc(sizeof(char)*l);

    c=read(0,buf,l);
    if(c==l) {
      
      for(c=0;c<l;c++) {
	if(buf[c] == '\n' || buf[c] == '\r')
	  buf[c] = '&';
      }
     
      for(s=strtok(buf,"&");s!=NULL;s=strtok(NULL,"&")) {
	
	if(strlen(s) < 2)
	  continue;
	
	c = strcspn(s,"=");
	s[c]=0; c++;
	tmp = make_string(vc,s+c);
	tmp = cons(vc,make_string(vc,s),tmp);
	VCELL(cgi_var)=cons(vc,tmp,VCELL(cgi_var));
	tmp = NULL;
	count++;
      } 
    }
    free(buf);
  }

  VCELL(cgi_var) = vcsi_list_reverse(vc,VCELL(cgi_var));

  if(count)
    return vc->true;
  
  return vc->false;
}

VCSI_OBJECT cgi_param(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {
  VCSI_OBJECT tmp;
  
  if(!TYPEP(x,STRING))
    return error(vc,"Invalid type to cgi-param",x);

  for(tmp=VCELL(cgi_var);tmp!=NULL&&TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    if(!TYPEP(CAR(tmp),CONS))
      continue;
    if(!strcasecmp(STR(x),STR(CAR(CAR(tmp)))))
      return CDR(CAR(tmp));
  }

  return vc->false;
}

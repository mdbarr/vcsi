#include "vcsi.h"

void regexp_init(VCSI_CONTEXT vc) {

  set_int_proc(vc,"regexp?",PROC1,regexpq);

  set_int_proc(vc,"regexp-compile",PROC1,regexp_compile);
  set_int_proc(vc,"regexp-match?",PROC2,regexp_match);
  set_int_proc(vc,"regexp-matches",PROC2,regexp_matches);

  set_int_proc(vc,"regexp-compile-ci",PROC1,regexp_compile_ci);
  set_int_proc(vc,"regexp-match-ci?",PROC2,regexp_match_ci);
  set_int_proc(vc,"regexp-matches-ci",PROC2,regexp_matches_ci);

  set_int_proc(vc,"=~",PROC2,regexp_full);

  REGEXP_MM[0] = make_internal(vc,"$0");
  REGEXP_MM[1] = make_internal(vc,"$1");
  REGEXP_MM[2] = make_internal(vc,"$2");
  REGEXP_MM[3] = make_internal(vc,"$3");
  REGEXP_MM[4] = make_internal(vc,"$4");
  REGEXP_MM[5] = make_internal(vc,"$5");
  REGEXP_MM[6] = make_internal(vc,"$6");
  REGEXP_MM[7] = make_internal(vc,"$7");
  REGEXP_MM[8] = make_internal(vc,"$8");
  REGEXP_MM[9] = make_internal(vc,"$9");
}

VCSI_OBJECT regexpq(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(TYPEP(x,REGEXPN))
    return vc->true;
  return vc->false;
}

/* case sensitive */
VCSI_OBJECT regexp_compile(VCSI_CONTEXT vc, 
			   VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  
  check_arg_type(vc,x,STRING,"regexp-compile");
  
  tmp = alloc_obj(vc,REGEXPN);
  REGEXPNV(tmp) = (regex_t*)MALLOC(sizeof(regex_t));

  if(regcomp(REGEXPNV(tmp),STR(x),REG_EXTENDED) < 0) /* no fancy options */
    return error(vc,"regexp compilation error", x);
  else
    return tmp;
}

VCSI_OBJECT regexp_match(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x, 
			 VCSI_OBJECT y) {

  regmatch_t *pmatch = NULL; 
  
  if(TYPEP(x,STRING))
    x = regexp_compile(vc,x);
  
  check_arg_type(vc,x,REGEXPN,"regexp-match?");
  check_arg_type(vc,y,STRING,"regexp-match?");
  
  if(regexec(REGEXPNV(x), STR(y), 0, pmatch, 0) != REG_NOMATCH)
    return vc->true;
  
  return vc->false;
}

VCSI_OBJECT regexp_matches(VCSI_CONTEXT vc, 
			   VCSI_OBJECT x, 
			   VCSI_OBJECT y)  { 

  VCSI_OBJECT tmp = NULL;
  regmatch_t pmatch[10];
  int i;

  if(TYPEP(x,STRING))
    x = regexp_compile(vc,x);

  check_arg_type(vc,x,REGEXPN,"regexp-matches");
  check_arg_type(vc,y,STRING,"regexp-matches");

  if(regexec(REGEXPNV(x), STR(y), 10, pmatch, 0) != REG_NOMATCH) {
    for(i=0;i<10;i++) {
      if(pmatch[i].rm_so != -1 && pmatch[i].rm_so < SLEN(y))
	tmp = cons(vc,string_sub_i(vc,y,pmatch[i].rm_so,pmatch[i].rm_eo),tmp);
    }
    return vcsi_list_reverse(vc,tmp);
  }
  return vc->false;
}

/* case insensitive */
VCSI_OBJECT regexp_compile_ci(VCSI_CONTEXT vc, 
			      VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  
  check_arg_type(vc,x,STRING,"regexp-compile-ci");
  
  tmp = alloc_obj(vc,REGEXPN);
  REGEXPNV(tmp) = (regex_t*)MALLOC(sizeof(regex_t));

  if(regcomp(REGEXPNV(tmp),STR(x),REG_EXTENDED|REG_ICASE) < 0) 
    return error(vc,"regexp compilation error", x);
  else
    return tmp;
}

VCSI_OBJECT regexp_match_ci(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y) {

  regmatch_t *pmatch = NULL; 
  
  if(TYPEP(x,STRING))
    x = regexp_compile_ci(vc,x);
  
  check_arg_type(vc,x,REGEXPN,"regexp-match-ci?");
  check_arg_type(vc,y,STRING,"regexp-match-ci?");
  
  if(regexec(REGEXPNV(x), STR(y), 0, pmatch, 0) != REG_NOMATCH)
    return vc->true;
  
  return vc->false;
}

VCSI_OBJECT regexp_matches_ci(VCSI_CONTEXT vc, 
			      VCSI_OBJECT x, 
			      VCSI_OBJECT y)  { 
  
  VCSI_OBJECT tmp = NULL;
  regmatch_t pmatch[10];
  int i;

  if(TYPEP(x,STRING))
    x = regexp_compile_ci(vc,x);

  check_arg_type(vc,x,REGEXPN,"regexp-matches-ci");
  check_arg_type(vc,y,STRING,"regexp-matches-ci");

  if(regexec(REGEXPNV(x), STR(y), 10, pmatch, 0) != REG_NOMATCH) {
    for(i=0;i<10;i++) {
      if(pmatch[i].rm_so != -1 && pmatch[i].rm_so < SLEN(y))
	tmp = cons(vc,string_sub_i(vc,y,pmatch[i].rm_so,
				   pmatch[i].rm_eo),tmp);
    }
    return vcsi_list_reverse(vc,tmp);
  }
  return vc->false;
}

VCSI_OBJECT regexp_full(VCSI_CONTEXT vc,
			VCSI_OBJECT s,
			VCSI_OBJECT r) {

  INFOSTD_REGEXP_ENGINE engine;
  INFOSTD_REGEXP_RESULT result; 
  VCSI_OBJECT res = NULL;
  VCSI_OBJECT obj = NULL;
  unsigned long i, len;
  char *tmp;

  check_arg_type(vc,s,STRING,"=~");
  check_arg_type(vc,r,STRING,"=~");

  engine = infostd_regexp_engine_init();
  result = infostd_regexp_result_init();
  infostd_regexp(engine,result,STR(s),SLEN(s),STR(r));

  switch (result->type) {
  case INFOSTD_REGEXP_ERROR:
    res = make_string(vc,result->result->buff);

    infostd_regexp_engine_free(engine);
    infostd_regexp_result_free(result);

    return error(vc,STR(res),r);
    break;
  case INFOSTD_REGEXP_NONE:
  case INFOSTD_REGEXP_NOMATCH:
    res = vc->false;
    break;
  case INFOSTD_REGEXP_SUBSTITUTION:
    free(STR(s));

    tmp = strdup(result->result->buff);
    STR(s) = tmp;
    SLEN(s) = result->result->length;

    res = s;
    break;
  case INFOSTD_REGEXP_MATCH:
    for(i=0,len=0;i<result->num_matches;i++) {
      obj = make_string(vc,result->result->buff+len);

      if(i<10) VCELL(REGEXP_MM[i]) = obj;

      res = cons(vc,obj,res);
      len+=strlen(result->result->buff+len)+1;
    }

    res = vcsi_list_reverse(vc,res);
    break;
  }
  
  infostd_regexp_engine_free(engine);
  infostd_regexp_result_free(result);

  return res;
}

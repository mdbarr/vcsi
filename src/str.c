#include "vcsi.h"

void str_init(VCSI_CONTEXT vc) {

  set_int_proc(vc,"char?",PROC1,charq);
  set_int_proc(vc,"char=?",PROC2,chareq);
  set_int_proc(vc,"char<?",PROC2,charless);
  set_int_proc(vc,"char>?",PROC2,chargreater);
  set_int_proc(vc,"char<=?",PROC2,charlesseq);
  set_int_proc(vc,"char>=?",PROC2,chargreatereq);
   
  set_int_proc(vc,"char-ci=?",PROC2,chareqci);
  set_int_proc(vc,"char-ci<?",PROC2,charlessci);
  set_int_proc(vc,"char-ci>?",PROC2,chargreaterci);
  set_int_proc(vc,"char-ci<=?",PROC2,charlesseqci);
  set_int_proc(vc,"char-ci>=?",PROC2,chargreatereqci);

  set_int_proc(vc,"char->integer",PROC1,char_int);
  set_int_proc(vc,"integer->char",PROC1,int_char);
   
  set_int_proc(vc,"char-alphabetic?",PROC1,char_alpha);
  set_int_proc(vc,"char-numeric?",PROC1,char_numeric);
  set_int_proc(vc,"char-whitespace?",PROC1,char_ws);
  set_int_proc(vc,"char-upper-case?",PROC1,char_upper);
  set_int_proc(vc,"char-lower-case?",PROC1,char_lower);
  set_int_proc(vc,"char-upcase",PROC1,chartoupper);
  set_int_proc(vc,"char-downcase",PROC1,chartolower);
   
  set_int_proc(vc,"string?",PROC1,stringq);
  set_int_proc(vc,"make-string",PROCOPT,str_make);
  set_int_proc(vc,"string",PROCOPT,string);
  set_int_proc(vc,"string->symbol",PROC1,string_symbol);
  set_int_proc(vc,"string-length",PROC1,string_len);
  set_int_proc(vc,"string-ref",PROC2,string_ref);
  set_int_proc(vc,"string-set!",PROC3,string_set);

  set_int_proc(vc,"string=?",PROC2,string_eq);
  set_int_proc(vc,"string-ci=?",PROC2,string_eqci);
  set_int_proc(vc,"string<?",PROC2,string_less);
  set_int_proc(vc,"string>?",PROC2,string_greater);
  set_int_proc(vc,"string<=?",PROC2,string_lesseq);
  set_int_proc(vc,"string>=?",PROC2,string_greatereq);

  set_int_proc(vc,"string-ci<?",PROC2,string_lessci);
  set_int_proc(vc,"string-ci>?",PROC2,string_greaterci);
  set_int_proc(vc,"string-ci<=?",PROC2,string_lesseqci);
  set_int_proc(vc,"string-ci>=?",PROC2,string_greatereqci);
   
  set_int_proc(vc,"substring",PROC3,string_sub);
  set_int_proc(vc,"string-append",PROC2,string_append);
  set_int_proc(vc,"string->list",PROC1,string_list);
  set_int_proc(vc,"list->string",PROC1,list_string);
  set_int_proc(vc,"string-copy",PROC1,string_copy);
  set_int_proc(vc,"string-fill",PROC2,string_fill);
}

VCSI_OBJECT charq(VCSI_CONTEXT vc, 
		  VCSI_OBJECT obj) {

  if(TYPEP(obj,CHAR))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT stringq(VCSI_CONTEXT vc, 
		    VCSI_OBJECT obj) {
  if(TYPEP(obj,STRING))
    return vc->true;
  return vc->false;
}

int is_char(char* str) {
  char* s = str;

  if((strlen(str) == 3 || strlen(str) == 2) && str[0] == '#' && str[1] == '\\')
    return 1;

  if(str[0] == '#' && str[1] == '\\') {
    s += 2;
    if(!strcasecmp(s,"space") || !strcasecmp(s,"newline"))
      return 1;
  }
  return 0;
}

VCSI_OBJECT make_char(VCSI_CONTEXT vc, 
		      char* str) {

  VCSI_OBJECT tmp = alloc_obj(vc,CHAR);
  char *s = str;

  if(strlen(str) == 3)
    CHAR(tmp) = str[2];
  else if(strlen(str) == 2)
    CHAR(tmp) = ' ';
  else {
    s += 2;
    if(!strcasecmp(s,"space"))
      CHAR(tmp) = ' ';
    else if(!strcasecmp(s,"newline"))
      CHAR(tmp) = '\n';
    else
      return error(vc,"Unknown character name",make_string(vc,str));
  }
  return tmp;
}

VCSI_OBJECT make_char_int(VCSI_CONTEXT vc, 
			  int i) {

  VCSI_OBJECT tmp = alloc_obj(vc,CHAR);
  CHAR(tmp) = i;
  return tmp;
}

int is_string(char* str) {
  if(str[0] =='\"' && str[strlen(str)-1] == '\"')
    return 1;
  return 0;
}

VCSI_OBJECT make_string(VCSI_CONTEXT vc, 
			char* str) {

  VCSI_OBJECT tmp = alloc_obj(vc,STRING);
  int i,j, len, has_q;

  if(str == NULL)
    return NULL;

  len = strlen(str);

  STR(tmp) = (char*)MALLOC(len+1);

  if(STR(tmp) == NULL)
    return error(vc,"string allocation error\n",NULL);

  memset(STR(tmp),0,len+1);

  SLEN(tmp) = 0;
  
  for(i=0,j=0,has_q=0;i<len;i++) {
    if(i==0 && str[i] == '\"') {
      has_q=1;
      continue;
    }
    if(i == (len-1) && has_q && str[i] == '\"')
      continue;
    else if(str[i] == '\\' && str[i+1] == 'n') {
      STR(tmp)[j] = '\n';
      j++;
      i++;
    } else if(str[i] == '\\' && str[i+1] == 'r') {
      STR(tmp)[j] = '\r';
      j++;
      i++;
    } else if(str[i] == '\\' && str[i+1] == 'e') { // ESC
      STR(tmp)[j] = '\e';
      j++;
      i++;
    } else if(str[i] == '\\') {
      STR(tmp)[j] = str[i+1];
      j++;
      i++;
    } else  {
      STR(tmp)[j] = str[i];
      j++;
    }
  }

  SLEN(tmp) = strlen(STR(tmp));
  
  return tmp;
}

VCSI_OBJECT make_raw_string(VCSI_CONTEXT vc, 
			    char* str) {

  VCSI_OBJECT tmp = alloc_obj(vc,STRING);
  int i,j, len, has_q;
  
  if(str == NULL)
    return NULL;

  len = strlen(str);

  STR(tmp) = (char*)MALLOC(len+1);

  if(STR(tmp) == NULL)
    return error(vc,"string allocation error\n",NULL);

  memset(STR(tmp),0,len+1);
  strncpy(STR(tmp),str,len); 
  SLEN(tmp) = strlen(STR(tmp));
  
  return tmp;
}

VCSI_OBJECT chareq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x, 
		   VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char=?");
  check_arg_type(vc,y,CHAR,"char=?");
   
  if(CHAR(x) == CHAR(y))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT charless(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char<?");
  check_arg_type(vc,y,CHAR,"char<?");
   
  if(CHAR(x) < CHAR(y))
    return vc->true;
  return vc->false;
} 

VCSI_OBJECT chargreater(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char>?");
  check_arg_type(vc,y,CHAR,"char>?");
   
  if(CHAR(x) > CHAR(y))
    return vc->true;
  return vc->false;
} 

VCSI_OBJECT charlesseq(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char<=?");
  check_arg_type(vc,y,CHAR,"char<=?");
      
  if(CHAR(x) <= CHAR(y))
    return vc->true;
  return vc->false;
} 

VCSI_OBJECT chargreatereq(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char>=?");
  check_arg_type(vc,y,CHAR,"char>=?");
   
  if(CHAR(x) == CHAR(y))
    return vc->true;
  return vc->false;
} 


VCSI_OBJECT chareqci(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char-ci=?");
  check_arg_type(vc,y,CHAR,"char-ci=?");
   
  if(toupper((int)CHAR(x)) == toupper((int)CHAR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT charlessci(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char-ci<?");
  check_arg_type(vc,y,CHAR,"char-ci<?");
   
  if(toupper((int)CHAR(x)) < toupper((int)CHAR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT chargreaterci(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char-ci>?");
  check_arg_type(vc,y,CHAR,"char-ci>?");
   
  if(toupper((int)CHAR(x)) > toupper((int)CHAR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT charlesseqci(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x, 
			 VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char-ci<=?");
  check_arg_type(vc,y,CHAR,"char-ci<=?");
   
  if(toupper((int)CHAR(x)) <= toupper((int)CHAR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT chargreatereqci(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y) {

  check_arg_type(vc,x,CHAR,"char-ci>=?");
  check_arg_type(vc,y,CHAR,"char-ci>=?");
   
  if(toupper((int)CHAR(x)) == toupper((int)CHAR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT char_alpha(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  if(!TYPEP(x,CHAR))
    return error(vc,"object passed to char-alphabetic? is the wrong type",x);
  if(isalpha((int)CHAR(x)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT char_numeric(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x) {

  check_arg_type(vc,x,CHAR,"char-numeric?");

  if(isdigit((int)CHAR(x)))
    return vc->true;
  return vc->false;
}  

VCSI_OBJECT char_ws(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  check_arg_type(vc,x,CHAR,"char-whitespace?");

  if(isspace((int)CHAR(x)))
    return vc->true;
  return vc->false;
}  

VCSI_OBJECT char_upper(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  check_arg_type(vc,x,CHAR,"char-upper-case?");

  if(isupper((int)CHAR(x)))
    return vc->true;
  return vc->false;
}  

VCSI_OBJECT char_lower(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  check_arg_type(vc,x,CHAR,"char-lower-case?");

  if(islower((int)CHAR(x)))
    return vc->true;
  return vc->false;
}  

VCSI_OBJECT char_int(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  check_arg_type(vc,x,CHAR,"char->integer");

  return make_long(vc,(long)CHAR(x));
}

VCSI_OBJECT int_char(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  VCSI_OBJECT tmp;

  check_arg_type(vc,x,LNGNUM,"integer->char");

  tmp = alloc_obj(vc,CHAR);
  CHAR(tmp) = (char)LNGN(x);
  return tmp;
}

VCSI_OBJECT chartoupper(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  VCSI_OBJECT tmp;

  check_arg_type(vc,x,CHAR,"char-upcase");

  tmp = alloc_obj(vc,CHAR);
  CHAR(tmp) = toupper((int)CHAR(x));
  return tmp;
}

VCSI_OBJECT chartolower(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  VCSI_OBJECT tmp;

  check_arg_type(vc,x,CHAR,"char-downcase");

  tmp = alloc_obj(vc,CHAR);
  CHAR(tmp) = tolower((int)CHAR(x));
  return tmp;
}

VCSI_OBJECT str_make(VCSI_CONTEXT vc, 
		     VCSI_OBJECT args, 
		     int length) {

  VCSI_OBJECT tmp, tmp2;
  char* s;

  if(length == 1) {
    tmp = car(vc,args);

    check_arg_type(vc,tmp,LNGNUM,"make-string");

    s = (char*)MALLOC(LNGN(tmp)+1);

    if(!s)
      return error(vc,"string allocation error",NULL);
      
    memset(s,0,LNGN(tmp)+1);
    memset(s,32,LNGN(tmp));

    tmp2 = make_string(vc,s);

    FREE(s);
    return tmp2;
  } else if(length == 2) {
    tmp = car(vc,args);
    tmp2 = cadr(vc,args);

    check_arg_type(vc,tmp,LNGNUM,"make-string");
    check_arg_type(vc,tmp2,CHAR,"make-string");
	
    s = (char*)MALLOC(LNGN(tmp)+1);

    if(!s)
      return error(vc,"string allocation error",NULL);
      
    memset(s,0,LNGN(tmp)+1);
    memset(s,CHAR(tmp2),LNGN(tmp));
    tmp2 = make_string(vc,s);
    FREE(s);
    return tmp2;
  }
  return error(vc,"invalid number arguments to make-string",args);
}

VCSI_OBJECT string(VCSI_CONTEXT vc, 
		   VCSI_OBJECT args, 
		   int length) {

  VCSI_OBJECT tmp, nchar;
  char* s = NULL;
  int i;
  for(i=1,tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp),i++) {
    nchar = car(vc,tmp);
    check_arg_type(vc,nchar,CHAR,"string");

    s = realloc(s,i); /* What is this doing?? */
    s[i-1] = CHAR(nchar);
  }
  s[i-1] = 0;
  tmp = make_string(vc,s);
  FREE(s);
  return tmp;
}

VCSI_OBJECT string_symbol(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x) {

  check_arg_type(vc,x,STRING,"string->symbol");

  return make_symbol(vc,STR(x));
}

VCSI_OBJECT string_len(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  check_arg_type(vc,x,STRING,"string-length");

  return make_long(vc,SLEN(x));
}

VCSI_OBJECT string_ref(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-ref");
  check_arg_type(vc,y,LNGNUM,"string-ref");

  if(LNGN(y) > SLEN(x) || LNGN(y) < 0)
    return error(vc,"subscript out of range in string-ref",y);
  return make_char_int(vc,STR(x)[LNGN(y)]);
}

VCSI_OBJECT string_set(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y, 
		       VCSI_OBJECT z) {

  check_arg_type(vc,x,STRING,"string-set!");
  check_arg_type(vc,y,LNGNUM,"string-set!");
  check_arg_type(vc,z,CHAR,"string-set!");

  if(LNGN(y) > SLEN(x) || LNGN(y) < 0)
    return error(vc,"subscript out of range in string-set!",y);

  STR(x)[LNGN(y)] = CHAR(z);
  return vc->novalue;
}

VCSI_OBJECT string_eq(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x, 
		      VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-=?");
  check_arg_type(vc,y,STRING,"string-=?");

  if(!strcmp(STR(x),STR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_eqci(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-ci=?");
  check_arg_type(vc,y,STRING,"string-ci=?");

  if(!strcasecmp(STR(x),STR(y)))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_less(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string<?");
  check_arg_type(vc,y,STRING,"string<?");

  if(strcmp(STR(x),STR(y)) < 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_greater(VCSI_CONTEXT vc, 
			   VCSI_OBJECT x, 
			   VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string>?");
  check_arg_type(vc,y,STRING,"string>?");

  if(!strcmp(STR(x),STR(y)) > 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_lesseq(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string<=?");
  check_arg_type(vc,y,STRING,"string<=?");

  if(strcmp(STR(x),STR(y)) <= 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_greatereq(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x, 
			     VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string>=?");
  check_arg_type(vc,y,STRING,"string>=?");

  if(!strcmp(STR(x),STR(y)) >= 0)
    return vc->true;
  return vc->false;
}



VCSI_OBJECT string_lessci(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-ci<?");
  check_arg_type(vc,y,STRING,"string-ci<?");

  if(strcasecmp(STR(x),STR(y)) < 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_greaterci(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x, 
			     VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-ci>?");
  check_arg_type(vc,y,STRING,"string-ci>?");

  if(!strcasecmp(STR(x),STR(y)) > 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_lesseqci(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-ci<=?");
  check_arg_type(vc,y,STRING,"string-ci<=?");

  if(strcasecmp(STR(x),STR(y)) <= 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_greatereqci(VCSI_CONTEXT vc, 
			       VCSI_OBJECT x, 
			       VCSI_OBJECT y) {

  check_arg_type(vc,x,STRING,"string-ci>=?");
  check_arg_type(vc,y,STRING,"string-ci>=?");

  if(!strcasecmp(STR(x),STR(y)) >= 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT string_sub(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x,
		       VCSI_OBJECT y, 
		       VCSI_OBJECT z) {

  VCSI_OBJECT tmp;
  char* s = NULL;
  int i,len;

  check_arg_type(vc,x,STRING,"substring");
  check_arg_type(vc,y,LNGNUM,"substring");
  check_arg_type(vc,z,LNGNUM,"substring");

  if(LNGN(z) <  LNGN(y))
    return error(vc,"start > end in substring",cons(vc,y,z));
  if(LNGN(z) > SLEN(x) || LNGN(z) < 0 || LNGN(y) < 0)
    return error(vc,"subscript out of range in substring",y);

  len = (LNGN(z)-LNGN(y)) +1;

  s = (char*)MALLOC(len);

  if(!s)
    return error(vc,"string allocation error",NULL);

  memset(s,0,len);

  for(i=LNGN(y);i<LNGN(z);i++) {
    s[i-LNGN(y)] = STR(x)[i];
  }
  s[i-LNGN(y)] = 0;

  tmp = make_string(vc,s);

  FREE(s);
  return tmp;
}

VCSI_OBJECT string_sub_i(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x, 
			 int y, 
			 int z) { 

  VCSI_OBJECT tmp; 
  char* s = NULL; 
  int i, len;

  check_arg_type(vc,x,STRING,"substring");

  len = (z-y)+1;

  s = (char*)MALLOC(len);
  if(!s)
    return error(vc,"string allocation error",NULL);

  memset(s,0,len);

  for(i=y;i<z;i++) { 
    s[i-y] = STR(x)[i]; 
  } 
  s[i-y] = 0; 
  tmp = make_string(vc,s); 
  FREE(s); 
  return tmp; 
} 

VCSI_OBJECT string_append(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  VCSI_OBJECT tmp;
  char* s = NULL;
  int len;

  check_arg_type(vc,x,STRING,"string-append");
  check_arg_type(vc,y,STRING,"string-append");

  len = SLEN(x) + SLEN(y) + 1;

  s = (char*)MALLOC(len);

  if(!s)
    return error(vc,"string allocation error",NULL);

  memset(s,0,len);

  strncpy(s,STR(x),SLEN(x));
  strncat(s,STR(y),SLEN(y));

  tmp = make_string(vc,s);

  FREE(s);

  return tmp;
}

VCSI_OBJECT string_list(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  VCSI_OBJECT tmp = NULL;
  int i;

  check_arg_type(vc,x,STRING,"string->list");

  for(i=0;i<SLEN(x);i++)
    tmp=cons(vc,make_char_int(vc,STR(x)[i]),tmp);

  return vcsi_list_reverse(vc,tmp);
}

VCSI_OBJECT list_string(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  return string(vc,x,get_length(x));
}

VCSI_OBJECT string_copy(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  check_arg_type(vc,x,STRING,"string-copy");

  return make_string(vc,STR(x));
}

VCSI_OBJECT string_fill(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {

  int i;

  check_arg_type(vc,x,STRING,"string-fill");
  check_arg_type(vc,y,CHAR,"string-fill");

  for(i=0;i<SLEN(x);i++)
    STR(x)[i] = CHAR(y);

  return vc->novalue;
}

/* extensions to string handling */
void str_ext_init(VCSI_CONTEXT vc) {
  set_int_proc(vc,"string-downcase",PROC1,string_downcase);
  set_int_proc(vc,"string-upcase",PROC1,string_upcase);
  set_int_proc(vc,"string-eval",PROC1,string_eval);
  set_int_proc(vc,"string-split",PROC2,string_split);
  set_int_proc(vc,"string-join",PROC2,string_join);
  set_int_proc(vc,"string-tokenize",PROC2,string_tokenize);
}

VCSI_OBJECT string_downcase(VCSI_CONTEXT vc,
			    VCSI_OBJECT x) {

  VCSI_OBJECT y;
  int i;

  check_arg_type(vc,x,STRING,"string-downcase");

  y = string_copy(vc,x);
  for(i=0;i<SLEN(y);i++)
    STR(y)[i] = tolower(STR(y)[i]);
  
  return y;
}

VCSI_OBJECT string_upcase(VCSI_CONTEXT vc,
			  VCSI_OBJECT x) {

  VCSI_OBJECT y;
  int i;
  
  check_arg_type(vc,x,STRING,"string-upcase");

  y = string_copy(vc,x);
  for(i=0;i<SLEN(y);i++)
    STR(y)[i] = toupper(STR(y)[i]);
  
  return y;
}

VCSI_OBJECT string_eval(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {
  VCSI_CONTEXT vn;
  VCSI_OBJECT tmp;
  VCSI_OBJECT s2;
  char* str;
  
  check_arg_type(vc,x,STRING,"string-eval");

  /* Clone the context, and setup the stack pointers */
  vn = vcsi_clone_context(vc);
  DYNSTCKE(vc->root_wind) = &s2;
  
  /* Evaluate the string */
  if(!sigsetjmp(DYNERR(vn->root_wind),1)) {
    DYNSTCK(vn->root_wind) = DYNSTCK(vc->root_wind);
    DYNSTCKE(vn->root_wind) = &tmp;
    tmp = parse_text(vn,STR(x));
    tmp = eval(vn,tmp,DYNROOT(vn->root_wind));
    str = print_obj(vn,vn->result,tmp,1);
  } else 
    str = print_obj(vn,vn->result,vn->errobj,1);
 
  tmp = make_string(vc,str);
  
  vcsi_context_free(vn);

  return tmp;
}

VCSI_OBJECT string_split(VCSI_CONTEXT vc,
			 VCSI_OBJECT pat,
			 VCSI_OBJECT str) {

  VCSI_OBJECT list = NULL;
  char *tmp, *ln, *lstr=NULL;

  check_arg_type(vc,str,STRING,"string-split");
  if(!TYPEP(pat,STRING) && !TYPEP(pat,CHAR))
    error(vc,"object passed to string-split is of the wrong type",pat);
  
  lstr = strdup(STR(str));
  tmp = lstr;

  if(TYPEP(pat,STRING)) {
    while((ln=strstr(tmp,STR(pat)))) {
      *ln = '\0';
      if(ln != tmp)
	list = cons(vc,make_string(vc,tmp),list);
      tmp = ln+SLEN(pat);
    }
    if(strlen(tmp) && tmp!=lstr)
	list = cons(vc,make_string(vc,tmp),list);
    
  } else {
    while((ln=strchr(tmp,CHAR(pat)))) {
      *ln = '\0';
      if(ln != tmp)
	list = cons(vc,make_string(vc,tmp),list);
      tmp = ln+1;
    }
    if(strlen(tmp) && tmp!=lstr)
	list = cons(vc,make_string(vc,tmp),list);    
  }

  if(lstr)
    FREE(lstr);

  return vcsi_list_reverse(vc,list);
}

VCSI_OBJECT string_join(VCSI_CONTEXT vc,
			VCSI_OBJECT joiner,
			VCSI_OBJECT slist) {

  VCSI_OBJECT tmp,str;
  char tmpbuf[4096];

  check_arg_type(vc,slist,CONS,"string-join");
  if(!TYPEP(joiner,STRING) && !TYPEP(joiner,CHAR))
    error(vc,"object passed to string-join is of the wrong type",joiner);
  
  memset(tmpbuf,0,4096);

  for(tmp=slist;TYPEP(tmp,CONS);tmp=cdr(vc,tmp)) {
    str = car(vc,tmp);
    check_arg_type(vc,str,STRING,"string-join");

    if(strlen(tmpbuf)) {
      if(TYPEP(joiner,STRING))
	strncat(tmpbuf,STR(joiner),4095);
      else
	tmpbuf[strlen(tmpbuf)]=CHAR(joiner);
    }

    strncat(tmpbuf,STR(str),4095);
  }

  return make_string(vc,tmpbuf);
}

VCSI_OBJECT string_tokenize(VCSI_CONTEXT vc,
			    VCSI_OBJECT tokens,
			    VCSI_OBJECT str) {

  VCSI_OBJECT list=NULL;
  char *tmp, *lstr=NULL;

  check_arg_type(vc,str,STRING,"string-tokenize");  
  check_arg_type(vc,tokens,STRING,"string-tokenize");

  lstr = strdup(STR(str));

  while((tmp=strsep(&lstr,STR(tokens)))) {
    if(strlen(tmp))
       list = cons(vc,make_string(vc,tmp),list);
  }
  if(lstr)
    FREE(lstr);

  return vcsi_list_reverse(vc,list);
}
/* (string-tokenize "this. is a test. of stuff..." " .") */


/* Random string functions we need if they aren't defined */
#ifndef HAVE_STRSEP
char *strsep(char **stringp, 
	     const char *delim) {

  int i, c, ilen, clen;
  char* tmp;

  if(!(*stringp))
    return NULL;

  ilen = strlen(*stringp);
  clen = strlen(delim);

  for(i=0;i<ilen;i++) {
    for(c=0;c<clen;c++) {
      if(*(*stringp + i) == delim[c]) {
	*(*stringp + i) = '\0';
	tmp = *stringp;
	*stringp=*stringp+i+1;
	return tmp;
      }
    }
  }	
  tmp = *stringp;
  *stringp=NULL;
  return tmp;
}

#endif

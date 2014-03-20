#include "vcsi.h"
void num_init(VCSI_CONTEXT vc) {

  set_int_proc(vc,"number?",PROC1,numberq);
  set_int_proc(vc,"real?",PROC1,realq);
  set_int_proc(vc,"rational?",PROC1,ratq);
  set_int_proc(vc,"integer?",PROC1,intq);
  set_int_proc(vc,"exact?",PROC1,exactq);
  set_int_proc(vc,"inexact?",PROC1,inexactq);
  set_int_proc(vc,"zero?",PROC1,zeroq);
  set_int_proc(vc,"positive?",PROC1,posq);
  set_int_proc(vc,"negative?",PROC1,negq);
  set_int_proc(vc,"odd?",PROC1,oddq);
  set_int_proc(vc,"even?",PROC1,evenq);
   
  set_int_proc(vc,"*",PROCOPT,multiply);
  set_int_proc(vc,"/",PROCOPT,divide);
  set_int_proc(vc,"+",PROCOPT,add);
  set_int_proc(vc,"-",PROCOPT,subtract);
  set_int_proc(vc,"=",PROC2,eqv);
  set_int_proc(vc,"<",PROC2,lessthan);
  set_int_proc(vc,">",PROC2,greaterthan);
  set_int_proc(vc,"<=",PROC2,lesseq);
  set_int_proc(vc,">=",PROC2,greatereq);

  set_int_proc(vc,"max",PROCOPT,num_max);
  set_int_proc(vc,"min",PROCOPT,num_min);
  set_int_proc(vc,"abs",PROC1,num_abs);
  set_int_proc(vc,"quotient",PROC2,num_quotient);
  set_int_proc(vc,"remainder",PROC2,num_remainder);
  set_int_proc(vc,"modulo",PROC2,num_modulo);

  set_int_proc(vc,"gcd",PROCOPT,num_gcd);
  set_int_proc(vc,"lcm",PROCOPT,num_lcm);

  set_int_proc(vc,"numerator",PROC1,num_numer);
  set_int_proc(vc,"denominator",PROC1,num_denom);

  set_int_proc(vc,"floor",PROC1,num_floor);
  set_int_proc(vc,"ceiling",PROC1,num_ceil);
  set_int_proc(vc,"truncate",PROC1,num_trunc);
  set_int_proc(vc,"round",PROC1,num_round);
  set_int_proc(vc,"rationalize",PROC2,num_rationalize);
   
  set_int_proc(vc,"exp",PROC1,num_exp);
  set_int_proc(vc,"log",PROC1,num_log);
  set_int_proc(vc,"sin",PROC1,num_sin);
  set_int_proc(vc,"cos",PROC1,num_cos);
  set_int_proc(vc,"tan",PROC1,num_tan);
  set_int_proc(vc,"asin",PROC1,num_asin);
  set_int_proc(vc,"acos",PROC1,num_acos);
  set_int_proc(vc,"atan",PROCOPT,num_atan);
  set_int_proc(vc,"sqrt",PROC1,num_sqrt);
  set_int_proc(vc,"expt",PROC2,num_expt);

  set_int_proc(vc,"exact->inexact",PROC1,exact_inexact);
  set_int_proc(vc,"inexact->exact",PROC1,inexact_exact);

  set_int_proc(vc,"number->string",PROCOPT,num_string);
  set_int_proc(vc,"string->number",PROCOPT,string_num);
}

int is_num(char* str) {
  int isnum = 0;
  int isfloat = 0;
  int israt = 0;
  int radix = 0;
  int i = 0;
   
  if(str[i] == '#') {
    if(str[i+1] == 'i' || str[i+1] == 'e' || str[i+1] == 'd')
      i += 2;
    else if(str[i+1] == 'b' || str[i+1] == 'o' || str[i+1] == 'x')
      radix = 1;
    else
      return 0;
  }
   
  if(str[i] == '#') {
    if(str[i+1] == 'i' || str[i+1] == 'e' || str[i+1] == 'd')
      i += 2;
    else if(str[i+1] == 'b' || str[i+1] == 'o' || str[i+1] == 'x')
      radix = 1;
    else
      return 0;
  }
   
  if(radix)
    return 4;
   
  if(str[i] == '-' && strlen(str) != 1+i) {
    i++;
  }

  while(i < strlen(str)) {

    if(isdigit((int)str[i])) {
      isnum = 1;
      i++;
    } else if(str[i] == 'e' && i != strlen(str)-1 && isnum) {
      isfloat=1;
      isnum = 1;
      i++;
    } else if(i > 0 && (str[i] == '-' || str[i] == '+') && str[i-1] == 'e'  
	      && isnum && i != strlen(str)-1) {
      isfloat=1;
      isnum = 1;
      i++;
    } else
      break;
  }
  
  if(i == strlen(str) && isnum) {
    if(isfloat)
      return 2;
    return 1;
  }

  else if(str[i] == '/' && i != 0) {
    israt = 1;
    i++;
    while(i < strlen(str)) {
      if(isdigit((int)str[i])) {
	isnum = 1;
	i++;
      }
      else
	break;
    }
    if(i == strlen(str) && isnum)
      return 3;
  } else if(str[i] == '.') {
    isfloat = 1;
    i++;
    while(i < strlen(str)) {
      if(isdigit((int)str[i])) {
	isnum = 1;
	i++;
      } else if(str[i] == 'e' && i != strlen(str)-1) {
	isnum = 1;
	i++;
      } else if((str[i] == '-' || str[i] == '+') && str[i-1] == 'e'
		&& i != strlen(str)-1) {
	isnum = 1;
	i++;
      }
      else 	       
	break;
    }

    if(i == strlen(str) && isnum)
      return 2;
  }
  return 0;
}

VCSI_OBJECT check_exact(VCSI_CONTEXT vc, 
			char* str, 
			int* e) {
  int i;
  int exact = -1;
  int radix = -1;
  for(i=0;i<strlen(str);i++)
    {
      if(isdigit((int)str[i]) || str[i] == '-')
	break;
      if(str[i] == '#')
	{
	  if(str[i+1] == 'e' && exact == -1)
	    exact = 1;
	  else if(str[i+1] == 'i' && exact == -1)
	    exact = 0;
	  else if((str[i+1] == 'b' || str[i+1] == 'd' || str[i+1] == 'o'
		   || str[i+1] == 'x') && radix == -1)
	    radix = 1;
	  else
	    error(vc,"invalid number type",make_string(vc,str));
	     
	  str[i] = ' ';
	  str[i+1] = ' ';
	}
    }
  if(exact != -1)
    *e = exact;
  return NULL;
}
   
VCSI_OBJECT make_num(VCSI_CONTEXT vc, 
		     char* str) {
  int i, c;
  int exact = 1;
  int radix = 0;
  long x = 0;

  for(i=0;i<strlen(str);i++)
    {
      if(str[i] == '#')
	{
	  if(str[i+1] == 'b' && !radix)
	    radix = 1;
	  else if(str[i+1] == 'o' && !radix)
	    radix = 2;
	  else if(str[i+1] == 'x' && !radix)
	    radix = 3;
	  else 
	    radix = 0;
	  if(radix)
	    break;
	}
    }
  
  check_exact(vc,str,&exact);

  if(radix == 1)
    {
      for(c=0,i=strlen(str) -1;i >= 0;i--,c++)
	{
	  if(str[i] == '1')
	    x += pow(2,c);
	  else if(str[i] == '0')
	    continue;
	  else if(str[i] == ' ')
	    break;
	  else
	    error(vc,"invalid number",make_string(vc,str));
	}
    }
  else if(radix == 2)
    sscanf(str,"%o",(unsigned int*)&x);
  else if(radix == 3)
    sscanf(str,"%x",(unsigned int*)&x);
  else
    error(vc,"invalid number",make_string(vc,str));
   
  if(exact)
    return make_long(vc,x);
  return make_float(vc,x);
   
}

VCSI_OBJECT make_long(VCSI_CONTEXT vc, 
		      long num) {

  VCSI_OBJECT tmp = alloc_obj(vc,LNGNUM);
  LNGN(tmp) = num;
  return tmp;
}

VCSI_OBJECT make_long_str(VCSI_CONTEXT vc, 
			  char* str) {
  float z;
  int exact = 1;
   
  check_exact(vc,str,&exact);
   
  if(exact)
    z = atol(str);
  else
    z = atof(str);
  if(z == (long)z && exact)
    return make_long(vc,z);
  return make_float(vc,z);
}

VCSI_OBJECT make_float(VCSI_CONTEXT vc, 
		       float num) {

  VCSI_OBJECT tmp = alloc_obj(vc,FLTNUM);
  FLTN(tmp) = num;
  return tmp;
}

VCSI_OBJECT make_float_str(VCSI_CONTEXT vc, 
			   char* str) {
  float z;
  int exact = 0;
   
  check_exact(vc,str,&exact);
   
  z = atof(str);
  if(exact)
    return inexact_exact(vc,make_float(vc,z));
  return make_float(vc,z);
}

VCSI_OBJECT make_rat_int(VCSI_CONTEXT vc, 
			 long numer, 
			 long denom) {

  VCSI_OBJECT tmp = alloc_obj(vc,RATNUM);
  int i;
      
  if(denom == 0)
    error(vc,"attempted divide by zero",make_long(vc,denom));

  if(numer == 0)
    return make_long(vc,0);
   
  i = compute_gcd(numer,denom);

  if(i > 1) {
    numer = numer / i;
    denom = denom / i;
  }
  
  if(numer == denom) {
    tmp->type = LNGNUM;
    LNGN(tmp) = 1;
  } else if(denom == 1) {
    tmp->type = LNGNUM;
    LNGN(tmp) = numer;
  } else {
    RNUM(tmp) = numer;
    RDENOM(tmp) = denom;
  }
  return tmp;
}

VCSI_OBJECT make_rat(VCSI_CONTEXT vc, 
		     char* str) {
  char* s;
  long n, d;
   
  int exact = 1;
   
  check_exact(vc,str,&exact);
   
  str = strdup(str);

  s = strsep(&str,"/");
  n = atol(s);
  s = strsep(&str,"/");
  d = atol(s);

  if(str)
    FREE(str);
   
  if(exact)
    return make_rat_int(vc,n,d);
  return exact_inexact(vc,make_rat_int(vc,n,d));
}

VCSI_OBJECT numberq(VCSI_CONTEXT vc, 
		    VCSI_OBJECT obj) {

  if(TYPEP(obj,LNGNUM) || TYPEP(obj,FLTNUM) || TYPEP(obj,RATNUM))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT realq(VCSI_CONTEXT vc, 
		  VCSI_OBJECT obj) {

  if(TYPEP(obj,LNGNUM) || TYPEP(obj,FLTNUM) || TYPEP(obj,RATNUM))
    return vc->true;

  return vc->false;
}

VCSI_OBJECT ratq(VCSI_CONTEXT vc, 
		 VCSI_OBJECT obj) {

  if(TYPEP(obj,RATNUM))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT intq(VCSI_CONTEXT vc, 
		 VCSI_OBJECT obj) {

  if(TYPEP(obj,LNGNUM))
    return vc->true;
  else if(TYPEP(obj,FLTNUM) && FLTN(obj) == (long)FLTN(obj))
    return vc->true;

  return vc->false;
}  

VCSI_OBJECT get_nums(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y, 
		     float *i, 
		     float *j, 
		     float* l, 
		     float* m) {

  if(TYPEP(x,RATNUM) && TYPEP(y,LNGNUM)) {
    *i = RNUM(x);
    *l = RDENOM(x);
	
    *j = LNGN(y);
    *m = 1;
    return vc->true;
  }
  else if(TYPEP(x,LNGNUM) && TYPEP(y,RATNUM)) {
    *i = LNGN(x);
    *l = 1;
	
    *j = RNUM(y);
    *m = RDENOM(y);
    return vc->true;
  }   
  else if(TYPEP(x,RATNUM) && TYPEP(y,RATNUM)) {
    *i = RNUM(x);
    *l = RDENOM(x);
    *j = RNUM(y);
    *m = RDENOM(y);
    return vc->true;
  }
  if(TYPEP(x,LNGNUM))
    *i = (float)LNGN(x);
  else if(TYPEP(x,FLTNUM))
    *i = FLTN(x);
  else if(TYPEP(x,RATNUM))
    *i = (float)RNUM(x)/(float)RDENOM(x);
  else
    error(vc,"object not a number", x);
  *l = 1;
   
  if(TYPEP(y,LNGNUM))
    *j = (float)LNGN(y);
  else if(TYPEP(y,FLTNUM))
    *j = FLTN(y);
  else if(TYPEP(y,RATNUM))
    *j = (float)RNUM(y)/(float)RDENOM(y);
  else
    error(vc,"object not a number", y);
  *m = 1;
  if(TYPEP(x,LNGNUM) && TYPEP(y,LNGNUM))
    return vc->true;
  return NULL;
}

VCSI_OBJECT multiply(VCSI_CONTEXT vc, 
		     VCSI_OBJECT args, 
		     int length) {

  VCSI_OBJECT res, tmp, num;
  float i, j, k, l, c, d;

  res = make_long(vc,1);
  for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    num = car(vc,tmp);
	
    if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
      error(vc,"object passed to * is the wrong type",num);
    
    num = get_nums(vc,res,num,&i,&j,&c,&d);

    if(num) {
      k = i * j;
      l = c * d;
      res = make_rat_int(vc,k,l);
    }
    else {
      k = i * j;
      res = make_float(vc,k);
    }
  }
  return res;
}

VCSI_OBJECT divide(VCSI_CONTEXT vc, 
		   VCSI_OBJECT args, 
		   int length) {

  VCSI_OBJECT res=NULL, tmp, num;
  float i, j, k, l, c, d;

  if(length == 0)
    error(vc,"invalid number of arguments to /",args);
  else if(length > 1) {
    res = car(vc,args);
	
    for(tmp=CDR(args);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
      num = car(vc,tmp);
	     
      if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
	error(vc,"object passed to / is the wrong type",num);
      
      num = get_nums(vc,res,num,&i,&j,&c,&d);

      if(j == 0)
	error(vc,"attempted divide by zero",num);

      if(num) {
	k = i * d;
	l = j * c;
	res = make_rat_int(vc,k,l);
      } else {
	k = i / j;
	res = make_float(vc,k);
      }
    }
  } else {
    res = make_long(vc,1);

    for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
      num = car(vc,tmp);
             
      if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
	error(vc,"object passed to / is the wrong type",num);

      num = get_nums(vc,res,num,&i,&j,&c,&d);

      if(j == 0)
	error(vc,"attempted divide by zero",num);

      if(num) {
	k = i * d;
	l = j * c;
	res = make_rat_int(vc,k,l);
      } else {
	k = i / j;
	res = make_float(vc,k);
      }
    }
  }
  return res;
}

VCSI_OBJECT add(VCSI_CONTEXT vc, 
		VCSI_OBJECT args, 
		int length) {

  VCSI_OBJECT res, tmp, num;
  float i, j, k, l, c, d;

  res = make_long(vc,0);
  for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    num = car(vc,tmp);
        
    if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
      error(vc,"object passed to + is the wrong type",num);

    num = get_nums(vc,res,num,&i,&j,&c,&d);

    if(num) {
      k = (i * d) + (j * c);
      l = c * d;
      res = make_rat_int(vc,k,l);
    } else {
      k = i + j;
      res = make_float(vc,k);
    }
  }
  return res;
}

VCSI_OBJECT subtract(VCSI_CONTEXT vc, 
		     VCSI_OBJECT args, 
		     int length) {

  VCSI_OBJECT res=NULL, tmp, num;
  float i, j, k, l, c, d;

  if(length == 0)
    error(vc,"invalid number of arguments to -",args);

  else if(length > 1) {
    res = car(vc,args);
	
    for(tmp=CDR(args);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
      num = car(vc,tmp);
             
      if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
	error(vc,"Object passed to - is the wrong type",num);

      num = get_nums(vc,res,num,&i,&j,&c,&d);

      if(num) {		  
	k = (i * d) - (j * c);
	l = c * d;
	res = make_rat_int(vc,k,l);
      } else {
	k = i - j;
	res = make_float(vc,k);
      }
    }
  } else {
    res = make_long(vc,0);

    for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
      num = car(vc,tmp);
             
      if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
	error(vc,"object passed to - is the wrong type",num);
      
      num = get_nums(vc,res,num,&i,&j,&c,&d);

      if(num) {		  
	k = (i * d) - (j * c);
	l = c * d;
	res = make_rat_int(vc,k,l);
      } else {
	k = i - j;
	res = make_float(vc,k);
      }
    }
  }
  return res;
}

VCSI_OBJECT lessthan(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y) {

  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
  if(TYPEP(y,RATNUM))
    y = make_float(vc,(float)RNUM(y)/(float)RDENOM(y));
   
  if(TYPEP(x,LNGNUM) && TYPEP(y,LNGNUM))
    if(LNGN(x) < LNGN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,FLTNUM))
    if(FLTN(x) < FLTN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,LNGNUM))
    if(FLTN(x) < LNGN(y))
      return vc->true;
  if(TYPEP(x,LNGNUM) && TYPEP(y,FLTNUM))
    if(LNGN(x) < FLTN(y))
      return vc->true;
  if(numberq(vc,x) == NULL)
    error(vc,"object passed to < is the wrong type",x);
  if(numberq(vc,y) == NULL)
    error(vc,"object passed to < is the wrong type",y);
  return vc->false;
}

VCSI_OBJECT greaterthan(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y) {

  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
  if(TYPEP(y,RATNUM))
    y = make_float(vc,(float)RNUM(y)/(float)RDENOM(y));
   
  if(TYPEP(x,LNGNUM) && TYPEP(y,LNGNUM))
    if(LNGN(x) > LNGN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,FLTNUM))
    if(FLTN(x) > FLTN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,LNGNUM))
    if(FLTN(x) > LNGN(y))
      return vc->true;
  if(TYPEP(x,LNGNUM) && TYPEP(y,FLTNUM))
    if(LNGN(x) > FLTN(y))
      return vc->true;
  if(numberq(vc,x) == NULL)
    error(vc,"object passed to > is the wrong type",x);
  if(numberq(vc,y) == NULL)
    error(vc,"object passed to > is the wrong type",y);
  return vc->false;
}

VCSI_OBJECT lesseq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x, 
		   VCSI_OBJECT y) {

  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
  if(TYPEP(y,RATNUM))
    y = make_float(vc,(float)RNUM(y)/(float)RDENOM(y));
   
  if(TYPEP(x,LNGNUM) && TYPEP(y,LNGNUM))
    if(LNGN(x) <= LNGN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,FLTNUM))
    if(FLTN(x) <= FLTN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,LNGNUM))
    if(FLTN(x) <= LNGN(y))
      return vc->true;
  if(TYPEP(x,LNGNUM) && TYPEP(y,FLTNUM))
    if(LNGN(x) <= FLTN(y))
      return vc->true;
  if(numberq(vc,x) == NULL)
    error(vc,"object passed to <= is the wrong type",x);
  if(numberq(vc,y) == NULL)
    error(vc,"object passed to <= is the wrong type",y);
  return vc->false;
}

VCSI_OBJECT greatereq(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x, 
		      VCSI_OBJECT y) { 

  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
  if(TYPEP(y,RATNUM))
    y = make_float(vc,(float)RNUM(y)/(float)RDENOM(y));
   
  if(TYPEP(x,LNGNUM) && TYPEP(y,LNGNUM))
    if(LNGN(x) >= LNGN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,FLTNUM))
    if(FLTN(x) >= FLTN(y))
      return vc->true;
  if(TYPEP(x,FLTNUM) && TYPEP(y,LNGNUM))
    if(FLTN(x) >= LNGN(y))
      return vc->true;
  if(TYPEP(x,LNGNUM) && TYPEP(y,FLTNUM))
    if(LNGN(x) >= FLTN(y))
      return vc->true;
  if(numberq(vc,x) == NULL)
    error(vc,"object passed to >= is the wrong type",x);
  if(numberq(vc,y) == NULL)
    error(vc,"object passed to >= is the wrong type",y);
  return vc->false;
}

VCSI_OBJECT exactq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to exact? is the wrong type",x);
  if(TYPEP(x,LNGNUM) || TYPEP(x,RATNUM))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT inexactq(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to inexact? is the wrong type",x);
  if(TYPEP(x,FLTNUM))
    return vc->true;
  return vc->false;
}


VCSI_OBJECT zeroq(VCSI_CONTEXT vc, 
		  VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to zero? is the wrong type",x);
   
  if(TYPEP(x,LNGNUM) && LNGN(x) == 0)
    return vc->true;
  if(TYPEP(x,FLTNUM) && FLTN(x) == 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT posq(VCSI_CONTEXT vc, 
		 VCSI_OBJECT x) {
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to positive? is the wrong type",x);
   
  if(TYPEP(x,LNGNUM) && LNGN(x) >= 0)
    return vc->true;
  if(TYPEP(x,FLTNUM) && FLTN(x) >= 0)
    return vc->true;
  if(TYPEP(x,RATNUM) && RNUM(x) >= 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT negq(VCSI_CONTEXT vc, 
		 VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to negative? is the wrong type",x);
   
  if(TYPEP(x,LNGNUM) && LNGN(x) < 0)
    return vc->true;
  if(TYPEP(x,FLTNUM) && FLTN(x) < 0)
    return vc->true;
  if(TYPEP(x,RATNUM) && RNUM(x) < 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT oddq(VCSI_CONTEXT vc, 
		 VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM))
    error(vc,"object passed to odd? is the wrong type",x);
   
  if(TYPEP(x,LNGNUM)) {
    if(LNGN(x) % 2 == 1)
      return vc->true;
  }
  return vc->false;
}

VCSI_OBJECT evenq(VCSI_CONTEXT vc, 
		  VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM))
    error(vc,"object passed to even? is the wrong type",x);
   
  if(TYPEP(x,LNGNUM)) {
    if(LNGN(x) % 2 == 0)
      return vc->true;
  }
  return vc->false;
}

VCSI_OBJECT num_max(VCSI_CONTEXT vc, 
		    VCSI_OBJECT args, 
		    int length) {

  VCSI_OBJECT res, tmp, num;
  float i, j, k, c, d;

  if(length < 1)
    error(vc,"invalid number of arguments to max",args);
   
  res = car(vc,args);
   
  for(tmp=CDR(args);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    num = car(vc,tmp);
	
    if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
      error(vc,"object passed to max is the wrong type",num);

    num = get_nums(vc,res,num,&i,&j,&c,&d);

    if(num) {
      if((i/c) > (j/d))
	res = make_rat_int(vc,i,c);
      else
	res = make_rat_int(vc,j,d);
    } else {
      k = (i > j) ? i : j;
      if((int)k == k)
	res = make_long(vc,(int)k);
      else
	res = make_float(vc,k);
    }
  }
  return res;
}

VCSI_OBJECT num_min(VCSI_CONTEXT vc, 
		    VCSI_OBJECT args, 
		    int length) {

  VCSI_OBJECT res, tmp, num;
  float i, j, k, c, d=0;
   
  if(length < 1)
    error(vc,"invalid number of arguments to min",args);
   
  res = car(vc,args);
   
  for(tmp=CDR(args);TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    num = car(vc,tmp);

    if(!TYPEP(num,LNGNUM) && !TYPEP(num,FLTNUM) && !TYPEP(num,RATNUM))
      error(vc,"object passed to min is the wrong type",num);

    num = get_nums(vc,res,num,&i,&j,&c,&c);

    if(num) {
      if((i/c) < (j/d))
	res = make_rat_int(vc,i,c);
      else
	res = make_rat_int(vc,j,d);
    } else {
      k = (i < j) ? i : j;
      if((int)k == k)
	res = make_long(vc,(int)k);
      else
	res = make_float(vc,k);
    }
  }
  return res;
}

VCSI_OBJECT num_abs(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to abs is the wrong type",x);
   
  if(TYPEP(x,LNGNUM))
    return make_long(vc,abs(LNGN(x)));
  else if(TYPEP(x,FLTNUM))
    return make_float(vc,fabs(FLTN(x)));
  else
    return make_rat_int(vc,abs(RNUM(x)),RDENOM(x));
}

VCSI_OBJECT num_quotient(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x, 
			 VCSI_OBJECT y) {
  int exact = 1;
   
  if(TYPEP(x,FLTNUM) && FLTN(x) == (long)(FLTN(x))) {
    exact = 0;
    x = make_long(vc,FLTN(x));
  }
   
  if(TYPEP(y,FLTNUM) && FLTN(y) == (long)(FLTN(y))) {
    exact = 0;
    y = make_long(vc,FLTN(y));
  }
   
  if(!TYPEP(x,LNGNUM))
    error(vc,"object passed to quotient is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    error(vc,"object passed to quotient is the wrong type",y);
  if(LNGN(y) == 0)
    error(vc,"attempted divide by zero",y);
  if(exact)
    return make_long(vc,LNGN(x) / LNGN(y));
  return make_float(vc,LNGN(x) / LNGN(y));
}

VCSI_OBJECT num_remainder(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {
  int exact = 1;
   
  if(TYPEP(x,FLTNUM) && FLTN(x) == (long)(FLTN(x))) {
    exact = 0;
    x = make_long(vc,FLTN(x));
  }
   
  if(TYPEP(y,FLTNUM) && FLTN(y) == (long)(FLTN(y))) {
    exact = 0;
    y = make_long(vc,FLTN(y));
  }
   
  if(!TYPEP(x,LNGNUM))
    error(vc,"object passed to remainder is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    error(vc,"object passed to remainder is the wrong type",y);
  if(LNGN(y) == 0)
    error(vc,"attempted divide by zero",y);
  if(exact)
    return make_long(vc,LNGN(x) % LNGN(y));
  return make_float(vc,LNGN(x) % LNGN(y));
}

VCSI_OBJECT num_modulo(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y) {
  int exact = 1;
   
  if(TYPEP(x,FLTNUM) && (FLTN(x) == (long)(FLTN(x)))) {
    exact = 0;
    x = make_long(vc,FLTN(x));
  }
   
  if(TYPEP(y,FLTNUM) && (FLTN(y) == (long)(FLTN(y)))) {
    exact = 0;
    y = make_long(vc,FLTN(y));
  }
   
	  
  if(!TYPEP(x,LNGNUM))
    error(vc,"object passed to modulo is the wrong type",x);
  if(!TYPEP(y,LNGNUM))
    error(vc,"object passed to modulo is the wrong type",y);
  if(LNGN(y) == 0)
    error(vc,"attempted divide by zero",y);
  if(exact) {
    if((LNGN(x) > 0 && LNGN(y) < 0) || (LNGN(x) < 0 && LNGN(y) > 0))
      return make_long(vc,-1*((((LNGN(x) / LNGN(y))-1)*LNGN(y))-LNGN(x)));
    return make_long(vc,LNGN(x) % LNGN(y));
  }
  else {
    if((LNGN(x) > 0 && LNGN(y) < 0) || (LNGN(x) < 0 && LNGN(y) > 0))
      return make_float(vc,-1*((((LNGN(x) / LNGN(y))-1)*LNGN(y))-LNGN(x)));
    return make_float(vc,LNGN(x) % LNGN(y));
  }
}

int compute_gcd(int a, int b) {
  int i;
   
  a = abs(a);
  b = abs(b);
   
  if(b > a) {
    i = a;
    a = b;
    b = i;
  }
  
  while(b != 0) {
    i = a % b;
    a = b;
    b = i;
  }
  return a;
}

VCSI_OBJECT num_gcd(VCSI_CONTEXT vc, 
		    VCSI_OBJECT args, 
		    int length) {

  VCSI_OBJECT res, tmp, num;
  int exact = 1;
   
  res = make_long(vc,0);
  for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    num = car(vc,tmp);
	
    if(TYPEP(num,FLTNUM) && FLTN(num) ==(long)FLTN(num)) {
      exact = 0;
      res = make_long(vc,compute_gcd(LNGN(res),FLTN(num)));
    } else {
      if(!TYPEP(num,LNGNUM))
	error(vc,"object passed to gcd is the wrong type",num);
      res = make_long(vc,compute_gcd(LNGN(res),LNGN(num)));
    }
  }

  if(exact)
    return res;

  return make_float(vc,LNGN(res));
}

VCSI_OBJECT num_lcm(VCSI_CONTEXT vc, 
		    VCSI_OBJECT args, 
		    int length) {

  VCSI_OBJECT tmp, num;
  long i,j, k;
  int exact = 1;
  if(length == 0)
    return make_long(vc,0);

  num = num_max(vc,args,length);

  if(TYPEP(num,FLTNUM) && FLTN(num) == (long)FLTN(num)) {
    exact = 0;
    num = make_long(vc,FLTN(num));
  }

  if(!TYPEP(num,LNGNUM))
    error(vc,"object passed to lcm is the wrong type",num);
   
  tmp = multiply(vc,args,length);

  if(TYPEP(tmp,FLTNUM) && FLTN(tmp) == (long)FLTN(tmp)) {
    exact = 0;
    tmp = make_long(vc,FLTN(tmp));
  }

  if(!TYPEP(tmp,LNGNUM))
    error(vc,"object passed to lcm is the wrong type",tmp);

  k = LNGN(tmp);
  j = LNGN(num);

  for(i=j;i<k;i=i+j) {

    for(tmp=args;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
      num = car(vc,tmp);
	     
      if(TYPEP(num,FLTNUM) && FLTN(num) == (long)FLTN(num)) {
	exact = 0;
	num = make_long(vc,FLTN(num));
      }
      if(!TYPEP(num,LNGNUM))
	error(vc,"object passed tolcm> is the wrong type",num);
	     
      if(i % LNGN(num) != 0)
	break;
    }

    if(!TYPEP(tmp,CONS))
      break;
  }

  if(exact)
    return make_long(vc,i);
  return make_float(vc,i);
}

VCSI_OBJECT num_numer(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {
  if(TYPEP(x,LNGNUM))
    return x;
  else if(TYPEP(x,RATNUM))
    return make_long(vc,RNUM(x));
  else if(TYPEP(x,FLTNUM)) {
    x = inexact_exact(vc,x);
    if(TYPEP(x,LNGNUM))
      return make_float(vc,LNGN(x));
    if(TYPEP(x,RATNUM))
      return make_float(vc,RNUM(x));
  }
  return error(vc,"object passed to numerator is the wrong type",x);
}

VCSI_OBJECT num_denom(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  if(TYPEP(x,LNGNUM))
    return make_long(vc,1);
  else if(TYPEP(x,RATNUM))
    return make_long(vc,RDENOM(x));
  else if(TYPEP(x,FLTNUM)) {
    x = inexact_exact(vc,x);
    if(TYPEP(x,LNGNUM))
      return make_float(vc,LNGN(x));
    if(TYPEP(x,RATNUM))
      return make_float(vc,RDENOM(x));
  }
  return error(vc,"object passed to denominator is the wrong type",x);
}  

VCSI_OBJECT num_floor(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to floor is the wrong type",x);
   
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,(float)LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_long(vc,floor(FLTN(x)));
}

VCSI_OBJECT num_ceil(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to ceiling is the wrong type",x);
   
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,(float)LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_long(vc,ceil(FLTN(x)));
}

VCSI_OBJECT num_trunc(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to truncate is the wrong type",x);
   
  if(TYPEP(x,LNGNUM))
    return x;
  if(TYPEP(x,RATNUM))
    x = make_long(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,(long)FLTN(x));
}

VCSI_OBJECT num_round(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to round is the wrong type",x);
   
  if(TYPEP(x,LNGNUM))
    return x;
  if(TYPEP(x,RATNUM))
    return make_long(vc,rint((float)RNUM(x)/(float)RDENOM(x)));
   
  return make_float(vc,rint(FLTN(x)));
}

VCSI_OBJECT num_rationalize(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y) {

  float r1,r2,d1,d2,i,j,k,l,e = 1;
   
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to rationalize is the wrong type",x);
  if(TYPEP(x,FLTNUM))
    e = 0;
   
  if(!TYPEP(y,LNGNUM) && !TYPEP(y,FLTNUM) && !TYPEP(y,RATNUM))
    error(vc,"object passed to rationalize is the wrong type",y);
   
  x = inexact_exact(vc,x);
  y = inexact_exact(vc,y);
   
  get_nums(vc,x,y,&r1,&r2,&d1,&d2);
  if(d1 != d2) {
    i = d1;
    r1 *= d2;
    d1 *= d2;
    r2 *= i;
    d2 *= i;
  }
  for(j=1;j<d1;j++) {
    for(k=0;k<r1+r2;k++) {
      l = k/j;
      if(l <= (r1+r2)/d1 && l >= (r1-r2)/d1) {
	if(e)
	  return make_rat_int(vc,(int)k,(int)j);
	return make_float(vc,k/j);
      }
    }
  }
  return make_long(vc,0);
}

VCSI_OBJECT num_exp(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to exp is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,exp(FLTN(x)));
}

VCSI_OBJECT num_log(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to log is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,log(FLTN(x)));
}

VCSI_OBJECT num_sin(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to sin is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,sin(FLTN(x)));
}

VCSI_OBJECT num_cos(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to cos is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,cos(FLTN(x)));
}

VCSI_OBJECT num_tan(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to tan is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,tan(FLTN(x)));
}

VCSI_OBJECT num_asin(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to asin is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,asin(FLTN(x)));
}

VCSI_OBJECT num_acos(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to acos is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  return make_float(vc,acos(FLTN(x)));
}

VCSI_OBJECT num_atan(VCSI_CONTEXT vc, 
		     VCSI_OBJECT args, 
		     int length) {
  VCSI_OBJECT x, y;

  if(length > 2)
    error(vc,"invalid number of arguments to atan",args);
  
  x = car(vc,args);
  
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to atan is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));

  if(length == 1)
    return make_float(vc,atan(FLTN(x)));
  
  y = cadr(vc,args);
  
  if(!TYPEP(y,LNGNUM) && !TYPEP(y,FLTNUM) && !TYPEP(y,RATNUM))
    error(vc,"object passed to atan is the wrong type",y);
  if(TYPEP(y,LNGNUM))
    y = make_float(vc,LNGN(y));
  if(TYPEP(y,RATNUM))
    y = make_float(vc,(float)RNUM(y)/(float)RDENOM(y));
  return make_float(vc,atan2(FLTN(x),FLTN(y)));
}

VCSI_OBJECT num_sqrt(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  float z;
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to sqrt is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  z = sqrt(FLTN(x));
  if(z == (long)z)
    return make_long(vc,z);
  return make_float(vc,z);
}

VCSI_OBJECT num_expt(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y) {

  float z;
   
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to expt is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
   
  if(!TYPEP(y,LNGNUM) && !TYPEP(y,FLTNUM) && !TYPEP(y,RATNUM))
    error(vc,"object passed to expt is the wrong type",y);
  if(TYPEP(y,LNGNUM))
    y = make_float(vc,LNGN(y));
  if(TYPEP(y,RATNUM))
    y = make_float(vc,(float)RNUM(y)/(float)RDENOM(y));

  z = pow(FLTN(x),FLTN(y));
  if(z == (long)z)
    return make_long(vc,z);
  return make_float(vc,z);
}

VCSI_OBJECT exact_inexact(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x) {

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to exact->inexact is the wrong type",x);
  if(TYPEP(x,LNGNUM))
    x = make_float(vc,LNGN(x));
  if(TYPEP(x,RATNUM))
    x = make_float(vc,(float)RNUM(x)/(float)RDENOM(x));
  return x;
}

VCSI_OBJECT inexact_exact(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x) {
  
  float y;
  int i;
   
  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to inexact->exact is the wrong type",x);
  if(TYPEP(x,LNGNUM) || TYPEP(x,RATNUM))
    return x;
  if(FLTN(x) == (long)FLTN(x))
    return make_long(vc,FLTN(x));
  for(y = FLTN(x),i=1;y != (long)y && i < 10;i++,y=y*10);
  return make_rat_int(vc,y,pow(10,i-1));
}

VCSI_OBJECT num_string(VCSI_CONTEXT vc, 
		       VCSI_OBJECT args, 
		       int length) {

  VCSI_OBJECT x=NULL, y=NULL;
  char number[128];
  long num,mask;
  int i;
  char b;

  memset(number,0,sizeof(char)*128);

  if(length >2 || length < 1)
    error(vc,"invalid number of arguments to number->string",args);

  x = car(vc,args);

  if(!TYPEP(x,LNGNUM) && !TYPEP(x,FLTNUM) && !TYPEP(x,RATNUM))
    error(vc,"object passed to number->string is the wrong type",x);
   
  if(length != 1) {
    y = cadr(vc,args);
	
    if(!TYPEP(y,LNGNUM))
      error(vc,"object passed to number->string is the wrong type",y);
  }
   
  if(length == 1 || LNGN(y) == 10) {
	
    if(TYPEP(x,LNGNUM))
      sprintf(number,"%ld",LNGN(x));
    else if(TYPEP(x,FLTNUM))
      sprintf(number,"%f",FLTN(x));
    else if(TYPEP(x,RATNUM))
      sprintf(number,"%d/%d",RNUM(x),RDENOM(x));
    return make_string(vc,number);
  }
  else if(LNGN(y) == 2 && TYPEP(x,LNGNUM)) {
    num = LNGN(x);
    i=sizeof(long)*8;
    mask=1<<(i-1);
    b = 0;
    while(i--) {
      if(!b && (num & mask))
	b = '0';

      number[strlen(number)] = (num & mask) ? '1' : b;
      num <<= 1; 
    }
  }
  else if(LNGN(y) == 8 && TYPEP(x,LNGNUM))
    sprintf(number,"%o",(unsigned int)LNGN(x));
  else if(LNGN(y) == 16 && TYPEP(x,LNGNUM))
    sprintf(number,"%x",(unsigned int)LNGN(x));
  else
    return vc->false;
  return make_string(vc,number);
}

VCSI_OBJECT string_num(VCSI_CONTEXT vc, 
		       VCSI_OBJECT args, 
		       int length) {
  int i, c;
  VCSI_OBJECT x=NULL, y=NULL;
  long z = 0;
   
  if(length >2 || length < 1)
    error(vc,"invalid number of arguments to string->number",args);

  x = car(vc,args);
   
  if(!TYPEP(x,STRING))
    error(vc,"object passed to string->number is the wrong type",x);
  if(length != 1) {
    y = cadr(vc,args);
     
    if(!TYPEP(y,LNGNUM))
      error(vc,"object passed to string->number is the wrong type",y);
  }
   
  if(length == 1 || LNGN(y) == 10) {
    i = is_num(STR(x));
    if(i == 1)
      return make_long_str(vc,STR(x));
    else if(i == 2)
      return make_float(vc,atof(STR(x)));
    else if(i == 3)
      return make_rat(vc,STR(x));
    else
      return vc->false;
  }
   
  if(LNGN(y) == 2) {
    for(c=0,i=strlen(STR(x)) -1;i >= 0;i--,c++) {
      if(STR(x)[i] == '1')
	z += pow(2,c);
      else if(STR(x)[i] == '0')
	continue;
      else if(STR(x)[i] == ' ')
	break;
      else
	return vc->false;
    }
  }
  else if(LNGN(y) == 8)
    sscanf(STR(x),"%o",(unsigned int*)&z);
  else if(LNGN(y) == 16)
    sscanf(STR(x),"%x",(unsigned int*)&z);
  else
    return vc->false;
   
  return make_long(vc,z);
}

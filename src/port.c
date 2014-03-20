#include "vcsi.h"

void port_init(VCSI_CONTEXT vc)
{
  
  vc->port_in = make_port_fp(vc,stdin,"console",PORTI);
  vc->port_curr_in = vc->port_in;
   
  vc->port_out = make_port_fp(vc,stdout,"console",PORTO);
  vc->port_curr_out = vc->port_out;
   
  vc->eofobj = make_protected("EOF Object");

  set_int_proc(vc,"call-with-input-file",PROCENV,call_w_i_file);
  set_int_proc(vc,"call-with-output-file",PROCENV,call_w_o_file);

  set_int_proc(vc,"input-port?",PROC1,portiq);
  set_int_proc(vc,"output-port?",PROC1,portoq);

  set_int_proc(vc,"current-input-port",PROC0,get_curr_in);
  set_int_proc(vc,"current-output-port",PROC0,get_curr_out);

  set_int_proc(vc,"with-input-from-file",PROCENV,with_i_f_file);
  set_int_proc(vc,"with-output-to-file",PROCENV,with_o_t_file);

  set_int_proc(vc,"open-input-file",PROC1,open_input_file);
  set_int_proc(vc,"open-output-file",PROC1,open_output_file);

  set_int_proc(vc,"close-input-port",PROC1,close_input_port);
  set_int_proc(vc,"close-output-port",PROC1,close_output_port);

  set_int_proc(vc,"open-input-pipe",PROC1,open_input_pipe);

  set_int_proc(vc,"read",PROCX,port_read);
  set_int_proc(vc,"read-string",PROCX,port_read_string);
  set_int_proc(vc,"read-char",PROCX,port_read_char);
  set_int_proc(vc,"peek-char",PROCX,port_peek_char);

  set_int_proc(vc,"eof-object?",PROC1,eofq);
   
  set_int_proc(vc,"char-ready?",PROCX,port_char_ready);

  set_int_proc(vc,"write",PROCOPT,port_write);
  set_int_proc(vc,"display",PROCOPT,port_display);
  set_int_proc(vc,"newline",PROCX,port_newline);
  set_int_proc(vc,"write-char",PROCOPT,port_write_char);
}

VCSI_OBJECT make_port_fp(VCSI_CONTEXT vc, 
			 FILE* fp, 
			 char* name, 
			 VCSI_OBJECT_TYPE type) {

  VCSI_OBJECT tmp;
  tmp = alloc_obj(vc,type);
  PORTFP(tmp) = fp;
  PORTP(tmp) = strdup(name);
  return tmp;
}

VCSI_OBJECT make_port(VCSI_CONTEXT vc, 
		      char* path, 
		      VCSI_OBJECT_TYPE type) {

  VCSI_OBJECT tmp;
   
  tmp = alloc_obj(vc,type);
   
  if(type == PORTI)
    PORTFP(tmp) = fopen(path,"r");
  else if (type == PORTO)
    PORTFP(tmp) = fopen(path,"w");
  else if(type == PORTB)
    PORTFP(tmp) = fopen(path,"rw");
  if(!PORTFP(tmp))
    return error(vc,"can't open file",make_string(vc,path));
   
  PORTP(tmp) = strdup(path);
   
  return tmp;
}

VCSI_OBJECT close_port(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {
  if(!eq(vc,x,vc->port_curr_in) && !eq(vc,x,vc->port_curr_out) &&
     PORTFP(x) != stdin && PORTFP(x) != stdout) {
      fclose(PORTFP(x));
      PORTFP(x) = NULL;
    }
  return x;
}
   
VCSI_OBJECT call_w_i_file(VCSI_CONTEXT vc, 
			  VCSI_OBJECT args, 
			  VCSI_OBJECT env) {
  VCSI_OBJECT x, y, tmp;
   
  if(get_length(args) != 2)
    return error(vc,"invalid number of arguments to call-with-input-file",args);
  x = eval(vc,car(vc,args),env);
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to call-with-input-file is the wrong type",x);
   
  tmp = make_port(vc,STR(x),PORTI);
   
  y = CAR(CDR(args));
   
  return eval(vc,cons(vc,y,cons(vc,tmp,NULL)),env);
}
 
VCSI_OBJECT call_w_o_file(VCSI_CONTEXT vc, 
			  VCSI_OBJECT args, 
			  VCSI_OBJECT env) {

  VCSI_OBJECT x, y, tmp;
  if(get_length(args) != 2)
    return error(vc,"invalid number of arguments to call-with-output-file",args);
  x = eval(vc,car(vc,args),env);
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to call-with-output-file is the wrong type",x);
   
  tmp = make_port(vc,STR(x),PORTO);
   
  y = CAR(CDR(args));
   
  return eval(vc,cons(vc,y,cons(vc,tmp,NULL)),env);
}

VCSI_OBJECT portiq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x) {

  if(TYPEP(x,PORTI) || TYPEP(x,PORTB))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT portoq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x) {

  if(TYPEP(x,PORTO) || TYPEP(x,PORTB))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT get_curr_in(VCSI_CONTEXT vc) {
  return vc->port_curr_in;
}

VCSI_OBJECT get_curr_out(VCSI_CONTEXT vc) {
  return vc->port_curr_out;
}

VCSI_OBJECT with_i_f_file(VCSI_CONTEXT vc, 
			  VCSI_OBJECT args, 
			  VCSI_OBJECT env) {

  VCSI_OBJECT x, y, tmp, c_old;
  if(get_length(args) != 2)
    return error(vc,"invalid number of arguments with-input-from-file",args);
  x = eval(vc,car(vc,args),env);
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to with-input-from-file is the wrong type",x);
   
  tmp = make_port(vc,STR(x),PORTI);
   
  c_old = vc->port_curr_in;
  vc->port_curr_in = tmp;
   
  y = eval(vc,CDR(args),env);
 
  vc->port_curr_in = c_old;
   
  return y;
}

VCSI_OBJECT with_o_t_file(VCSI_CONTEXT vc, 
			  VCSI_OBJECT args, 
			  VCSI_OBJECT env) {

  VCSI_OBJECT x, y, tmp, c_old;   
   
  if(get_length(args) != 2)
    return error(vc,"invalid number of arguments with-output-to-file",args);
  x = eval(vc,car(vc,args),env);
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to with-output-to-file is the wrong type",x);
   
  tmp = make_port(vc,STR(x),PORTO);
      
  c_old = vc->port_curr_out;
  vc->port_curr_in = tmp;
   
  y = eval(vc,CDR(args),env);
   
  vc->port_curr_out = c_old;
   
  return y;
}

VCSI_OBJECT open_input_file(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to open-input-file is the wrong type",x);
  tmp = make_port(vc,STR(x),PORTI);
  return tmp;
}

VCSI_OBJECT open_output_file(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to open-output-file is the wrong type",x);
  tmp = make_port(vc,STR(x),PORTO);
  return tmp;
}

VCSI_OBJECT close_input_port(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x) {

  if(!TYPEP(x,PORTI) && !TYPEP(x,PORTB))
    return error(vc,"object passed to close-input-port is the wrong type",x);
  close_port(vc,x);
  return vc->novalue;
}

VCSI_OBJECT close_output_port(VCSI_CONTEXT vc, 
			      VCSI_OBJECT x) {

  if(!TYPEP(x,PORTO) && !TYPEP(x,PORTB))
    return error(vc,"object passed to close-output-port is the wrong type",x);
  close_port(vc,x);
  return vc->novalue;
}

VCSI_OBJECT open_input_pipe(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  int fd;
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to open-input-file is the wrong type",x);
  fd = open(STR(x), O_RDONLY | O_NONBLOCK);
  tmp = make_port_fp(vc,fdopen(fd,"r"),STR(x),PORTI);
  return tmp;
}


VCSI_OBJECT port_read(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  VCSI_OBJECT tmp;

  if(x != vc->novalue)
    tmp = x;
  else
    tmp = vc->port_curr_in;
   
  if(!TYPEP(tmp,PORTI) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to read is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
   
  if(get_pmatch(vc,PORTFP(tmp)) == -1)
    return vc->eofobj;
  return parse_text(vc,vc->input->buff);
}

/* INFOSTD read-string addon */
VCSI_OBJECT port_read_string(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
    
  if(x != vc->novalue)
    tmp = x;
  else
    tmp = vc->port_curr_in;
  
  if(!TYPEP(tmp,PORTI) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to read is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
   
  if(infostd_dyn_str_fgets(vc->tmpstr,PORTFP(tmp)))
    return make_string(vc,vc->tmpstr->buff);
  
  return vc->eofobj;

}
  
VCSI_OBJECT port_read_char(VCSI_CONTEXT vc, 
			   VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  int i;

  if(x != vc->novalue)
    tmp = x;
  else
    tmp = vc->port_curr_in;
   
  if(!TYPEP(tmp,PORTI) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to read-char is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
   
  if((i = fgetc(PORTFP(tmp))) == -1)
    return vc->eofobj;
  return make_char_int(vc,i);
}

VCSI_OBJECT port_peek_char(VCSI_CONTEXT vc, 
			   VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  int i;

  if(x != vc->novalue)
    tmp = x;
  else
    tmp = vc->port_curr_in;
   
  if(!TYPEP(tmp,PORTI) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to peek-char is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
   
  if((i = fgetc(PORTFP(tmp))) == -1)
    return vc->eofobj;
  else
    {
      ungetc(i,PORTFP(tmp));
      return make_char_int(vc,i);
    }
}

VCSI_OBJECT eofq(VCSI_CONTEXT vc, 
		 VCSI_OBJECT x) {

  if(eq(vc,x,vc->eofobj))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT port_char_ready(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  fd_set rfds;
  struct timeval tv;
  int i, j;
   
  if(x != vc->novalue)
    tmp = x;
  else
    tmp = vc->port_curr_in;
   
  if(!TYPEP(tmp,PORTI) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to char-ready? is the wrong type",tmp);
   
  if(feof(PORTFP(tmp)))
    return vc->true;
   
  j = fileno(PORTFP(tmp));
  FD_ZERO(&rfds);
  FD_SET(j, &rfds);
   
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  i = select(j+1, &rfds, NULL, NULL, &tv);
   
  if(i)
    return vc->true;
  
  return vc->false;
}

VCSI_OBJECT port_write(VCSI_CONTEXT vc, 
		       VCSI_OBJECT args, 
		       int length) {

  VCSI_OBJECT tmp, x;

  if(length > 2)
    return error(vc,"too many arguments to write",args);
   
  x = car(vc,args);    
  
  if(length == 2)
    tmp = cadr(vc,args);
  else
    tmp = vc->port_curr_out;
      
  if(!TYPEP(tmp,PORTO) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to write is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
  fprintf(PORTFP(tmp),"%s",print_obj(vc,vc->tmpstr,x,1));
  fflush(PORTFP(tmp));
  return vc->novalue;
}

VCSI_OBJECT port_display(VCSI_CONTEXT vc, 
			 VCSI_OBJECT args, 
			 int length) {

  VCSI_OBJECT tmp, x;

  if(length > 2)
    return error(vc,"too many arguments to display",args);
   
  x = car(vc,args);
      
  if(length == 2)
    tmp = cadr(vc,args);
  else
    tmp = vc->port_curr_out;
   
  if(!TYPEP(tmp,PORTO) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to display is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
   
  if(TYPEP(x,STRING))
    fprintf(PORTFP(tmp),"%s",STR(x));
  else if(TYPEP(x,CHAR))
    fprintf(PORTFP(tmp),"%c",CHAR(x));
  else
    fprintf(PORTFP(tmp),"%s",print_obj(vc,vc->tmpstr,x,1));

  fflush(PORTFP(tmp));
  return vc->novalue;
}

VCSI_OBJECT port_newline(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x) {

  VCSI_OBJECT tmp;

  if(x != vc->novalue)
    tmp = x;
  else
    tmp = vc->port_curr_out;
   
  if(!TYPEP(tmp,PORTO) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to newline is the wrong type",tmp);
	
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);

  fprintf(PORTFP(tmp),"\n\r");
  fflush(PORTFP(tmp));
   
  return vc->novalue;
}

VCSI_OBJECT port_write_char(VCSI_CONTEXT vc, 
			    VCSI_OBJECT args,
			    int length) {

  VCSI_OBJECT tmp, x;

  if(length > 2)
    return error(vc,"too many arguments to write-char",args);
   
  x = car(vc,args);
     
  if(!TYPEP(x,CHAR))
    return error(vc,"object passed to write-char is the wrong type",x);
   
  if(length == 2)
    tmp = cadr(vc,args);
  else
    tmp = vc->port_curr_out;
   
  if(!TYPEP(tmp,PORTO) && !TYPEP(tmp,PORTB))
    return error(vc,"object passed to write-char is the wrong type",tmp);
   
  if(PORTFP(tmp) == NULL)
    return error(vc,"closed port",tmp);
   
  fprintf(PORTFP(tmp),"%c",CHAR(x));
   
  return vc->novalue;
}

#include <vcsi.h>
#include "pipe.h"

const char module_name[] = "pipe";

void module_init(VCSI_CONTEXT vc) { 
  set_int_proc(vc,"pipe-open",PROC1,pipe_open);
  set_int_proc(vc,"pipe-read",PROC1,pipe_read);
}

VCSI_OBJECT pipe_open(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

   int fd;

   if(!TYPEP(x,STRING))
     return error(vc,"Invalid type to pipe-open",x);
      
   fd = open(STR(x), O_RDONLY | O_NONBLOCK);
   if(fd == -1)
     return error(vc,"Unable to open file in pipe-open", x);

   return make_long(vc,fd);
}

VCSI_OBJECT pipe_read(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  char buf[1024];
  int len;

   if(!TYPEP(x,LNGNUM))
     return error(vc,"Invalid type to pipe-read",x);

   memset(buf,0,1024);

   if(read(LNGN(x),&buf,1024) > 0) {
     len = strlen(buf);
     if(len > 0 && buf[len-1] == '\n') {
       if(len - 1 == 0) 
	 return vc->eofobj;
       buf[len-1] = 0;
     }
     return make_string(vc,buf);
   }
   return vc->eofobj;
}

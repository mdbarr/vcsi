#include <vcsi.h>
#include "mod.h"
#include "lzw.h"

const char module_name[] = "compress";

void module_init(VCSI_CONTEXT vc) {
  set_int_proc(vc,"lzw-compress",PROC1,lzw_comp);
  set_int_proc(vc,"lzw-decompress",PROC1,lzw_decomp);
}

VCSI_OBJECT lzw_comp(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x) {

  VCSI_OBJECT tmp;
  char *s;
  if(!TYPEP(x,STRING))
    return error(vc,"Invalid type to lzw-compress",x);
   
  s = lzw_compress(STR(x));
  tmp = make_string(vc,s);
  free(s);
   
  return tmp;
}

VCSI_OBJECT lzw_decomp(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {
  VCSI_OBJECT tmp;
  char *s;
  if(!TYPEP(x,STRING))
    return error(vc,"Invalid type to lzw-decompress",x);
   
  s = lzw_decompress(STR(x));
  tmp = make_string(vc,s);
  free(s);
   
  return tmp;
}
   

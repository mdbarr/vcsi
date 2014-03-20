#include <vcsi.h>
#include <gdbm.h>

const char module_name[] = "gdbm";

typedef struct str_gdbm_obj {
  GDBM_FILE dbf;
  
  char* file;

  int mode;

  short open;
} *GDBM_OBJ;
#define SIZE_GDBM_OBJ (sizeof(struct str_gdbm_obj))


/* User type handlers */
VCSI_USER_DEF gdbm_type = NULL;

void print_gdbm(VCSI_CONTEXT vc,
		INFOSTD_DYN_STR res, 
		VCSI_OBJECT obj) {

  GDBM_OBJ gobj = (GDBM_OBJ)UDVAL(obj);

  infostd_dyn_str_printf(vc->res_extra,"#[gdbm \"%s\" (%s)]",gobj->file,
	  (!gobj->open) ? "closed" :  (gobj->mode == GDBM_WRCREAT) 
	  ? "rw" : "r");
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void free_gdbm(VCSI_CONTEXT vc,
	       VCSI_OBJECT obj) {

  GDBM_OBJ gobj;
  
  gobj = (GDBM_OBJ)UDVAL(obj);
  gdbm_close(gobj->dbf);
  FREE(gobj->file);
  FREE(gobj);
}

/* Main functions */
void check_gdbm(VCSI_CONTEXT vc, 
		VCSI_OBJECT gdb) {
  GDBM_OBJ gobj = NULL;
  
  if(!same_type_user_def(vc,gdb,gdbm_type))
    error(vc,"Not a GDBM Database",gdb);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
 
  if(!gobj->open)
    error(vc,"GDBM Database is not open",gdb);
}

VCSI_OBJECT do_open_gdbm(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x, 
			 int flag) {
  GDBM_OBJ gobj = NULL;
  VCSI_OBJECT tmp = NULL;
  GDBM_FILE dbf = NULL;

  if(!TYPEP(x,STRING))
    error(vc,"Incorrect type to gdbm-open",x);

  dbf = gdbm_open(STR(x),0,flag|GDBM_SYNC|GDBM_NOLOCK,0644,0);
  if(!dbf)
    error(vc,"Couldn't open GDBM file",x);

  gobj = MALLOC(SIZE_GDBM_OBJ);
  gobj->dbf = dbf;
  gobj->file = strdup(STR(x));
  gobj->mode = flag;
  gobj->open=1;
    
  tmp = make_user_def(vc,(void*)gobj,gdbm_type);
  
  return tmp;
}

VCSI_OBJECT open_gdbm(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  return do_open_gdbm(vc,x,GDBM_WRCREAT);
}

VCSI_OBJECT read_open_gdbm(VCSI_CONTEXT vc, 
			   VCSI_OBJECT x) {

  return do_open_gdbm(vc,x,GDBM_READER);
}

VCSI_OBJECT close_gdbm(VCSI_CONTEXT vc, 
		       VCSI_OBJECT gdb) {

  GDBM_OBJ gobj = NULL;

  if(!same_type_user_def(vc,gdb,gdbm_type))
    error(vc,"Not a GDBM Database",gdb);

  gobj = (GDBM_OBJ)UDVAL(gdb);

  if(gobj->open) {
    gdbm_close(gobj->dbf);
    gobj->open=0;
    return vc->true;
  }
    
  return vc->false;
}

int do_store_gdbm(VCSI_CONTEXT vc, 
		  VCSI_OBJECT gdb, 
		  VCSI_OBJECT x, 
		  VCSI_OBJECT y, 
		  int flag) {

  GDBM_OBJ gobj;
  datum key, content;

  check_gdbm(vc,gdb);

  if(!TYPEP(x,STRING))
    error(vc,"Incorrect type to gdbm-store",x);
  if(!TYPEP(y,STRING))
    error(vc,"Incorrect type to gdbm-store",y);

  gobj = (GDBM_OBJ)UDVAL(gdb);

  key.dptr = STR(x);
  key.dsize = strlen(key.dptr)+1;

  content.dptr = STR(y);
  content.dsize = strlen(content.dptr)+1;
  
  return gdbm_store(gobj->dbf,key,content,flag);
}

VCSI_OBJECT store_gdbm(VCSI_CONTEXT vc, 
		       VCSI_OBJECT gdb, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y) {

  if(do_store_gdbm(vc,gdb,x,y,GDBM_INSERT)==0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT replace_store_gdbm(VCSI_CONTEXT vc, 
			       VCSI_OBJECT gdb, 
			       VCSI_OBJECT x, 
			       VCSI_OBJECT y) {
  
  if(do_store_gdbm(vc,gdb,x,y,GDBM_REPLACE)==0)
    return vc->true;
  return vc->false;
}


VCSI_OBJECT fetch_gdbm(VCSI_CONTEXT vc, 
		       VCSI_OBJECT gdb, 
		       VCSI_OBJECT x) {

  GDBM_OBJ gobj;
  VCSI_OBJECT tmp;
  datum key, content;

  check_gdbm(vc,gdb);
  
  if(!TYPEP(x,STRING))
    error(vc,"Incorrect type to gdbm-fetch",x);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  key.dptr = STR(x);
  key.dsize = strlen(key.dptr)+1;

  content = gdbm_fetch(gobj->dbf, key);

  if(content.dptr) {
    tmp = make_string(vc,content.dptr);
    FREE(content.dptr);
    return tmp;
  }
  return vc->false;
}

VCSI_OBJECT exists_gdbm(VCSI_CONTEXT vc, 
			VCSI_OBJECT gdb, 
			VCSI_OBJECT x) {

  GDBM_OBJ gobj;
  datum key;
  int ret;
  
  check_gdbm(vc,gdb);

  if(!TYPEP(x,STRING))
    error(vc,"Incorrect type to gdbm-store",x);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  key.dptr = STR(x);
  key.dsize = strlen(key.dptr)+1;

  ret = gdbm_exists(gobj->dbf,key);
  if(ret)
    return vc->true;

  return vc->false;
}

VCSI_OBJECT delete_gdbm(VCSI_CONTEXT vc, 
			VCSI_OBJECT gdb, 
			VCSI_OBJECT x) {

  GDBM_OBJ gobj;
  datum key;
  int ret;
  
  check_gdbm(vc,gdb);

  if(!TYPEP(x,STRING))
    error(vc,"Incorrect type to gdbm-store",x);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  key.dptr = STR(x);
  key.dsize = strlen(key.dptr)+1;

  ret = gdbm_delete(gobj->dbf,key);
  if(ret==0)
    return vc->true;

  return vc->false;
}

VCSI_OBJECT firstkey_gdbm(VCSI_CONTEXT vc, 
			  VCSI_OBJECT gdb) {

  GDBM_OBJ gobj;
  VCSI_OBJECT tmp;
  datum key;

  check_gdbm(vc,gdb);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  key = gdbm_firstkey(gobj->dbf);
  if(key.dptr) {
    tmp = make_string(vc,key.dptr);
    FREE(key.dptr);
    return tmp;
  }
  
  return vc->false;
}

VCSI_OBJECT nextkey_gdbm(VCSI_CONTEXT vc, 
			 VCSI_OBJECT gdb, 
			 VCSI_OBJECT x) {

  GDBM_OBJ gobj;
  VCSI_OBJECT tmp;
  datum key, content;
  
  check_gdbm(vc,gdb);

  if(!TYPEP(x,STRING))
    error(vc,"Incorrect type to gdbm-store",x);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  key.dptr = STR(x);
  key.dsize = strlen(key.dptr)+1;

  content = gdbm_nextkey(gobj->dbf,key);

  if(content.dptr) {
    tmp = make_string(vc,content.dptr);
    FREE(content.dptr);
    return tmp;
  }
  
  return vc->false;
}

VCSI_OBJECT reorganize_gdbm(VCSI_CONTEXT vc, 
			    VCSI_OBJECT gdb) {

  GDBM_OBJ gobj;
  int ret;
  
  check_gdbm(vc,gdb);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  ret = gdbm_reorganize(gobj->dbf);
  if(ret==0)
    return vc->true;

  return vc->false;
}

VCSI_OBJECT sync_gdbm(VCSI_CONTEXT vc, 
		      VCSI_OBJECT gdb) {

  GDBM_OBJ gobj;
  
  check_gdbm(vc,gdb);
  
  gobj = (GDBM_OBJ)UDVAL(gdb);
  
  gdbm_sync(gobj->dbf);
  
  return vc->true;
}

VCSI_OBJECT gdbmq(VCSI_CONTEXT vc, 
		  VCSI_OBJECT x) {

  if(!same_type_user_def(vc,x,gdbm_type))
    return vc->false;

  return vc->true;
}

void module_init(VCSI_CONTEXT vc) {

  gdbm_type = make_user_def_type(vc,(char*)module_name,
				 print_gdbm,NULL,free_gdbm);

  set_int_proc(vc,"gdbm?",PROC1,gdbmq);

  set_int_proc(vc,"gdbm-open",PROC1,open_gdbm);
  set_int_proc(vc,"gdbm-open-write",PROC1,open_gdbm);
  set_int_proc(vc,"gdbm-open-read",PROC1,read_open_gdbm);

  set_int_proc(vc,"gdbm-close",PROC1,close_gdbm);

  set_int_proc(vc,"gdbm-store",PROC3,store_gdbm);
  set_int_proc(vc,"gdbm-store-insert",PROC3,store_gdbm);
  set_int_proc(vc,"gdbm-store-replace",PROC3,replace_store_gdbm);

  set_int_proc(vc,"gdbm-fetch",PROC2,fetch_gdbm);

  set_int_proc(vc,"gdbm-exists",PROC2,exists_gdbm);

  set_int_proc(vc,"gdbm-delete",PROC2,delete_gdbm);

  set_int_proc(vc,"gdbm-firstkey",PROC1,firstkey_gdbm);
  set_int_proc(vc,"gdbm-nextkey",PROC2,nextkey_gdbm);

  set_int_proc(vc,"gdbm-reorganize",PROC1,reorganize_gdbm);
  set_int_proc(vc,"gdbm-sync",PROC1,sync_gdbm);
}


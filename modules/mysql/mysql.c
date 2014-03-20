#include <vcsi.h>
#include <mysql/mysql.h> 

const char module_name[] = "mysql";

typedef struct str_mysql_obj {
  MYSQL mysql;
  MYSQL_RES *res;
  MYSQL_ROW row;

  char* host;
  char* user;
  char* passwd;
  char* database;

  short connected;
} *MYSQL_OBJ;
#define SIZE_MYSQL_OBJ (sizeof(struct str_mysql_obj))


/* User type handlers */
VCSI_USER_DEF mysql_type = NULL;

VCSI_OBJECT init_mysql(VCSI_CONTEXT vc) {
  
  VCSI_OBJECT tmp;
  MYSQL_OBJ mobj;
  
  mobj = MALLOC(SIZE_MYSQL_OBJ);
  memset(mobj,0,SIZE_MYSQL_OBJ);

  tmp = make_user_def(vc,(void*)mobj,mysql_type);

  return tmp;
}

void print_mysql(VCSI_CONTEXT vc,
		 INFOSTD_DYN_STR res, 
		 VCSI_OBJECT obj) {

  MYSQL_OBJ mobj = (MYSQL_OBJ)UDVAL(obj);

  if(mobj->connected) {
    infostd_dyn_str_printf(vc->res_extra,"#[mysql \"%s\" (%s@%s)]",
			   (!mobj->database) ? "*default db*" : mobj->database,
			   (!mobj->user) ? "*default user*" : mobj->user,
			   (!mobj->host) ? "localhost" : mobj->host);
  } else
    infostd_dyn_str_printf(vc->res_extra,"#[mysql (not connected)]");
  
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void free_mysql(VCSI_CONTEXT vc,
		VCSI_OBJECT obj) {
  
  MYSQL_OBJ mobj = (MYSQL_OBJ)UDVAL(obj);

  mysql_close(&(mobj->mysql));

  if(mobj->res) mysql_free_result(mobj->res); 
  
  if(mobj->database) FREE(mobj->database);
  if(mobj->host) FREE(mobj->host);
  if(mobj->user) FREE(mobj->user);
  if(mobj->passwd) FREE(mobj->passwd);
  FREE(mobj);
}

/* Main functions */
void check_mysql(VCSI_CONTEXT vc, 
		 VCSI_OBJECT mysql) {
  MYSQL_OBJ mobj = NULL;
  
  if(!same_type_user_def(vc,mysql,mysql_type))
    error(vc,"Not a MYSQL object",mysql);
  
  mobj = (MYSQL_OBJ)UDVAL(mysql);
  
  if(!mobj->connected)
    error(vc,"MYSQL Database is not connected",mysql);
}

VCSI_OBJECT close_mysql(VCSI_CONTEXT vc, 
			VCSI_OBJECT mysql) {

  MYSQL_OBJ mobj = NULL;

  if(!same_type_user_def(vc,mysql,mysql_type))
    error(vc,"Not a MYSQL object",mysql);

  mobj = (MYSQL_OBJ)UDVAL(mysql);

  if(mobj->connected) {
    if(mobj->res) {
      mysql_free_result(mobj->res);
      mobj->res = NULL;
    }
    mysql_close(&(mobj->mysql));
    mobj->connected=0;
    return vc->true;
  }
    
  return vc->false;
}

VCSI_OBJECT connect_mysql(VCSI_CONTEXT vc, 
			  VCSI_OBJECT mysql,
			  VCSI_OBJECT host,
			  VCSI_OBJECT user,
			  VCSI_OBJECT passwd,
			  VCSI_OBJECT database) {

  MYSQL_OBJ mobj = NULL;
  my_bool my_bool_true = 1;
  
  if(!same_type_user_def(vc,mysql,mysql_type))
    error(vc,"Not a MYSQL object",mysql);
  
  mobj = (MYSQL_OBJ)UDVAL(mysql);
    
  if(host && !TYPEP(host,STRING))
    error(vc,"Not a 'string' in host to mysql-connect",host);  
  
  if(user && !TYPEP(user,STRING))
    error(vc,"Not a 'string' in user to mysql-connect",user);  
  
  if(passwd && !TYPEP(passwd,STRING))
    error(vc,"Not a 'string' in passwd to mysql-connect",passwd);  
  
  if(database && !TYPEP(database,STRING))
    error(vc,"Not a 'string' in database to mysql-connect",database);  
  
  if(host)
    mobj->host = strdup(STR(host));
  if(user)
    mobj->user = strdup(STR(user));
  if(passwd)
    mobj->passwd = strdup(STR(passwd));
  if(database)
    mobj->database = strdup(STR(database));
  
  if(mobj->connected) close_mysql(vc,mysql);

  if(mysql_real_connect(&(mobj->mysql), mobj->host,
			mobj->user, mobj->passwd,
			mobj->database,3306,NULL,0) != NULL) {

    mobj->connected = 1;
    mysql_options(&(mobj->mysql),MYSQL_OPT_RECONNECT,&my_bool_true);

    return vc->true;
  }
  return vc->false;
}

VCSI_OBJECT query_mysql(VCSI_CONTEXT vc, 
			VCSI_OBJECT mysql, 
			VCSI_OBJECT q) {

  MYSQL_OBJ mobj;  

  check_mysql(vc,mysql);
  mobj = (MYSQL_OBJ)UDVAL(mysql);
  
  if(!TYPEP(q,STRING))
    error(vc,"Not a 'string' in query mysql-fetch",q);


  if(mobj->res) {
    mysql_free_result(mobj->res);
    mobj->res = NULL;
  }
    
  if(mysql_real_query(&(mobj->mysql),STR(q),(unsigned int)SLEN(q)) == 0) {
    mobj->res = mysql_store_result(&(mobj->mysql));     
    return vc->true;
  }
  return vc->false;
}

VCSI_OBJECT fetch_row_mysql(VCSI_CONTEXT vc, 
			    VCSI_OBJECT mysql) {

  MYSQL_OBJ mobj;
  MYSQL_ROW row; 
  VCSI_OBJECT list = NULL;
  unsigned long i, cols = 0;

  check_mysql(vc,mysql);
  mobj = (MYSQL_OBJ)UDVAL(mysql);
  
  if(!mobj->res) return NULL;

  if((row = mysql_fetch_row(mobj->res))) {
    cols = (unsigned long)mysql_num_fields(mobj->res);
    /* build a list from the row */
    for(i=0;i<cols;i++)
      list = cons(vc,make_string(vc,row[i]),list);
    return vcsi_list_reverse(vc,list);
  }
  return NULL;
}

VCSI_OBJECT mysqlq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x) {

  if(!same_type_user_def(vc,x,mysql_type))
    return vc->false;

  return vc->true;
}

VCSI_OBJECT connected_mysql(VCSI_CONTEXT vc,
			    VCSI_OBJECT x) {

  MYSQL_OBJ mobj;

  if(!same_type_user_def(vc,x,mysql_type))
    error(vc,"Not a MYSQL object",x);

  mobj = (MYSQL_OBJ)UDVAL(x);
 
  if(mobj->connected)
    return vc->true;

  return vc->false;
}


VCSI_OBJECT affected_rows_mysql(VCSI_CONTEXT vc,
				VCSI_OBJECT x) {

  MYSQL_OBJ mobj;

  if(!same_type_user_def(vc,x,mysql_type))
    error(vc,"Not a MYSQL object",x);

  mobj = (MYSQL_OBJ)UDVAL(x);
 
  return make_long(vc,mysql_affected_rows(&(mobj->mysql)));
}

VCSI_OBJECT insert_id_mysql(VCSI_CONTEXT vc,
			    VCSI_OBJECT x) {
  
  MYSQL_OBJ mobj;

  if(!same_type_user_def(vc,x,mysql_type))
    error(vc,"Not a MYSQL object",x);

  mobj = (MYSQL_OBJ)UDVAL(x);
  
  return make_long(vc,mysql_insert_id(&(mobj->mysql)));
}

VCSI_OBJECT num_fields_mysql(VCSI_CONTEXT vc,
			    VCSI_OBJECT x) {
  
  MYSQL_OBJ mobj;

  if(!same_type_user_def(vc,x,mysql_type))
    error(vc,"Not a MYSQL object",x);

  mobj = (MYSQL_OBJ)UDVAL(x);
  
  return make_long(vc,mysql_num_fields(mobj->res));
}

VCSI_OBJECT num_rows_mysql(VCSI_CONTEXT vc,
			   VCSI_OBJECT x) {
  
  MYSQL_OBJ mobj;
  
  if(!same_type_user_def(vc,x,mysql_type))
    error(vc,"Not a MYSQL object",x);
  
  mobj = (MYSQL_OBJ)UDVAL(x);
  
  return make_long(vc,mysql_num_rows(mobj->res));
}

VCSI_OBJECT ping_mysql(VCSI_CONTEXT vc,
		       VCSI_OBJECT x) {
  
  MYSQL_OBJ mobj;
  
  if(!same_type_user_def(vc,x,mysql_type))
    error(vc,"Not a MYSQL object",x);
  
  mobj = (MYSQL_OBJ)UDVAL(x);
  
  if(mysql_ping(&(mobj->mysql)) == 0)
    return vc->true;

  return vc->false;
}

VCSI_OBJECT select_db_mysql(VCSI_CONTEXT vc,
			    VCSI_OBJECT x,
			    VCSI_OBJECT y) {
  
  MYSQL_OBJ mobj;

  check_mysql(vc,x);

  if(!TYPEP(y,STRING))
    error(vc,"Not a 'string' to mysql-select-db",y);  
  
  mobj = (MYSQL_OBJ)UDVAL(x);
  
  if(mysql_select_db(&(mobj->mysql),STR(y)) == 0) {
    if(mobj->database)
      FREE(mobj->database);
    mobj->database = strdup(STR(y));
    return vc->true;
  }
  return vc->false;
}

VCSI_OBJECT change_user_mysql(VCSI_CONTEXT vc,
			      VCSI_OBJECT x,
			      VCSI_OBJECT user,
			      VCSI_OBJECT passwd,
			      VCSI_OBJECT database) {
  
  MYSQL_OBJ mobj;

  check_mysql(vc,x);

  if(user && !TYPEP(user,STRING))
    error(vc,"Not a 'string' in user to mysql-change-user",user);  
  
  if(passwd && !TYPEP(passwd,STRING))
    error(vc,"Not a 'string' in passwd to mysql-change-user",passwd);  
  
  if(database && !TYPEP(database,STRING))
    error(vc,"Not a 'string' in database to mysql-change-user",database);  
  
  mobj = (MYSQL_OBJ)UDVAL(x);

  if(user) {
    if(mobj->user) FREE(mobj->user);
    mobj->user = strdup(STR(user));
  }
  if(passwd) {
    if(mobj->passwd) FREE(mobj->passwd);
    mobj->passwd = strdup(STR(passwd));
  }
  if(database) {
    if(mobj->database) FREE(mobj->database);
    mobj->database = strdup(STR(database));
  }

  if(mysql_change_user(&(mobj->mysql),mobj->user,mobj->passwd,mobj->database) == 0)
    return vc->true;

  return vc->false;

}

void module_init(VCSI_CONTEXT vc) {

  mysql_type = make_user_def_type(vc,(char*)module_name,
				  print_mysql,NULL,free_mysql);


  set_int_proc(vc,"make-mysql",PROC0,init_mysql);

  set_int_proc(vc,"mysql?",PROC1,mysqlq);

  set_int_proc(vc,"mysql-connect",PROC5,connect_mysql);
  set_int_proc(vc,"mysql-query",PROC2,query_mysql);
  set_int_proc(vc,"mysql-fetch-row",PROC1,fetch_row_mysql);
  set_int_proc(vc,"mysql-connected?",PROC1,connected_mysql);

  set_int_proc(vc,"mysql-affected-rows",PROC1,affected_rows_mysql);
  set_int_proc(vc,"mysql-insert-id",PROC1,insert_id_mysql);
  set_int_proc(vc,"mysql-num-fields",PROC1,num_fields_mysql);
  set_int_proc(vc,"mysql-num-rows",PROC1,num_rows_mysql);
  set_int_proc(vc,"mysql-ping",PROC1,ping_mysql);

  set_int_proc(vc,"mysql-select-db",PROC2,select_db_mysql);
  set_int_proc(vc,"mysql-change-user",PROC4,change_user_mysql);

  /****************************************************************
   *
   * To Do: 
   * x - mysql_affected_rows (returns the number of rows changed)
   * x - mysql_change_user (changes to new user,passwd, db)
   * x - mysql_eof (determines if last row has been read)
   * x - mysql_insert_id (id of last AUTO_INCREMENT insert)
   * x - mysql_num_fields (returns number of cols in result)
   * x - mysql_num_rows (returns number of rows in result)
   * x - mysql_ping (determines if connection is working)
   * n - mysql_real_escape_string (escapes a string for use in queries)
   * x - mysql_select_db (changes the current database)
   *
   *
   *****************************************************************/


}

#include <vcsi.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "http.h"

const char module_name[] = "http";

enum http_methods {
  GET, POST, HEAD
};

/* HTTP Requets */
typedef struct str_http_request {
  enum http_methods method;

  /* User-Agent */
  INFOSTD_DYN_STR user_agent;

  /* Credentials */
  INFOSTD_DYN_STR username;
  INFOSTD_DYN_STR password;
  INFOSTD_DYN_STR credentials;

  /* From Data (already processed and encoded) */
  INFOSTD_DYN_STR form_data;
  /* Form Data (hashed) */

  /* Cookie Jar */
  
  /* Referer */
  INFOSTD_DYN_STR referer;

  /* Redirect attempts */
  unsigned long redirects;

  /* Timeout */
  unsigned long timeout;

  /* URL and parsed components*/
  INFOSTD_DYN_STR url;
  INFOSTD_DYN_STR protocol;
  INFOSTD_DYN_STR host;
  INFOSTD_DYN_STR path;
  unsigned long port;

  /* Working memory */
  INFOSTD_DYN_STR request;
 
} *HTTP_REQUEST;
#define SIZE_HTTP_REQUEST (sizeof(struct str_http_request))

/* User Defined Type */
VCSI_USER_DEF http_request_type = NULL;
#define HTTP_REQ(x) ((HTTP_REQUEST)(*x).data.userdef.ud_value)

VCSI_OBJECT make_http_request(VCSI_CONTEXT vc) {

  HTTP_REQUEST htreq = NULL;
  VCSI_OBJECT tmp = NULL;

  htreq = MALLOC(SIZE_HTTP_REQUEST);
  memset(htreq,0,SIZE_HTTP_REQUEST);

  htreq->method = GET;
  htreq->user_agent = infostd_dyn_str_init(100);

  htreq->username = infostd_dyn_str_init(100);
  htreq->password = infostd_dyn_str_init(100);
  htreq->credentials = infostd_dyn_str_init(100);

  htreq->form_data = infostd_dyn_str_init(100);

  htreq->referer = infostd_dyn_str_init(100);

  htreq->redirects = 0;

  htreq->timeout = 30;

  htreq->url = infostd_dyn_str_init(100);
  htreq->protocol = infostd_dyn_str_init(100);
  htreq->host = infostd_dyn_str_init(100);
  htreq->path = infostd_dyn_str_init(100);
  htreq->port = 80;

  htreq->request = infostd_dyn_str_init(256);

  tmp = make_user_def(vc,(void*)htreq,http_request_type);

  user_def_set_eval(vc,tmp,PROCOPT,eval_http_request);

  return tmp;
}

void print_http_request(VCSI_CONTEXT vc,
			INFOSTD_DYN_STR res, 
			VCSI_OBJECT obj) {
  
  infostd_dyn_str_printf(vc->res_extra,"#[http-request]");
  
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void mark_http_request(VCSI_CONTEXT vc,
		       VCSI_OBJECT obj, 
		       int flag) {  

  gc_mark(vc,obj,flag);
}

void free_http_request(VCSI_CONTEXT vc,
		       VCSI_OBJECT obj) {

  HTTP_REQUEST htreq;

  if(obj == NULL)
    return;

  htreq = HTTP_REQ(obj);

  infostd_dyn_str_free(htreq->user_agent);
  infostd_dyn_str_free(htreq->username);
  infostd_dyn_str_free(htreq->password);
  infostd_dyn_str_free(htreq->credentials);
  infostd_dyn_str_free(htreq->form_data);
  infostd_dyn_str_free(htreq->referer);
  infostd_dyn_str_free(htreq->url);
  infostd_dyn_str_free(htreq->protocol);
  infostd_dyn_str_free(htreq->host);
  infostd_dyn_str_free(htreq->path);
  infostd_dyn_str_free(htreq->request);

  FREE(htreq);
}

/* (htreq 'agent "blah") */
VCSI_OBJECT eval_http_request(VCSI_CONTEXT vc,
			      VCSI_OBJECT args, 
			      int length) {

  VCSI_OBJECT htreq, method, arg1, arg2;
  char *mstr;

  method = arg1 = arg2 = NULL;

  if(length < 2 || length > 4)
    error(vc,"invalid number of arguments to http-request",args);

  htreq = car(vc,args);

  if(!same_type_user_def(vc,htreq,http_request_type))
    error(vc,"object passed to http-request is of the wrong type",htreq);

  method = cadr(vc,args);

  if(length > 2)
    arg1 = caddr(vc,args);
  if(length > 3)
    arg2 = cadddr(vc,args);

  if(!(TYPEP(method,SYMBOL) || TYPEP(method,STRING)))
    error(vc,"invalid method to http-request",method);

  mstr = (TYPEP(method,SYMBOL)) ? SNAME(method) : STR(method);

  // Parse the method
  if(!strcasecmp(mstr,"agent")) {
    if(arg1) {
      check_arg_type(vc,arg1,STRING,"http-request");
      infostd_dyn_str_strcpy(HTTP_REQ(htreq)->user_agent,STR(arg1));
    } else {
      return make_string(vc,HTTP_REQ(htreq)->user_agent->buff);
    }
  }
  
  return vc->true;
}

/* HTTP Response */


/* Functions */
VCSI_OBJECT http_simple_get(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x) {

  INFOSTD_DYN_STR http_host;
  INFOSTD_DYN_STR http_path;
  INFOSTD_DYN_STR http_request;
  INFOSTD_DYN_STR http_response;
  int s, len, i, found = 0;
  char c, *ln, *ln2;
  struct hostent *hp;
  struct sockaddr_in name;
  unsigned long slen;

  check_arg_type(vc,x,STRING,"http-simple-get");


  http_request = infostd_dyn_str_init(100);
  http_response = infostd_dyn_str_init(100); 
  http_host = infostd_dyn_str_init(100);
  http_path = infostd_dyn_str_init(100);

  infostd_dyn_str_clear(http_host);
  infostd_dyn_str_clear(http_path);

  ln = STR(x);
  if(strncasecmp(STR(x),"http://",7) == 0) {
    ln += 7;
  }
  
  len = strlen(ln);
  for(i=0;i<len;i++) {
    c = ln[i];

    if(c == '/') found = 1;

    if(found) {
      infostd_dyn_str_addchar(http_path,c);
    } else {
      infostd_dyn_str_addchar(http_host,c);
    }
  }

  if(!infostd_dyn_str_length(http_path)) 
    infostd_dyn_str_addchar(http_path,'/');

  //printf("HOST: %s\nPATH: %s\n",http_host->buff,http_path->buff);
  
  infostd_dyn_str_clear(http_request);
  infostd_dyn_str_printf(http_request,
			 "GET %s HTTP/1.0\nHost: %s\n\n",
			 http_path->buff, http_host->buff);
  
  if((hp = gethostbyname(http_host->buff)) == NULL)
    return vc->false;
  
  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return vc->false;

  memset(&name, 0, sizeof(struct sockaddr_in));
  name.sin_family = AF_INET;
  name.sin_port = htons(80);
  memcpy(&name.sin_addr, hp->h_addr_list[0], hp->h_length);
  len = sizeof(struct sockaddr_in);

  if(connect(s, (struct sockaddr *) &name, len) < 0)
    return vc->false;
  
  slen = infostd_dyn_str_length(http_request);

  send(s,http_request->buff,slen,0);
  
  infostd_dyn_str_clear(http_response);

  while(recv(s,&c,1,0) > 0) {
    infostd_dyn_str_addchar(http_response,c);
  }
  shutdown(s,2);

  ln = http_response->buff;
  if(ln2 = strstr(ln,"\n\n"))
    ln = ln2 + 2;
  else if(ln2 = strstr(ln,"\r\n\r\n"))
    ln = ln2 + 4;

  infostd_dyn_str_free(http_request);
  infostd_dyn_str_free( http_response);
  infostd_dyn_str_free(http_host);
  infostd_dyn_str_free(http_path);

  return make_string(vc,ln);
}

/* Module Init */
void module_init(VCSI_CONTEXT vc) {  

  http_request_type = make_user_def_type(vc, "http-request", print_http_request, 
					 mark_http_request, free_http_request);
  set_int_proc(vc,"make-http-request",PROC0,make_http_request);
  

  set_int_proc(vc,"http-simple-get",PROC1,http_simple_get);
}

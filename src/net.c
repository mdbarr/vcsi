#include "vcsi.h"
/* Networking Code */
void net_init(VCSI_CONTEXT vc) {
  set_int_proc(vc,"open-tcp-socket",PROC2,open_tcp_socket);   
}

VCSI_OBJECT open_tcp_socket(VCSI_CONTEXT vc, 
			    VCSI_OBJECT host, 
			    VCSI_OBJECT port) {
  VCSI_OBJECT tmp;
  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  char* path;
   
  if(!TYPEP(host,STRING))
    return error(vc,"object passed to open-tcp-socket is the wrong type",host);
  if(!TYPEP(port,LNGNUM))
    return error(vc,"object passed to open-tcp-socket is the wrong type",port);

  if((hp = gethostbyname(STR(host))) == NULL)
    /*return error(vc,"unknown host",host);*/
    return vc->false;

  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return error(vc,"unable to create socket",host);

  memset(&name, 0, sizeof(struct sockaddr_in));
  name.sin_family = AF_INET;
  name.sin_port = htons(LNGN(port));
  memcpy(&name.sin_addr, hp->h_addr_list[0], hp->h_length);
  len = sizeof(struct sockaddr_in);

  if(connect(s, (struct sockaddr *) &name, len) < 0)
    /*return error(vc,"connection refused",host);*/
    return vc->false;

  /*val = fcntl(s,F_GETFL,0);
    fcntl(s,F_SETFL, val|O_NONBLOCK);

    tmp = alloc_obj(vc,SOCKTCP);
    SOCK(tmp) = s;
    SOCKN(tmp) = 
  */
  path = (char*)MALLOC(strlen(STR(host))+8);
  sprintf(path,"%s:%ld",STR(host),LNGN(port));
  tmp = make_port_fp(vc,fdopen(s,"w+"),path,PORTB);
  free(path);
  return tmp;
}

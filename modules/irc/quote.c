/* A www.bash.org IRC Quote getter */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <vcsi.h>

const char module_name[] = "quote";

char* quote_extract_text(char* in, 
			 char* buffer, 
			 int size) {

  int i, d, c, inb;
  char *ln;
  char l;

  memset(buffer,0,size);

  for(i=0,c=0,inb=0;in[i]!=0&&i<size;i++) {
    if(in[i] == '<')
      inb = 1;
    
    if(!inb) {
      if(in[i] == '&') {
	for(d=i;in[d]!=';' && in[d] != 0 && d<size;d++);

	ln = in+i;

	if(!strncmp(ln,"&lt",d-i))
	  l = '<';
	else if(!strncmp(ln,"&gt",d-i))
	  l = '>';
	else
	  l = ' ';

	buffer[c] = l;
	c++;	
	i = d;
      } else {
	buffer[c] = in[i];
	c++;
      }
    }
    
    if(inb && in[i] == '>')
      inb = 0;
  }
  return buffer;
}

VCSI_OBJECT get_quote(VCSI_CONTEXT vc,
		      VCSI_OBJECT num) {

  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  char url[256];
  char buffer[4096];
  char tmp[4096];
  char *ln;
  char c;
  int i, n, end = 0, inq = 0;
  unsigned long number;
  VCSI_OBJECT list = NULL;

  check_arg_type(vc,num,LNGNUM,"get-quote");

  number = LNGN(num);
  
  if((hp = gethostbyname("www.bash.org")) == NULL)
    return list;
  
  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return list;

  memset(&name, 0, sizeof(struct sockaddr_in));
  name.sin_family = AF_INET;
  name.sin_port = htons(80);
  memcpy(&name.sin_addr, hp->h_addr_list[0], hp->h_length);
  len = sizeof(struct sockaddr_in);

  if(connect(s, (struct sockaddr *) &name, len) < 0)
    return list;
  
  snprintf(url,255,"GET http://www.bash.org/?%d HTTP/1.0\n\n",number);
  send(s,url,strlen(url),0);

  while(!end) {
    memset(buffer,0,4096);

    for(i=0,c=0;i<4095;i++) {
      n = recv(s,&c,1,0);
      if(n<1) {
	end=1;
	break;
      }      
      if(c=='\r')
	continue;

      if(c=='\n')
	break;      
      buffer[i] = c;
    }

    if(inq)
      list = cons(vc,make_string(vc,quote_extract_text(buffer,tmp,4095)),list);
    
    if(!strncmp(buffer,"<p class=\"quote\">",17)) {
      inq=1;
      ln = strstr(buffer,"<p class=\"qt\">");
      if(ln) {
	ln+=14;	
	list = cons(vc,make_string(vc,quote_extract_text(ln,tmp,4095)),list);
      }
    }

    if(inq && strlen(buffer) > 3) {
      ln = buffer + (strlen(buffer)-4);
      if(!strcmp(ln,"</p>"))
	inq = 0;
    }
  }
  shutdown(s,2);
  
  return vcsi_list_reverse(vc,list);
}

void module_init(VCSI_CONTEXT vc) { 
  set_int_proc(vc,"get-quote",PROC1,get_quote);
}

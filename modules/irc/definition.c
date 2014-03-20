/* A www.dictionary.com definition fetcher */
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

const char module_name[] = "definition";

char* definition_extract_text(char* in, 
			 char* buffer, 
			 int size) {

  int i, d, c, inb, ins;
  char *ln;
  char l;

  memset(buffer,0,size);

  for(i=0,c=0,inb=0,ins=0;in[i]!=0&&i<size;i++) {
    if(in[i] == '<')
      inb = 1;
    if(in[i] == '[')
      ins = 1;
    
    if(!inb && !ins) {
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

    if(ins && in[i] == ']')
      ins = 0;
  }
  return buffer;
}

VCSI_OBJECT get_definition(VCSI_CONTEXT vc,
			   VCSI_OBJECT x) {

  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  char url[256];
  char buffer[4096];
  char tmp[4096];
  char *ln, *ln2, *word;
  char c;
  int i, n, count, end = 0, inq = 0;
  unsigned long number;
  VCSI_OBJECT list = NULL;

  check_arg_type(vc,x,STRING,"get-definition");
  word = STR(x);
   
  if((hp = gethostbyname("dictionary.reference.com")) == NULL)
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
  
  snprintf(url,255,"GET http://dictionary.referencecom/search?q=%s HTTP/1.0\n\n",word);
  send(s,url,strlen(url),0);

  count = 0;
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
    
    if((ln=strstr(buffer,"<DL><DD>"))) {
      ln+=8;
      if((ln2=strstr(ln,"</DD></DL>"))) {
	*ln2 = '\0';

	for(;*ln==' '&&*ln!='\0';ln+=1);

	list = cons(vc,make_string(vc,
				   definition_extract_text(ln,tmp,4095)),
		    list);
	count++;
      }
    }    

    if((ln=strstr(buffer,"<LI>"))) {
      ln+=4;
      if((ln2=strstr(ln,"</LI>"))) {

	for(;*ln==' '&&*ln!='\0';ln+=1);
	
	list = cons(vc,make_string(vc,
				   definition_extract_text(ln,tmp,4095)),
		    list);
	count++;
      }
    }

    if(count > 5)
      end=1;
    
  }
  shutdown(s,2);
  
  return vcsi_list_reverse(vc,list);
}

VCSI_OBJECT get_synonym(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {

  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  char url[256];
  char buffer[16384];
  char tmp[16384];
  char *ln, *ln2, *word;
  char c;
  int i, n, end = 0, inq = 0;
  unsigned long number;
  VCSI_OBJECT list = NULL;

  check_arg_type(vc,x,STRING,"get-synonym");
  word = STR(x);
   
  if((hp = gethostbyname("thesaurus.reference.com")) == NULL)
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
  
  snprintf(url,255,"GET http://thesaurus.reference.com/search?q=%s HTTP/1.0\n\n",word);
  send(s,url,strlen(url),0);

  while(!end) {
    memset(buffer,0,16384);

    for(i=0,c=0;i<16383 && !end;) {
      n = recv(s,&c,1,0);

      if(n<1) {
	end=1;
	break;
      } else if(c=='\r')
	continue;
      else if(c=='\n')
	break;
      else {
	buffer[i] = c;
	i++;
      }
    }
    
    if((ln=strstr(buffer,"<STRONG><A HREF"))) {
      if((ln=strstr(ln,"\">"))) {
	ln+=2;

	if((ln2=strstr(ln,"</A>"))) {
	  *ln2 = '\0';
	  
	  for(;*ln==' '&&*ln!='\0';ln+=1);
	  
	  list = cons(vc,make_string(vc,
				     definition_extract_text(ln,tmp,16383)),
		      list);
	}
      }
    }

    if((ln=strstr(buffer,"<b>Synonyms:</b>&nbsp;&nbsp;</td><td>"))) {
      ln+=37;

      if((ln2=strstr(ln,"</td></tr>"))) {
	*ln2 = '\0';
	
	for(;*ln==' '&&*ln!='\0';ln+=1);
	
	list = string_tokenize(vc, make_string(vc,", "),
			       make_string(vc,
					   definition_extract_text(ln,
								   tmp,
								   16383)));
	end=1;
      }      
    }    

  }
  shutdown(s,2);
  
  return list;
}

VCSI_OBJECT get_antonym(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {

  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  char url[256];
  char buffer[16384];
  char tmp[16384];
  char *ln, *ln2, *word;
  char c;
  int i, n, end = 0, inq = 0;
  unsigned long number;
  VCSI_OBJECT list = NULL;

  check_arg_type(vc,x,STRING,"get-synonym");
  word = STR(x);
   
  if((hp = gethostbyname("thesaurus.reference.com")) == NULL)
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
  
  snprintf(url,255,"GET http://thesaurus.reference.com/search?q=%s HTTP/1.0\n\n",word);
  send(s,url,strlen(url),0);

  while(!end) {
    memset(buffer,0,16384);

    for(i=0,c=0;i<16383 && !end;) {
      n = recv(s,&c,1,0);

      if(n<1) {
	end=1;
	break;
      } else if(c=='\r')
	continue;
      else if(c=='\n')
	break;
      else {
	buffer[i] = c;
	i++;
      }
    }
    
    if((ln=strstr(buffer,"<b>Antonyms:</b>&nbsp;&nbsp;</td><td>"))) {
      ln+=37;
      if((ln2=strstr(ln,"</td></tr>"))) {
	*ln2 = '\0';
	
	for(;*ln==' '&&*ln!='\0';ln+=1);
	
	list = string_tokenize(vc, make_string(vc,", "),
			       make_string(vc,
					   definition_extract_text(ln,
								   tmp,
								   16383)));

	end=1;
      }
    }    

  }
  shutdown(s,2);
  
  return list;
}

void module_init(VCSI_CONTEXT vc) { 
  set_int_proc(vc,"get-definition",PROC1,get_definition);
  set_int_proc(vc,"get-synonym",PROC1,get_synonym);
  set_int_proc(vc,"get-antonym",PROC1,get_antonym);
}

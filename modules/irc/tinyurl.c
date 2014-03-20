#include <vcsi.h>
#include <sys/types.h>
#include <sys/socket.h>

INFOSTD_DYN_STR tinyurl_enc;
INFOSTD_DYN_STR tinyurl_msg;
INFOSTD_DYN_STR tinyurl_content;

INFOSTD_DYN_STR tinyurl_encode(char* s,
			       INFOSTD_DYN_STR enc) {
  
  unsigned long i, len;
  char buf[10];
  char c;

  len = strlen(s);

  infostd_dyn_str_clear(enc);
  for(i=0;i<len;i++) {
    c = s[i];
    switch (c) {
    case ' ':
      infostd_dyn_str_addchar(enc, '+');
      break;
    case '&':
    case '+':
    case '"':
    case '\'':
    case '(':
    case ')':
      memset(buf,0,10);
      snprintf(buf,10,"%%%2.2X",c);
      infostd_dyn_str_strcat(enc, buf);
      break;
  default:
    infostd_dyn_str_addchar(enc, c);
    break;
    }
  }
  return tinyurl_enc;
}

VCSI_OBJECT tinyurl_get(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  int s, len;
  char c, *ln, *ln2;
  struct hostent *hp;
  struct sockaddr_in name;
  unsigned long dlen, slen;

  check_arg_type(vc,x,STRING,"tinyurl");
  tinyurl_encode(STR(x),tinyurl_enc);
  dlen = infostd_dyn_str_length(tinyurl_enc) + 4;
  
  infostd_dyn_str_clear(tinyurl_msg);
  infostd_dyn_str_printf(tinyurl_msg,
			 "POST /create.php HTTP/1.0\n"
			 "Host: tinyurl.com\n"
			 "Content-type: application/x-www-form-urlencoded\n"
			 "Content-length: %d\n"
			 "\n"
			 "url=%s\n\n",
			 dlen, tinyurl_enc->buff);

  if((hp = gethostbyname("tinyurl.com")) == NULL)
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
  
  slen = infostd_dyn_str_length(tinyurl_msg);

  send(s,tinyurl_msg->buff,slen,0);
  
  infostd_dyn_str_clear(tinyurl_content);

  while(recv(s,&c,1,0) > 0) {
    infostd_dyn_str_addchar(tinyurl_content,c);
  }
  shutdown(s,2);

  ln = strstr(tinyurl_content->buff, "<b>http://tiny");
  if(!ln)
    return vc->false;
  ln += 3;

  ln2 = strstr(ln,"</b>");
  if(!ln2)
    return vc->false;
  *ln2 = '\0';
  
  return make_string(vc,ln);
}

void module_init(VCSI_CONTEXT vc) {

  tinyurl_enc = infostd_dyn_str_init(100);
  tinyurl_msg = infostd_dyn_str_init(100);
  tinyurl_content = infostd_dyn_str_init(100);

  set_int_proc(vc,"tinyurl",PROC1,tinyurl_get);
}

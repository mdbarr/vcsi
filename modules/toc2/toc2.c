/* TOC Connections Module */
#include "toc2.h"

/* TODO
 * Pause, Quoting, Buddies 
 */

const char module_name[] = "toc2";

#define TOC_DEBUG 1

/**** TOC Buddy Functions */
TOC_BUDDY toc_buddy_init(char* name) {
  
  TOC_BUDDY tocb;
  
  tocb = (TOC_BUDDY)MALLOC(SIZE_TOC_BUDDY);
  memset(tocb,0,SIZE_TOC_BUDDY);

  strncpy(tocb->name,name,255);
  strncpy(tocb->nice_name,name,255);

  if(TOC_DEBUG) printf("TOC BUDDY CREATED - %s\n",name);

  return tocb;
}

TOC_BUDDY toc_buddy_update(TOC_BUDDY tocb, 
			   char* nice_name,
			   unsigned char online,
			   int evil,
			   time_t signon,
			   int idle,
			   unsigned char away) {

  if(!tocb)
    return NULL;

  if(TOC_DEBUG) 
    printf("TOC BUDDY UPDATE %s ONLINE(%d) EVIL(%d) SIGNON(%ld) IDLE(%d)\n",
	   nice_name,online,evil,signon,idle);

  memset(tocb->nice_name,0,256);
  strcpy(tocb->nice_name,nice_name);

  tocb->online = online;
  tocb->evil = evil;
  tocb->signon = signon;
  tocb->idle = idle;
  tocb->away = away;

  return tocb;
}

TOC_BUDDY_GROUP toc_buddy_group_init(char* name) {
  TOC_BUDDY_GROUP tocg;
  
  tocg = (TOC_BUDDY_GROUP)MALLOC(SIZE_TOC_BUDDY_GROUP);
  memset(tocg,0,SIZE_TOC_BUDDY_GROUP);

  strncpy(tocg->name,name,255);

  if(TOC_DEBUG) printf("TOC BUDDY GROUP CREATED - %s\n",name);

  return tocg;
}

TOC_BUDDY toc_buddy_group_add(TOC_BUDDY_GROUP tocg,
			      TOC_BUDDY tocb) {

  TOC_BUDDY tmp;

  if(!tocg)
    return NULL;

  if(tocg->buddies) {
    /* Find last */
    for(tmp=tocg->buddies;tmp&&tmp->next;tmp=tmp->next);
    tmp->next = tocb;
  } else
    tocg->buddies = tocb;
  
  return tocb;
}

TOC_BUDDY_GROUP toc_buddy_group_add_group(TOC_CONN tocc,
					  TOC_BUDDY_GROUP tocg) {

  TOC_BUDDY_GROUP tmp;

  if(tocc->buddies) {
    /* Find last */
    for(tmp=tocc->buddies;tmp&&tmp->next;tmp=tmp->next);
    tmp->next = tocg;
  } else
    tocc->buddies = tocg;
  
  return tocg;
}

void toc_buddy_group_remove_buddy(TOC_BUDDY_GROUP tocg,
				  TOC_BUDDY tocb) {

  TOC_BUDDY tmp;
  if(tocg->buddies == tocb) {
    tocg->buddies = tocb->next;
  } else {
    for(tmp=tocg->buddies;tmp&&tmp->next!=tocb;tmp=tmp->next);
    if(tmp)
      tmp->next = tocb->next;
  }
}

void toc_buddy_group_remove_all(TOC_BUDDY_GROUP tocg) {
			    
  TOC_BUDDY tmp, next;
  tmp = next = tocg->buddies;
  while(next) {
    next = tmp->next;
    FREE(tmp);
  }
}

void toc_buddy_group_remove(TOC_BUDDY_GROUP tocg) {
			    
  TOC_BUDDY_GROUP tmp, next;
  tmp = next = tocg;
  while(next) {
    next = tmp->next;
    toc_buddy_group_remove_all(tmp);
    FREE(tmp);
  }
}

TOC_BUDDY_GROUP toc_buddy_group_find_group(TOC_CONN tocc,
					   char* name) {
  TOC_BUDDY_GROUP tmp;

  if(tocc->buddies) {    
    for(tmp=tocc->buddies;tmp;tmp=tmp->next) {
      if(!strcasecmp(tmp->name,name))
	return tmp;
    }
  }
 
  return NULL;
}

TOC_BUDDY toc_buddy_group_find(TOC_BUDDY_GROUP tocg,
			       char* name) {

  TOC_BUDDY tmp;
  if(tocg->buddies) {
    for(tmp=tocg->buddies;tmp;tmp=tmp->next)
      if(!strcasecmp(tmp->name,toc_normalize_username(name)))
	return tmp;
  }  
  return NULL;
}

TOC_BUDDY toc_buddy_group_find_all(TOC_CONN tocc,
				   char* name) {

  TOC_BUDDY_GROUP tmp;
  TOC_BUDDY tocb;

  if(tocc->buddies) {
    for(tmp=tocc->buddies;tmp;tmp=tmp->next) 
      if((tocb=toc_buddy_group_find(tmp,name)))
	return tocb;    
  }
  return NULL;
}

void toc_config_parse(TOC_CONN tocc, 
		      char* config) {
  
  TOC_BUDDY_GROUP tocg = NULL;
  TOC_BUDDY tocb = NULL;
  char *str, *tmp;

  /* Kill the old config */
  if(tocc->buddies) {
    toc_buddy_group_remove(tocc->buddies);
    tocc->buddies = NULL;
  }

  if(tocc->permit) {
    toc_buddy_group_remove(tocc->permit);
    tocc->permit = NULL;
  }

  if(tocc->deny) {
    toc_buddy_group_remove(tocc->deny);
    tocc->deny = NULL;
  }

  tocc->mode = 1;

  /* Parse the new config */
  for(str=strtok(config,"\n");str;str=strtok(NULL,"\n")) {
    tmp = str+2;

    switch (str[0]) {
    case 'g':
      /* Create the new group */
      tocg = toc_buddy_group_add_group(tocc,toc_buddy_group_init(tmp));
      break;
    case 'b':
      /* Create a group if none */
      if(!tocg)
	tocg = toc_buddy_group_add_group(tocc,toc_buddy_group_init("Buddies"));
      /* Add the buddy to the group */
      tocb = toc_buddy_group_add(tocg,toc_buddy_init(tmp));
      /* Add the buddy to the TOC Buddy list */
      toc_send_printf(tocc,"toc_add_buddy %s",toc_normalize_username(tmp));
      break;
    case 'p':
      /* Create the permit list if need */
      if(!tocc->permit)
	tocc->permit = toc_buddy_group_init("Permit List");
      /* Add the person to the list */
      tocb = toc_buddy_group_add(tocc->permit,toc_buddy_init(tmp));
      /* Add the person to the TOC Permit list */
      toc_send_printf(tocc,"toc2_add_permit %s",toc_normalize_username(tmp));
      break;
    case 'd':
      /* Create the deny list if need */
      if(!tocc->deny)
	tocc->deny = toc_buddy_group_init("Deny List");
      /* Add the person to the list */
      tocb = toc_buddy_group_add(tocc->deny,toc_buddy_init(tmp));
      /* Add the person to the TOC Deny list */
      toc_send_printf(tocc,"toc2_add_deny %s",toc_normalize_username(tmp));
      break;
    case 'm':
      tocc->mode = atol(tmp);
      break;
    }
  }
}

void toc_config_save(TOC_CONN tocc) {

  char data[2048];

  TOC_BUDDY_GROUP tmp;
  TOC_BUDDY  tmp2;

  strcpy(data,"m 1\n");

  if(tocc->buddies) {    
    for(tmp=tocc->buddies;tmp;tmp=tmp->next) {
      if(tmp->buddies) {
	if(!strcmp(tmp->name,"Permit List")) {
	  for(tmp2=tmp->buddies;tmp2;tmp2=tmp2->next) {
	    strncat(data,"p ",2048);
	    strncat(data,tmp2->name,2048);
	    strncat(data,"\n",2048);
	  }
	} else if(!strcmp(tmp->name,"Deny List")) {
	  for(tmp2=tmp->buddies;tmp2;tmp2=tmp2->next) {
	    strncat(data,"d ",2048);
	    strncat(data,tmp2->name,2048);
	    strncat(data,"\n",2048);
	  }
	} else {
	  strncat(data,"g ",2048);
	  strncat(data,tmp->name,2048);
	  strncat(data,"\n",2048);
	  for(tmp2=tmp->buddies;tmp2;tmp2=tmp2->next) {
	    strncat(data,"b ",2048);
	    strncat(data,tmp2->name,2048);
	    strncat(data,"\n",2048);
	  }
	}
      }
    }
  }
  
  printf("DATA: \n%s\n",data);
  toc_send_printf(tocc,"toc2_set_config \"%s\"",data);
}

TOC_BUDDY toc_update_buddy_parse(TOC_CONN tocc,
				 char* update) {
  
  TOC_BUDDY tocb;
  char *name, *ln;
  unsigned char online = 0;
  int evil = 0;
  time_t signon = 0;
  int idle = 0;
  unsigned char away = 0;
  
  name = update;
  if((ln=strchr(name,':'))) {
    *ln='\0';
    ln++;
    if(*ln == 'T')
      online = 1;
    if((ln=strchr(ln,':'))) {
      *ln='\0';
      ln++;    
      evil=atol(ln);
      if((ln=strchr(ln,':'))) {
	*ln='\0';
	ln++;    
	signon = atol(ln);
	if((ln=strchr(ln,':'))) {
	  *ln='\0';
	  ln++; 
	  idle = atol(ln);
	  if((ln=strchr(ln,':'))) {
	    *ln='\0';
	    ln++; 
	    if(ln[2] == 'U')
	      away = 1;
	    if((tocb=toc_buddy_group_find_all(tocc,name))) {
	      toc_buddy_update(tocb,name,online,evil,signon,idle,away);
	      return tocb;
	    }
	  }
	}
      }
    }
  }
  return NULL;
}

/**** TOC Event Functions ****/
TOC_EVENT toc_read_event(TOC_CONN tocc) {
  TOC_EVENT toce;
  int n, type, seq, len;


  /* Clear the recv buffer */
  memset(tocc->recv_buf,0,2064);
  
  /* Create the event */
  toce = MALLOC(SIZE_TOC_EVENT);
  memset(toce,0,SIZE_TOC_EVENT);

  /* Read a FLAP Header */
  if((n = read(tocc->skt,tocc->recv_buf,6)) < 0) {
    toce->type = TOC_EV_ERROR;
    strcpy(toce->data,"connection closed.");
    toce->length = strlen(toce->data);
    tocc->connected = 0;
    close(tocc->skt);
    if(TOC_DEBUG) printf("TOC_EVENT: disconnected!");
    return toce;
  }

  /* Parse the header */
  type = tocc->recv_buf[1];
  seq = ntohs(* ((unsigned short *)(&tocc->recv_buf[2])));
  len = ntohs(* ((unsigned short *)(&tocc->recv_buf[4])));

  if(TOC_DEBUG)
    printf("TOC RECV HEADER(%d) TYPE(%d) SEQ(%d) LEN(%d)\n",n,type,seq,len);

  /* Clear the recv buffer */
  memset(tocc->recv_buf,0,2064);
  
  /* Read a FLAP Payload */
  n = read(tocc->skt,tocc->recv_buf,len);
  
  if(type == TOC_TYPE_SIGNON) {
    /* A SIGNON Packet */
    if(TOC_DEBUG) printf("TOC_EVENT: SIGNON packet received\n");
    if(len != 4 || tocc->recv_buf[0] != '\0' || tocc->recv_buf[1] != '\0' || 
       tocc->recv_buf[2] != '\0' || tocc->recv_buf[3] != '\1') {
      toce->type = TOC_EV_ERROR;
      sprintf(toce->data,"Signon failed. TOC Server didn't answer properly.");
    } else
      toce->type = TOC_EV_SIGNON_REQUEST;
  } else if(type == TOC_TYPE_DATA) {
    if(TOC_DEBUG) printf("TOC_EVENT: DATA packet received\n");
    /* A Data Packet */
    if(!strncmp(tocc->recv_buf,"SIGN_ON:",8)) {
      /* Sign on event */
      toce->type = TOC_EV_SIGNON;
      toce->length = len-8;
      strncpy(toce->data,tocc->recv_buf+8,toce->length);
    } else if(!strncmp(tocc->recv_buf,"CONFIG2:",8)) {
      /* Config event */
      toce->type = TOC_EV_CONFIG;
      toce->length = len-8;
      strncpy(toce->data,tocc->recv_buf+8,toce->length);
    } else if(!strncmp(tocc->recv_buf,"NICK:",5)) {
      /* Nick event */
      toce->type = TOC_EV_NICK;
      toce->length = len-5;
      strncpy(toce->data,tocc->recv_buf+5,toce->length);
    } else if(!strncmp(tocc->recv_buf,"IM_IN2:",7)) {
      /* Instant message event */
      toce->type = TOC_EV_IM;
      toce->length = len-7;
      strncpy(toce->data,tocc->recv_buf+7,toce->length);
    } else if(!strncmp(tocc->recv_buf,"UPDATE_BUDDY2:",14)) {
      /* Update buddy event */
      toce->type = TOC_EV_UPDATE_BUDDY;
      toce->length = len-14;
      strncpy(toce->data,tocc->recv_buf+14,toce->length);
    } else if(!strncmp(tocc->recv_buf,"ERROR:",6)) {
      toce->type = TOC_EV_ERROR;
      toce->length = len-6;
      strncpy(toce->data,tocc->recv_buf+6,toce->length);
    } else {
      toce->type = TOC_EV_UNKNOWN;
      toce->length = len;
      strncpy(toce->data,tocc->recv_buf,len);    
    }
  } else if(type == TOC_TYPE_KEEPALIVE) {
    /* Keep alive packet */
    toce->type = TOC_EV_KEEPALIVE;
  }

  if(TOC_DEBUG)
    printf("EVENT TYPE(%d) DATA(%d): %s\n",toce->type,toce->length,toce->data);
  
  return toce;
}

VCSI_OBJECT toc_handle_event(VCSI_CONTEXT vc,
			     TOC_CONN tocc,
			     TOC_EVENT toce) {

  TOC_BUDDY tocb;
  VCSI_OBJECT func = NULL;
  VCSI_OBJECT ret = vc->true;
  char *x,*y,*z;

  switch(toce->type) {

  case TOC_EV_SIGNON:
    if(TOC_DEBUG) printf("* TOC_EVENT SIGNON\n");
    break;
  case TOC_EV_CONFIG:
    if(TOC_DEBUG) printf("* TOC_EVENT CONFIG\n");
    toc_config_parse(tocc,toce->data);
    break;
  case TOC_EV_NICK:
    if(TOC_DEBUG) printf("* TOC_EVENT NICK\n");
    /* Update our stored username */
    strcpy(tocc->username,toce->data);
    break;    
  case TOC_EV_IM:
    if(TOC_DEBUG) printf("* TOC_EVENT IM\n");

    if(tocc->on_im) {
      x=toce->data;
      if((y=strchr(x,':'))) {
	*y='\0';
	y++;
	if((z=strchr(y,':'))) {
	  *z='\0';
	  z++;
	  func = make_list(vc,4,tocc->on_im,
			   make_string(vc,x),
			   make_string(vc,(char*)toc_strip_html(z)),
			   (*y=='T') ? vc->true : vc->false);
	}
      }
    }

    break;
  case TOC_EV_UPDATE_BUDDY:
    if(TOC_DEBUG) printf("* TOC_EVENT UPDATE_BUDDY\n");
    tocb = toc_update_buddy_parse(tocc,toce->data);

    if(tocb && tocc->on_update_buddy) {
      func = make_list(vc,7,tocc->on_update_buddy,
		       make_string(vc,tocb->nice_name),
		       (tocb->online) ? vc->true : vc->false,
		       make_long(vc,tocb->evil),
		       make_long(vc,tocb->signon),
		       make_long(vc,tocb->idle),
		       (tocb->away) ? vc->true : vc->false);
    }
    break;
  case TOC_EV_ERROR:
    if(TOC_DEBUG) printf("* TOC_EVENT KEEPALIVE\n");
    if(tocc->on_error)
      func = make_list(vc,2,tocc->on_error,make_string(vc,toce->data));
    break;
  case TOC_EV_KEEPALIVE:
    if(TOC_DEBUG) printf("* TOC_EVENT KEEPALIVE\n");
    break;
  default:
    break;
  }
  if(func)
    ret=eval(vc,func,vc->the_global_env);

  return ret;
}

/***** TOC Internal Functions *****/
const char* toc_normalize_username(char* name) {
  static char tmp[256];
  int i,c,len;

  memset(tmp,0,256);
  
  len = strlen(name);
  
  if(TOC_DEBUG) printf("USERNAME NORMALIZE: %s -> ",name);
  
  for(i=0,c=0;i<len;i++) {
    if(isalnum(name[i])) {
      tmp[c] = tolower(name[i]);
      c++;
    }
  }
  
  if(TOC_DEBUG) printf("%s\n",tmp);
  return tmp;
}

const char* toc_strip_html(char* text) {

  static char data[4096];

  unsigned long i, j;
  int in_tag = 0;

  memset(data,0,4096);

  for(i=0,j=0;text[i]!=0 && j<4095;i++) {
    if(!in_tag) {
      if(text[i] == '<')
	in_tag = 1;
      else {
	data[j] = text[i];
	j++;
      }
    } else if(text[i] == '>')
      in_tag = 0;
  }
  
  return data;
}

unsigned long toc_code(char s, char p) {

  unsigned long sn, pw, a, b, c;

  sn = (int)s - 96;
  pw = (int)p - 96;

  a = sn * 7696 + 738816;
  b = sn * 746512;
  c = pw * a;
  
  return c - a +b + 71665152;
}

void toc_roast_password(TOC_CONN tocc) {
  char *roast = TOC_ROAST;
  int x, pos = 2;

  strcpy(tocc->tmp_buf,tocc->passwd);

  if(TOC_DEBUG) printf("PASSWD ROAST: %s -> ",tocc->tmp_buf);
  
  memset(tocc->passwd,0,256);
  strcpy(tocc->passwd,"0x");

  for(x = 0;x < 150 && tocc->tmp_buf[x]; x++)
    pos += sprintf(&tocc->passwd[pos],"%02x",
		   tocc->tmp_buf[x]^roast[x%strlen(roast)]);

  tocc->passwd[pos] = '\0';	

  if(TOC_DEBUG) printf("%s\n",tocc->passwd);
}

unsigned char* toc_header_fill(TOC_CONN tocc,
			       unsigned char type,
			       int data_length) {

  /* Clear the send bufer */
  memset(tocc->send_buf,0,2064);
  
  /* Increment the sequence number */
  tocc->sequence++;
    
  /* Create the header */

  /* Magic number */
  tocc->send_buf[0] = '*';  
  /* Frame type */
  tocc->send_buf[1] = type; 
  /* Sequence number in NBO */ 
  tocc->send_buf[2] = (unsigned char)tocc->sequence/256;
  tocc->send_buf[3] = (unsigned char)tocc->sequence%256;
  /* Length in NBO */
  tocc->send_buf[4] = (unsigned char)data_length/256;
  tocc->send_buf[5] = (unsigned char)data_length%256;

  tocc->send_len = 6;

  return tocc->send_buf+6;
}

unsigned char* toc_header_fill_signon(TOC_CONN tocc) {
  
  char* data;
  size_t len;
  
  len = strlen(tocc->username);

  data = tocc->send_buf + TOC_HDR_LEN;
  
  data[0] = '\0';
  data[1] = '\0';
  data[2] = '\0';
  data[3] = '\001';
  data[4] = '\0';
  data[5] = '\001';
  data[6] = len/256;
  data[7] = len%256;
  strcat(data+8,tocc->username);

  tocc->send_len += 8 + len;

  return tocc->send_buf + tocc->send_len;
}

int toc_send(TOC_CONN tocc) {
  int i;

  int type,seq,len;
  type = tocc->send_buf[1];
  seq = ntohs(* ((unsigned short *)(&tocc->send_buf[2])));
  len = ntohs(* ((unsigned short *)(&tocc->send_buf[4])));

  if(TOC_DEBUG)
    printf("TOC_SEND TYPE(%d) SEQ(%d) LEN(%d)  ",type,seq,len);

  if(!tocc->connected)
    return 0;
  if(tocc->state == TOC_STATE_PAUSE)
    return 0;

  if(TOC_DEBUG) {
    for(i=0;i<tocc->send_len;i++)
      if(tocc->send_buf[i] < 32)
	printf("%d",tocc->send_buf[i]);
      else
	printf("%c",tocc->send_buf[i]);
    printf("\n");
  }

  return send(tocc->skt,tocc->send_buf,tocc->send_len,0);
}

int toc_send_text(TOC_CONN tocc,
		  unsigned char* data) {

  unsigned char* tmp;
  unsigned short len;
  
  /* Quote data */

  /* Build the header */
  len = strlen(data);
  if(len >= TOC_MAX_MSG_LEN-1)
    len = TOC_MAX_MSG_LEN - 2;
  
  len++; /* send the null character */

  tmp = toc_header_fill(tocc,TOC_TYPE_DATA,len);

  /* Add the Data */
  strncpy(tmp,data,len);  
  tocc->send_len += len;

  /* Send it */
  return toc_send(tocc);
}

int toc_send_printf(TOC_CONN tocc,
		    const char* format,
		    ...) {
  va_list ap;

  /* Clear the tmp buffer */
  memset(tocc->tmp_buf,0,2064);

  /* Do the printf */
  va_start(ap, format);
  vsprintf(tocc->tmp_buf,format,ap);
  va_end(ap);

  return toc_send_text(tocc,tocc->tmp_buf);
}

/**** VCSI Functions *****/
VCSI_OBJECT toc_handler_set(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y, 
			    VCSI_OBJECT z) {

  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-handler-set! is of the wrong type",x);
  
  check_arg_type(vc,y,SYMBOL,"toc-handler-set!");
  
  if(!strcasecmp(SNAME(y),"im")) {
    TOCC(x)->on_im = z;
  } else if(!strcasecmp(SNAME(y),"update-buddy")) {
    TOCC(x)->on_update_buddy = z;
  } else if(!strcasecmp(SNAME(y),"error")) {
    TOCC(x)->on_error = z;
  } else 
    error(vc,"invalid handler name",y);
  
  return vc->true;
}

VCSI_OBJECT toc_handle_one_event(VCSI_CONTEXT vc,
				 VCSI_OBJECT x) {

  TOC_EVENT toce;

  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-handle-one-event is of the wrong type",x);

  if(!TOCC(x)->connected)
    error(vc,"not connected",x);

  /* Get the event */
  toce = toc_read_event(TOCC(x));
  
  /* Handle it */
  toc_handle_event(vc,TOCC(x),toce);

  FREE(toce);

  return vc->true;
}

VCSI_OBJECT toc_event_ready(VCSI_CONTEXT vc,
			    VCSI_OBJECT x) {

  fd_set rfds;
  struct timeval tv;
  int i;

  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-event-ready? is of the wrong type",x);

  if(!TOCC(x)->connected)
    return vc->false;

  FD_ZERO(&rfds);
  FD_SET(TOCC(x)->skt, &rfds);
   
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  i = select(TOCC(x)->skt+1, &rfds, NULL, NULL, &tv);
   
  if(i || i == -1)
    return vc->true;
  
  return vc->false;
}

VCSI_OBJECT toc_init(VCSI_CONTEXT vc, 
		     VCSI_OBJECT username,
		     VCSI_OBJECT passwd) {

  TOC_CONN tocc = NULL;
  VCSI_OBJECT tmp;

  check_arg_type(vc,username,STRING,"make-toc");
  check_arg_type(vc,passwd,STRING,"make-toc");

  tocc = MALLOC(SIZE_TOC_CONN);
  memset(tocc,0,SIZE_TOC_CONN);

  strncpy(tocc->username,toc_normalize_username(STR(username)),255);
  strncpy(tocc->passwd,STR(passwd),255);
  
  tocc->code = toc_code(tocc->username[0],tocc->passwd[0]);

  toc_roast_password(tocc);

  tocc->connected = 0;
  tocc->sequence = 0;
  tocc->state = TOC_STATE_OFFLINE;

  tmp = make_user_def(vc,(void*)tocc,toc_conn_type);
  
  return tmp;
}

VCSI_OBJECT toc_signon(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  TOC_CONN tocc;
  TOC_EVENT toce;
  int len;

  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-signon is of the wrong type",x);

  tocc = TOCC(x);

  tocc->state = TOC_STATE_FLAPON;
  
  /* Client sends FLAPON */
  send(tocc->skt,TOC_FLAPON,strlen(TOC_FLAPON),0);

  /* TOC send client FLAP SIGNON */
  toce = toc_read_event(tocc);
  if(toce->type != TOC_EV_SIGNON_REQUEST)
    error(vc,toce->data,x);
  FREE(toce);

  tocc->state = TOC_STATE_SIGNON_REQUEST;

  /* Grab a random sequence number */
  tocc->sequence = 1+(unsigned short) (65536.0*rand()/(RAND_MAX+1.0));

  /* Client sends TOC FLAP SIGNON */
  len = TOC_HDR_SIGNON_LEN + strlen(tocc->username);
  toc_header_fill(tocc,TOC_TYPE_SIGNON,len);
  toc_header_fill_signon(tocc);
  toc_send(tocc);

  /* Client Send toc_signon message */
  toc_send_printf(tocc, "toc2_signon %s %d %s %s %s \"TIC:vcsi %s toc2 %s\" 160 %d",
		  TOC_AUTH_HOST, TOC_AUTH_PORT,
		  tocc->username, tocc->passwd,
		  TOC_LANGUAGE,
		  vcsi_version, TOC_VERSION, 
		  tocc->code);

  /* TOC Closes connection or sends client SIGNON reply */
  toce = toc_read_event(tocc);
  if(toce->type != TOC_EV_SIGNON) {
    tocc->connected = 0;
    return error(vc,"signon failed",x);
  }
  toc_handle_event(vc,tocc,toce);
  FREE(toce);

  /* OPTIONALS */
  
  /* TOC sends client CONFIG */
  for(toce=toc_read_event(tocc);
      toce->type!=TOC_EV_CONFIG;
      toce=toc_read_event(tocc)) {
    toc_handle_event(vc,tocc,toce);
    FREE(toce);
  }

  /* Client sends TOC permit/denys */
  /* Client sends TOC toc_add_buddy's */
  toc_handle_event(vc,tocc,toce);
  FREE(toce);

  /* END OPTIONALS */

  /* Client sends TOC toc_init_done */
  toc_send_text(tocc,"toc_init_done");

  /* Set out capabilities */
  toc_send_text(tocc,"toc2_set_caps 09461343-4C7F-11D1-8222-444553540000");

  tocc->state = TOC_STATE_ONLINE;

  return vc->true;
}

VCSI_OBJECT toc_connect(VCSI_CONTEXT vc,
			VCSI_OBJECT x) {

  TOC_CONN tocc;
  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  
  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-signon is of the wrong type",x);
  
  tocc = TOCC(x);

  if((hp = gethostbyname(TOC_SERVER_HOST)) == NULL)
    return error(vc,"unknown host",make_string(vc,TOC_SERVER_HOST));
  
  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return error(vc,"unable to create socket",make_string(vc,TOC_SERVER_HOST));
  
  memset(&name, 0, sizeof(struct sockaddr_in));
  name.sin_family = AF_INET;
  name.sin_port = htons(TOC_SERVER_PORT);
  memcpy(&name.sin_addr, hp->h_addr_list[0], hp->h_length);
  len = sizeof(struct sockaddr_in);
  
  if(connect(s, (struct sockaddr *) &name, len) < 0)
    return error(vc,"connection refused",make_string(vc,TOC_SERVER_HOST));
  
  tocc->skt = s;
  tocc->connected = 1;

  return toc_signon(vc,x);
}

VCSI_OBJECT toc_disconnect(VCSI_CONTEXT vc,
			   VCSI_OBJECT x) {
  
  TOC_CONN tocc;
  
  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-signon is of the wrong type",x);
  
  tocc = TOCC(x);
     
  if(!tocc->connected)
    return error(vc,"not connected",x);

  close(tocc->skt);
  tocc->connected = 0;
  tocc->state = TOC_STATE_OFFLINE;
  return vc->true;
}

VCSI_OBJECT toc_gen_buddy_list(VCSI_CONTEXT vc,
			       VCSI_OBJECT x,
			       int mode) {
  
  TOC_CONN tocc;
  VCSI_OBJECT res = NULL, list;
  TOC_BUDDY_GROUP tmp;
  TOC_BUDDY  tmp2;
    
  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-buddy-list is of the wrong type",x);
  
  tocc = TOCC(x);
     
  if(!tocc->connected)
    return error(vc,"not connected",x);
  if(tocc->buddies) {    
    for(tmp=tocc->buddies;tmp;tmp=tmp->next) {
      list = cons(vc,make_string(vc,tmp->name),NULL);
      if(tmp->buddies) {
	for(tmp2=tmp->buddies;tmp2;tmp2=tmp2->next) {
	  if(mode == 0)
	    list = cons(vc,make_string(vc,tmp2->nice_name),list);
	  else if(mode == 1 && tmp2->online)
	    list = cons(vc,make_string(vc,tmp2->nice_name),list);
	  else if(mode == 2 && !tmp2->online)
	    list = cons(vc,make_string(vc,tmp2->nice_name),list);
	  else if(mode == 3 && tmp2->online && !tmp2->away)
	    list = cons(vc,make_string(vc,tmp2->nice_name),list);
	  else if(mode == 4 && tmp2->online && tmp2->away)
	    list = cons(vc,make_string(vc,tmp2->nice_name),list);
	}
      }
      res = cons(vc,vcsi_list_reverse(vc,list),res);
    }
  }
  
  return vcsi_list_reverse(vc,res);
}

VCSI_OBJECT toc_buddy_list(VCSI_CONTEXT vc,
			   VCSI_OBJECT x) {

  return toc_gen_buddy_list(vc,x,0);
}

VCSI_OBJECT toc_buddy_list_online(VCSI_CONTEXT vc,
				  VCSI_OBJECT x) {

  return toc_gen_buddy_list(vc,x,1);
}

VCSI_OBJECT toc_buddy_list_offline(VCSI_CONTEXT vc,
				   VCSI_OBJECT x) {

  return toc_gen_buddy_list(vc,x,2);
}

VCSI_OBJECT toc_buddy_list_available(VCSI_CONTEXT vc,
				     VCSI_OBJECT x) {

  return toc_gen_buddy_list(vc,x,3);
}

VCSI_OBJECT toc_buddy_list_away(VCSI_CONTEXT vc,
				VCSI_OBJECT x) {

  return toc_gen_buddy_list(vc,x,4);
}

VCSI_OBJECT toc_gen_buddy_list_flat(VCSI_CONTEXT vc,
				    VCSI_OBJECT x,
				    int mode) {
  
  TOC_CONN tocc;
  VCSI_OBJECT res = NULL;
  TOC_BUDDY_GROUP tmp;
  TOC_BUDDY  tmp2;
  
  
  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-buddy-list is of the wrong type",x);
  
  tocc = TOCC(x);
     
  if(!tocc->connected)
    return error(vc,"not connected",x);

  if(tocc->buddies) {    
    for(tmp=tocc->buddies;tmp;tmp=tmp->next) {
      if(tmp->buddies) {
	for(tmp2=tmp->buddies;tmp2;tmp2=tmp2->next) {
	  if(mode == 0)
	    res = cons(vc,make_string(vc,tmp2->nice_name),res);
	  else if(mode == 1 && tmp2->online)
	    res = cons(vc,make_string(vc,tmp2->nice_name),res);
	  else if(mode == 2 && !tmp2->online)
	    res = cons(vc,make_string(vc,tmp2->nice_name),res);
	  else if(mode == 3 && tmp2->online && !tmp2->away)
	    res = cons(vc,make_string(vc,tmp2->nice_name),res);
	  else if(mode == 4 && tmp2->online && tmp2->away)
	    res = cons(vc,make_string(vc,tmp2->nice_name),res);
	}
      }
    }
  }
  
  return vcsi_list_reverse(vc,res);
}

VCSI_OBJECT toc_buddy_list_flat(VCSI_CONTEXT vc,
			   VCSI_OBJECT x) {

  return toc_gen_buddy_list_flat(vc,x,0);
}

VCSI_OBJECT toc_buddy_list_online_flat(VCSI_CONTEXT vc,
				  VCSI_OBJECT x) {

  return toc_gen_buddy_list_flat(vc,x,1);
}

VCSI_OBJECT toc_buddy_list_offline_flat(VCSI_CONTEXT vc,
				   VCSI_OBJECT x) {

  return toc_gen_buddy_list_flat(vc,x,2);
}

VCSI_OBJECT toc_buddy_list_available_flat(VCSI_CONTEXT vc,
				     VCSI_OBJECT x) {

  return toc_gen_buddy_list_flat(vc,x,3);
}

VCSI_OBJECT toc_buddy_list_away_flat(VCSI_CONTEXT vc,
				VCSI_OBJECT x) {

  return toc_gen_buddy_list_flat(vc,x,4);
}

VCSI_OBJECT toc_normalize_name(VCSI_CONTEXT vc,
			       VCSI_OBJECT x) {
  
  check_arg_type(vc,x,STRING,"toc-normalize-name");
  return make_string(vc,(char*)toc_normalize_username(STR(x)));
}

VCSI_OBJECT toc_buddy_add(VCSI_CONTEXT vc,
			  VCSI_OBJECT x,
			  VCSI_OBJECT y) {

  TOC_BUDDY_GROUP tocg;
  TOC_BUDDY tocb;

  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-buddy-add is of the wrong type",x);
  
  if(!TOCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"toc-buddy-add");
  
  tocg = toc_buddy_group_find_group(TOCC(x),"Buddies");
  if(!tocg)
    tocg = toc_buddy_group_add_group(TOCC(x),toc_buddy_group_init("Buddies"));

  tocb = toc_buddy_group_add(tocg,toc_buddy_init(STR(y)));

  if(tocb) {
    toc_config_save(TOCC(x));
    return vc->true;
  }

  return vc->false;
}

VCSI_OBJECT toc_im(VCSI_CONTEXT vc,
		   VCSI_OBJECT x,
		   VCSI_OBJECT y,
		   VCSI_OBJECT z) {

  if(!same_type_user_def(vc,x,toc_conn_type))
    error(vc,"objected passed to toc-im is of the wrong type",x);
  
  if(!TOCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"toc-im");
  check_arg_type(vc,z,STRING,"toc-im");

  toc_send_printf(TOCC(x),"toc2_send_im %s \"%s\"",
		  toc_normalize_username(STR(y)),STR(z));
  return vc->true;
}

/***** Module Required Function *****/
void toc_print(VCSI_CONTEXT vc,
	       INFOSTD_DYN_STR res, 
	       VCSI_OBJECT obj) {
  
  infostd_dyn_str_printf(vc->res_extra,"#[toc \"%s\" (%s)]",
			 TOCC(obj)->username,
			 (TOCC(obj)->connected) ? 
			 "connected" : "disconnected");
  
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void toc_mark(VCSI_CONTEXT vc,
	      VCSI_OBJECT obj, 
	      int flag) {

  /* Mark all the handlers */
  gc_mark(vc,TOCC(obj)->on_im,flag);
  gc_mark(vc,TOCC(obj)->on_update_buddy,flag);
  gc_mark(vc,TOCC(obj)->on_error,flag);
}

void toc_free(VCSI_CONTEXT vc,
	      VCSI_OBJECT obj) {
  
  if(TOCC(obj)->connected) {
    /* disconnect */
    close(TOCC(obj)->skt);
  }

  FREE(TOCC(obj));
}

void module_init(VCSI_CONTEXT vc) {

  toc_conn_type = make_user_def_type(vc, (char*)module_name,
				     toc_print,
				     toc_mark,
				     toc_free);

  /*set_int_proc(vc,"toc?",PROC1,toc_connq);*/
  set_int_proc(vc,"make-toc",PROC2,toc_init);
  set_int_proc(vc,"toc-connect",PROC1,toc_connect);
  set_int_proc(vc,"toc-disconnect",PROC1,toc_disconnect);

  set_int_proc(vc,"toc-buddy-list",PROC1,toc_buddy_list);
  set_int_proc(vc,"toc-buddy-list-online",PROC1,toc_buddy_list_online);
  set_int_proc(vc,"toc-buddy-list-offline",PROC1,toc_buddy_list_offline);
  set_int_proc(vc,"toc-buddy-list-available",PROC1,toc_buddy_list_available);
  set_int_proc(vc,"toc-buddy-list-away",PROC1,toc_buddy_list_away);

  set_int_proc(vc,"toc-buddy-list-flat",PROC1,toc_buddy_list_flat);
  set_int_proc(vc,"toc-buddy-list-online-flat",PROC1,
	       toc_buddy_list_online_flat);
  set_int_proc(vc,"toc-buddy-list-offline-flat",PROC1,
	       toc_buddy_list_offline_flat);
  set_int_proc(vc,"toc-buddy-list-available-flat",PROC1,
	       toc_buddy_list_available_flat);
  set_int_proc(vc,"toc-buddy-list-away-flat",PROC1,toc_buddy_list_away_flat);

  set_int_proc(vc,"toc-handler-set!",PROC3,toc_handler_set);
  set_int_proc(vc,"toc-handle-one-event",PROC1,toc_handle_one_event);
  set_int_proc(vc,"toc-event-ready?",PROC1,toc_event_ready);

  set_int_proc(vc,"toc-normalize-name",PROC1,toc_normalize_name);

  set_int_proc(vc,"toc-buddy-add",PROC2,toc_buddy_add);

  set_int_proc(vc,"toc-im",PROC3,toc_im);
}

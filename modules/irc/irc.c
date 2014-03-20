#include <vcsi.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

extern int errno;

const char module_name[] = "irc";
const char module_version[] = "irc-v1.0-DEBUG";

/* IRC Connection */
typedef struct str_irc_conn {
  /* server */
  char server[128];
  long port;

  /* nick and name */
  char nick[30];
  char name[128];

  /* security */
  int secure_mode;
  char secure_passwd[30];

  /* call backs */

  /* messages */
  VCSI_OBJECT on_notice; /* general notices */
  VCSI_OBJECT on_error; /* error notices */
  VCSI_OBJECT on_public; /* public msg */
  VCSI_OBJECT on_msg; /* private msg */
  VCSI_OBJECT on_action; /* action */
  VCSI_OBJECT on_ctcp; /* generic ctcp */

  /* CTCPs */
  VCSI_OBJECT on_ctcp_ping; /* ping */
  VCSI_OBJECT on_ctcp_version; /* version ping */

  /* channel stuff */
  VCSI_OBJECT on_join; /* when someone joins the channel */
  VCSI_OBJECT on_part; /* when someone leaves the channel */
  VCSI_OBJECT on_quit; /* when someone quits */
  VCSI_OBJECT on_disconnect; /* when we are disconnected */

  /* other stuff */
  VCSI_OBJECT on_nick_change; /* someone changed their nick */
  VCSI_OBJECT on_nick_taken; /* our nick is taken */
  VCSI_OBJECT on_kick; /* when we are kicked */

  /* command respsonses */
  VCSI_OBJECT on_names; /* results of a names command */
  VCSI_OBJECT on_topic; /* when the topic changes */

  VCSI_OBJECT on_is_away; /* a user is away */
  VCSI_OBJECT on_away; /* when we go away */
  VCSI_OBJECT on_back; /* when we are back */

  VCSI_OBJECT on_ison; /* results of /ison */
  
  /* connected? */
  int connected;

  /* buffers */
  char send_buf[1024];
  char recv_buf[1024];

  /* the actual socket */
  int skt;

} *IRC_CONN;
#define SIZE_IRC_CONN (sizeof(struct str_irc_conn))
#define IRCBUF 1024

enum irc_event_type {
  NOTICE, ERROR, PING, PONG, PUBLIC, MSG, ACTION, CTCP, CTCP_PING, 
  CTCP_VERSION, JOIN, PART, QUIT, DISCONNECT, NICK_CHANGE, NICK_TAKEN, KICK, 
  NAMES, TOPIC, IS_AWAY, AWAY, BACK, ISON
};

/* IRC Event */
typedef struct str_irc_event {
  enum irc_event_type type;
  char channel[128];
  char target[128];
  char nick[128];
  char host[128];
  char text[1024];
} *IRC_EVENT;
#define SIZE_IRC_EVENT (sizeof(struct str_irc_event))

/* user defined type stuff */
VCSI_USER_DEF irc_conn_type = NULL;
#define IRCC(x) ((IRC_CONN)(*x).data.userdef.ud_value)

/* Event Functions */
IRC_EVENT parse_event(char* etext) {
  IRC_EVENT irce = NULL;
  char *nick, *host, *type, *target, *target2, *text;
  int len, i, dtype;

  nick = host = type = target = target2 = text = NULL;
  
  irce = MALLOC(SIZE_IRC_EVENT);
  memset(irce,0,SIZE_IRC_EVENT);

  if(!etext) {
    irce->type = DISCONNECT;
    return irce;
  }

  if(etext[0] == ':') {
    nick = etext; /* get the nick */
    nick++;
    
    type = strchr(nick,' '); /* get the type */
    if(type) {
      *type='\0';
      type++;
      
      target = strchr(type,' '); /* get the target */
      if(target) {
	*target='\0';
	target++;
	
	if(*target==':') {
	  text=target;
	  text++;
	  target=NULL;

	} else {
	  text = strchr(target,' ');
	  
	  if(text) {
	    *text='\0';
	    text++;
	    if(*text==':')
	      text++;
	    else {
	      target2=text;
	      if(*target2 == '=' || *target2 == '@')
		target2+=2;

	      text=strchr(target2,' ');
	      if(text) {
		*text='\0';
		text++;
		if(*text==':')
		  text++;
	      }
	      
	    }
	  }
	}
      }
    }
    
    if(nick && (host=strchr(nick,'!'))) {    
      *host='\0';
      host++;
    } else {
      host=nick;
      nick=NULL;
    }
    
    /* Did the text come though as the second target? */
    if(!text) {
      if(target2) {
	text=target2;
	target2=NULL;
      }
    }

    /* strip extra garbage */
    if(text && (len = strlen(text))) {
      if(text[len-1] == '\r' || text[len-1] == '\n')
	text[len-1] = '\0';
    }

    /* DEBUG */
    /*
    fprintf(stderr,"GOT: %s %s %s %s %s %s\n",
	    nick,host,type,target,target2,text);
    */
    if(type)
      dtype = atol(type);
    else
      dtype == 0;

    if(!strcmp(type,"PRIVMSG")) { /* message */
      if(*target=='#') { /* public */
	irce->type = PUBLIC;
	strncpy(irce->channel,target,127);	
      } else  /* private */
	irce->type = MSG;
      
      if(target)
	strncpy(irce->target,target,127);	
      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	

      if(*text==1) { /* CTCP something */
	text++;
	text[strlen(text)-1]='\0';

	if(!strncmp(text,"ACTION",6)) {
	  irce->type = ACTION;
	  text+=7;
	} else if(!strncmp(text,"VERSION",7)) {
	  irce->type = CTCP_VERSION;
	  text+=8;
	  if(!strncmp(text,"PING",4))
	    text+=5;
	} else if(!strncmp(text,"PING",4)) {
	  irce->type = CTCP_PING;
	  text+=5;
	} else
	  irce->type = CTCP;
      }
      
      strncpy(irce->text,text,1023);
    } else if(!strcmp(type,"JOIN")) { /* someone joins */
      irce->type = JOIN;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(text)
	strncpy(irce->channel,text,127);	

    } else if(!strcmp(type,"PART")) { /* someone parts */
      irce->type = PART;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->channel,target,127);	

    } else if(!strcmp(type,"QUIT")) { /* someone quits */
      irce->type = QUIT;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(text)
	strncpy(irce->text,text,1023);	

    } else if(!strcmp(type,"NICK")) { /* nick change */
      irce->type = NICK_CHANGE;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"PONG")) {
      irce->type = PONG;

      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->channel,target,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"PING")) {
      irce->type = PING;

      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->channel,target,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"433")) { /* nick already taken */
      irce->type = NICK_TAKEN;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	

    } else if(!strcmp(type,"KICK")) { /* when someone is kicked */
      irce->type = KICK;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->channel,target,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	

    } else if(!strcmp(type,"353")) { /* result of names command */
      irce->type = NAMES;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->channel,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	

    } else if(!strcmp(type,"TOPIC")) {
      /* result of setting the topic */
      irce->type = TOPIC;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->channel,target,127);	
      if(text)
	strncpy(irce->text,text,1023);	

    } else if(!strcmp(type,"332")) { 
      /* result of getting the topic */
      irce->type = TOPIC;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->channel,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	    
    } else if(!strcmp(type,"301")) { 
      /* A user is away */
      irce->type = IS_AWAY;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"303")) { 
      /* We are now back */
      irce->type = ISON;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"305")) { 
      /* We are now back */
      irce->type = BACK;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"306")) { 
      /* We are now away */
      irce->type = AWAY;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	
    } else if(!strcmp(type,"365") || !strcmp(type,"366") ||
	      !strcmp(type,"368") || !strcmp(type,"374") ||
	      !strcmp(type,"376") || !strcmp(type,"394")) {
      /* end of messages - ignore! */
      FREE(irce);
      return NULL;
    } else if(dtype > 400 && dtype < 503) {
      /* Generic error */
      irce->type = ERROR;

      if(host)
	strncpy(irce->host,host,127);	
      if(target2)
	strncpy(irce->target,target2,127);	
      if(text)
	strncpy(irce->text,text,1023);	

    } else if(dtype >= 200 && dtype <= 600) {
      /* Numeric reply of some kind */
      irce->type = NOTICE;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->target,target,127);	

      if(text)
	snprintf(irce->text,1023,"IRC(%d) %s",dtype,text);
      else
	snprintf(irce->text,1023,"IRC(%d)",dtype);	
    } else { /* assume general notice */
      irce->type = NOTICE;

      if(nick)
	strncpy(irce->nick,nick,127);	
      if(host)
	strncpy(irce->host,host,127);	
      if(target)
	strncpy(irce->target,target,127);	
      if(text)
	strncpy(irce->text,text,1023);
    }
  } else { /* generic notice */
    type = etext;

    text = strchr(type,' ');
    if(text) {
      *text = '\0';
      text++;

      if(*text == ':')
	text++;

    } else {
      type = NULL;
      text = etext;
    }

    if((len = strlen(text))) { /* strip extra garbage */
      if(text[len-1] == '\r' || text[len-1] == '\n')
	text[len-1] = '\0';
    }

    if(type && !strcmp(type,"PING")) {
      fflush(stdout);
      irce->type = PING;
      strncpy(irce->host,text,127);
    } else {
      irce->type = NOTICE;    
      
      strncpy(irce->text,text,1023);
    }
  }

  /* auto lowercase the channel */
  len = strlen(irce->channel);
  for(i=0;i<len;i++)
    irce->channel[i] = tolower(irce->channel[i]);

  return irce;
}

VCSI_OBJECT handle_ping_event(VCSI_CONTEXT vc, 
			      IRC_CONN ircc, 
			      IRC_EVENT irce) {
  
  if(!ircc->connected)
    return vc->false;
  
  memset(ircc->send_buf,0,IRCBUF);
  
  snprintf(ircc->send_buf,IRCBUF,"PONG :%s\n",irce->host);
  
  send(ircc->skt,ircc->send_buf,strlen(ircc->send_buf),0);
  
  //printf("DEBUG: I PONG'd\n");

  return vc->true;
}

VCSI_OBJECT handle_event(VCSI_CONTEXT vc, 
			 IRC_CONN ircc, 
			 IRC_EVENT irce) {

  VCSI_OBJECT res, func, vars, vals;
  VCSI_OBJECT channel, target, nick, host, text;

  res = vc->true; /* default value for handled event */
  func = vars = vals = channel = target = nick = host = text = NULL;


  switch(irce->type) {
  case NOTICE:
    func = ircc->on_notice;
    break;
  case ERROR:
    func = ircc->on_error;
    break;
  case PING:
    //printf("DEBUG: SERVER PING\n");
    res = handle_ping_event(vc,ircc,irce);
    break;
  case PONG:
    //printf("DEBUG: SERVER PONG\n");
    /* We don't actually care what it returns as long as it does... */
    break;
  case PUBLIC:
    func = ircc->on_public;
    break;
  case MSG:
    func = ircc->on_msg;
    break;
  case ACTION:
    func = ircc->on_action;
    break;
  case CTCP:
    func = ircc->on_ctcp;
    break;
  case CTCP_PING:
    //printf("DEBUG: CTCP_PING\n");
    func = ircc->on_ctcp_ping;
    break;
  case CTCP_VERSION:
    //printf("DEBUG: CTCP_VERSION\n");
    func = ircc->on_ctcp_version;
    break;
  case JOIN:
    func = ircc->on_join;
    break;
  case PART:
    func = ircc->on_part;
    break;
  case QUIT:
    func = ircc->on_quit;
    break;
  case DISCONNECT:
    func = ircc->on_disconnect;
    break;
  case NICK_CHANGE:
    func = ircc->on_nick_change;
    break;
  case NICK_TAKEN:
    func = ircc->on_nick_taken;
    break;
  case KICK:
    func = ircc->on_kick;
    break;
  case NAMES:
    func = ircc->on_names;
    break;
  case TOPIC:
    func = ircc->on_topic;
    break;
  case IS_AWAY:
    func = ircc->on_is_away;
    break;
  case AWAY:
    func = ircc->on_away;
    break;
  case BACK:
    func = ircc->on_back;
    break;
  case ISON:
    func = ircc->on_ison;
    break;

  default:
    res = vc->false; /* not an event we understand */;
  }
  
  if(func) {

    if(strlen(irce->channel))
      channel = make_string(vc,irce->channel);

    if(strlen(irce->target))
      target = make_string(vc,irce->target);
    
    if(strlen(irce->nick))
      nick = make_string(vc,irce->nick);
    
    if(strlen(irce->host))
      host = make_string(vc,irce->host);
    
    if(strlen(irce->text))
      text = make_string(vc,irce->text);
    
    func = make_list(vc,6,func,channel,target,nick,host,text);
      
    res=eval(vc,func,vc->the_global_env);
  }     

  return res;
}


void fake_event(VCSI_CONTEXT vc,
		IRC_CONN ircc,
		enum irc_event_type type,
		char *channel,
		char *target,
		char *source,
		char *hostmask,
		char *text) {

  IRC_EVENT irce;

  irce = MALLOC(SIZE_IRC_EVENT);
  memset(irce,0,SIZE_IRC_EVENT);

  irce->type = type;

  if(channel)
    strncpy(irce->channel,channel,127);	
  if(target)
    strncpy(irce->target,target,127);	
  if(source)
    strncpy(irce->nick,source,127);	
  if(hostmask)
    strncpy(irce->host,hostmask,127);	
  if(text)
    strncpy(irce->text,text,1023);	

  handle_event(vc,ircc,irce);

  FREE(irce);
}
/* IRC Functions */

VCSI_OBJECT init_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT server, 
			  VCSI_OBJECT port,
			  VCSI_OBJECT nick, 
			  VCSI_OBJECT name) {
  
  IRC_CONN ircc = NULL;
  VCSI_OBJECT tmp;
  
  check_arg_type(vc,server,STRING,"make-irc");
  check_arg_type(vc,port,LNGNUM,"make-irc");
  check_arg_type(vc,nick,STRING,"make-irc");
  check_arg_type(vc,name,STRING,"make-irc");


  ircc = MALLOC(SIZE_IRC_CONN);
  memset(ircc,0,SIZE_IRC_CONN);

  strncpy(ircc->server,STR(server),128);
  ircc->port = LNGN(port);
  
  strncpy(ircc->nick,STR(nick),30);
  strncpy(ircc->name,STR(name),128);

  ircc->secure_mode = 0;

  tmp = make_user_def(vc,(void*)ircc,irc_conn_type);
  
  return tmp;
}

VCSI_OBJECT connect_irc_conn(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x) {

  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  
  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-connect is of the wrong type",x);
  
  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);
  
  if((hp = gethostbyname(IRCC(x)->server)) == NULL)
    return error(vc,"unknown host",make_string(vc,IRCC(x)->server));
  
  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return error(vc,"unable to create socket",make_string(vc,IRCC(x)->server));
  
  memset(&name, 0, sizeof(struct sockaddr_in));
  name.sin_family = AF_INET;
  name.sin_port = htons(IRCC(x)->port);
  memcpy(&name.sin_addr, hp->h_addr_list[0], hp->h_length);
  len = sizeof(struct sockaddr_in);
  
  if(connect(s, (struct sockaddr *) &name, len) < 0)
    return error(vc,"connection refused",make_string(vc,IRCC(x)->server));
  
  IRCC(x)->skt = s;

  /* send the nick */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"NICK %s\n",IRCC(x)->nick);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  /* send the user */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"USER %s %s %s :%s\n",IRCC(x)->nick,
	   "localhost","localhost",IRCC(x)->name);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  IRCC(x)->connected = 1;
       
  return vc->true;
}

VCSI_OBJECT disconnect_irc_conn(VCSI_CONTEXT vc, 
				VCSI_OBJECT x) {

  int s, len;
  struct hostent *hp;
  struct sockaddr_in name;
  
  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-connect is of the wrong type",x);
  
  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  if(!IRCC(x)->connected)
    return vc->true;
  
  close(IRCC(x)->skt);
  IRCC(x)->connected=0;
  
  return vc->true;
}

VCSI_OBJECT secure_irc_conn(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-secure is of the wrong type",x);

  check_arg_type(vc,y,STRING,"irc-secure");

  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  strncpy(IRCC(x)->secure_passwd,STR(y),30);
  IRCC(x)->secure_mode = 1;

  return vc->true;
}

VCSI_OBJECT unsecure_irc_conn(VCSI_CONTEXT vc, 
			      VCSI_OBJECT x, 
			      VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-secure is of the wrong type",x);

  check_arg_type(vc,y,STRING,"irc-secure");

  if(!strcmp(IRCC(x)->secure_passwd,STR(y))) {
    IRCC(x)->secure_mode = 0;
    return vc->true;
  }
  
  return vc->false;
}


VCSI_OBJECT raw_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-raw is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-raw");

  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"%s\n",STR(y));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT quit_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-quit is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-quit");

  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"QUIT :%s\n",STR(y));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  shutdown(IRCC(x)->skt,2);

  IRCC(x)->connected=0;

  return vc->true;
}

VCSI_OBJECT join_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-join is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-join");


  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"JOIN %s\n",STR(y));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT part_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-part is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-part");

  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"PART %s\n",STR(y));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT nick_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-nick is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-nick");

  if(IRCC(x)->secure_mode)
    return error(vc,"irc security is enabled",x);

  strncpy(IRCC(x)->nick,STR(y),30);  

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"NICK %s\n",IRCC(x)->nick);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT notice_irc_conn(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x, 
			     VCSI_OBJECT y, 
			     VCSI_OBJECT z) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-notice is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-notice");
  check_arg_type(vc,z,STRING,"irc-notice");

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"NOTICE %s :%s\n",STR(y),STR(z));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT privmsg_irc_conn(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x, 
			     VCSI_OBJECT y, 
			     VCSI_OBJECT z) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-privmsg is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-privmsg");
  check_arg_type(vc,z,STRING,"irc-privmsg");

  if(IRCC(x)->secure_mode && (!strcasecmp(STR(y),"chanserv") || 
			      !strcasecmp(STR(y),"nickserv") ||
			      !strcasecmp(STR(y),"memoserv") ||
			      !strcasecmp(STR(y),"statserv") ||
			      !strcasecmp(STR(y),"seenserv") ||
			      !strcasecmp(STR(y),"imserv") ||
			      !strcasecmp(STR(y),"aimserv")))
			      
    return error(vc,"irc security is enabled",x);


  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"PRIVMSG %s :%s\n",STR(y),STR(z));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT me_irc_conn(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y, 
			VCSI_OBJECT z) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-me is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-me");
  check_arg_type(vc,z,STRING,"irc-me");

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"PRIVMSG %s :%cACTION %s%c\n",
	   STR(y), 1, STR(z), 1);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT ctcp_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y, 
			  VCSI_OBJECT z) {
  
  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-ctcp is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-ctcp");
  check_arg_type(vc,z,STRING,"irc-ctcp");

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"PRIVMSG %s :%c%s%c\n",
	   STR(y), 1, STR(z), 1);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT topic_irc_conn(VCSI_CONTEXT vc, 
			   VCSI_OBJECT args, 
			   int length) {

  VCSI_OBJECT x, y, z;
  
  x = y = z = NULL;
  
  if(length < 2 || length > 3)
    error(vc,"invalid number of arguments to irc-topic",args);

  x = car(vc,args);
  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-topic is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);

  y = cadr(vc,args);
  check_arg_type(vc,y,STRING,"irc-topic");

  
  if(length == 3) {
    z = caddr(vc,args);
    check_arg_type(vc,z,STRING,"irc-topic");
  } 

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  if(z)
    snprintf(IRCC(x)->send_buf,IRCBUF,"TOPIC %s :%s\n",STR(y), STR(z));
  else
    snprintf(IRCC(x)->send_buf,IRCBUF,"TOPIC %s\n",STR(y));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT names_irc_conn(VCSI_CONTEXT vc,
			   VCSI_OBJECT args,
			   int length) {
  VCSI_OBJECT x, y;
  
  x = y = NULL;
  
  if(length < 1 || length > 2)
    error(vc,"invalid number of arguments to irc-names",args);

  x = car(vc,args);
  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-names is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);

  if(length == 2) {
    y = cadr(vc,args);
    check_arg_type(vc,y,STRING,"irc-names");
  }

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  if(y)
    snprintf(IRCC(x)->send_buf,IRCBUF,"NAMES %s\n",STR(y));
  else
    snprintf(IRCC(x)->send_buf,IRCBUF,"NAMES\n");

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;

}

VCSI_OBJECT kick_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT args, 
			  int length) {
  VCSI_OBJECT c, x, y, z;

  c = x = y = z = NULL;

  if(length < 3 || length > 4)
    error(vc,"invalid number of arguments to irc-kick",args);


  c = car(vc,args); /* irc-conn */
  if(!same_type_user_def(vc,c,irc_conn_type))
    error(vc,"objected passed to irc-kick is of the wrong type",x);

  if(!IRCC(c)->connected)
    error(vc,"not connected",c);

  if(IRCC(c)->secure_mode)
    return error(vc,"irc security is enabled",x);
  
  x = cadr(vc,args); /* channel */
  check_arg_type(vc,x,STRING,"irc-kick");

  y = caddr(vc,args); /* nick */
  check_arg_type(vc,y,STRING,"irc-kick");
  
  if(!strcasecmp(STR(y),IRCC(c)->nick))
    error(vc,"unable to kick myself",y);

  if(length == 4) {
    z = cadddr(vc,args);
    check_arg_type(vc,z,STRING,"irc-kick");
  }

  /* send the command */
  memset(IRCC(c)->send_buf,0,IRCBUF);
  
  if(z)
    snprintf(IRCC(c)->send_buf,IRCBUF,"KICK %s %s :%s\n",
	     STR(x), STR(y), STR(z));
  else
    snprintf(IRCC(c)->send_buf,IRCBUF,"KICK %s %s :%s\n",
	     STR(x), STR(y), IRCC(c)->nick);
  
  send(IRCC(c)->skt,IRCC(c)->send_buf,strlen(IRCC(c)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT away_irc_conn(VCSI_CONTEXT vc,
			  VCSI_OBJECT args,
			  int length) {
  VCSI_OBJECT x, y;
  
  x = y = NULL;
  
  if(length < 1 || length > 2)
    error(vc,"invalid number of arguments to irc-away",args);

  x = car(vc,args);
  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-away is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);

  if(length == 2) {
    y = cadr(vc,args);
    check_arg_type(vc,y,STRING,"irc-away");
  }

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  if(y)
    snprintf(IRCC(x)->send_buf,IRCBUF,"AWAY :%s\n",STR(y));
  else
    snprintf(IRCC(x)->send_buf,IRCBUF,"AWAY :away\n");

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;

}

VCSI_OBJECT back_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x) {	       

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-back is of the wrong type",x);
  
  if(!IRCC(x)->connected)
    error(vc,"not connected",x);  

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"AWAY\n");

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT ison_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-ison is of the wrong type",x);
  
  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-ison");

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"ISON %s\n",STR(y));

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT ping_reply_irc_conn(VCSI_CONTEXT vc, 
				VCSI_OBJECT x, 
				VCSI_OBJECT y,
				VCSI_OBJECT z) {


  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-ping-reply is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-ping-reply");
  check_arg_type(vc,z,STRING,"irc-ping-reply");

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"NOTICE %s :%cPING %s%c\n",
	   STR(y), 1, STR(z), 1);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

VCSI_OBJECT version_reply_irc_conn(VCSI_CONTEXT vc, 
				   VCSI_OBJECT x, 
				   VCSI_OBJECT y,
				   VCSI_OBJECT z) {
  

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-version-reply is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);
  
  check_arg_type(vc,y,STRING,"irc-version-reply");
  check_arg_type(vc,z,STRING,"irc-version-reply");

  /* send the command */
  memset(IRCC(x)->send_buf,0,IRCBUF);

  snprintf(IRCC(x)->send_buf,IRCBUF,"NOTICE %s :%cVERSION %s%c\n",
	   STR(y), 1, STR(z), 1);

  send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);

  return vc->true;
}

/* Event handlers */

VCSI_OBJECT set_handler_irc_conn(VCSI_CONTEXT vc, 
				 VCSI_OBJECT x, 
				 VCSI_OBJECT y, 
				 VCSI_OBJECT z) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-handler-set! is of the wrong type",x);
  
  check_arg_type(vc,y,SYMBOL,"irc-handler-set!");
  
  if(!strcasecmp(SNAME(y),"notice")) {
    IRCC(x)->on_notice = z;
  } else if(!strcasecmp(SNAME(y),"error")) {
    IRCC(x)->on_error = z;
  } else if(!strcasecmp(SNAME(y),"public")) {
    IRCC(x)->on_public = z;
  } else if(!strcasecmp(SNAME(y),"msg")) {
    IRCC(x)->on_msg = z;
  } else if(!strcasecmp(SNAME(y),"action")) {
    IRCC(x)->on_action = z;
  } else if(!strcasecmp(SNAME(y),"ctcp")) {
    IRCC(x)->on_ctcp = z;
  } else if(!strcasecmp(SNAME(y),"ping")) {
    IRCC(x)->on_ctcp_ping = z;
  } else if(!strcasecmp(SNAME(y),"version")) {
    IRCC(x)->on_ctcp_version = z;
  } else if(!strcasecmp(SNAME(y),"join")) {
    IRCC(x)->on_join = z;
  } else if(!strcasecmp(SNAME(y),"part")) {
    IRCC(x)->on_part = z;
  } else if(!strcasecmp(SNAME(y),"quit")) {
    IRCC(x)->on_quit = z;
  } else if(!strcasecmp(SNAME(y),"disconnect")) {
    IRCC(x)->on_disconnect = z;
  } else if(!strcasecmp(SNAME(y),"nick-change")) {
    IRCC(x)->on_nick_change = z;
  } else if(!strcasecmp(SNAME(y),"nick-taken")) {
    IRCC(x)->on_nick_taken = z;
  } else if(!strcasecmp(SNAME(y),"kick")) {
    IRCC(x)->on_kick = z;
  } else if(!strcasecmp(SNAME(y),"names")) {
    IRCC(x)->on_names = z;
  } else if(!strcasecmp(SNAME(y),"topic")) {
    IRCC(x)->on_topic = z;
  } else if(!strcasecmp(SNAME(y),"is-away")) {
    IRCC(x)->on_is_away = z;
  } else if(!strcasecmp(SNAME(y),"away")) {
    IRCC(x)->on_away = z;
  } else if(!strcasecmp(SNAME(y),"back")) {
    IRCC(x)->on_back = z;
  } else if(!strcasecmp(SNAME(y),"ison")) {
    IRCC(x)->on_ison = z;
  } else 
    error(vc,"invalid handler name",y);

  return vc->true;
}

VCSI_OBJECT handle_one_event_irc_conn(VCSI_CONTEXT vc, 
				      VCSI_OBJECT x) {

  IRC_EVENT irce = NULL;
  VCSI_OBJECT res = vc->true;
  unsigned char c;
  long i;
  int n;

  fd_set rfds;
  struct timeval tv;

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-handle-one-event is of the wrong type",x);

  if(!IRCC(x)->connected)
    error(vc,"not connected",x);

  memset(IRCC(x)->recv_buf,0,IRCBUF);

  /* Wait for up to five[three] minutes, then send a forced PING and deal with the result */
  tv.tv_sec = 30; /* Thirty Seconds */
  tv.tv_usec = 0;
    
  FD_ZERO(&rfds);
  FD_SET(IRCC(x)->skt, &rfds);


  if(!select(IRCC(x)->skt+1,&rfds,NULL,NULL,&tv)) {
    /* Nothing, send a keep-alive ping */
    memset(IRCC(x)->send_buf,0,IRCBUF);
    
    snprintf(IRCC(x)->send_buf,IRCBUF,"PING %s\n",IRCC(x)->server);
    printf("DEBUG: %s",IRCC(x)->send_buf);
    send(IRCC(x)->skt,IRCC(x)->send_buf,strlen(IRCC(x)->send_buf),0);
    
  }

  for(i=0,c=0;i<IRCBUF;i++) {
    n = recv(IRCC(x)->skt,&c,1,0);

    if(n==-1) {
      /* Something went wrong, maybe */
      printf("DEBUG: Something went wrong on recv...\n");
      switch(errno) {
      case EBADF:  
	printf("DEBUG: The argument s is an invalid descriptor.");
	break;
      case ECONNREFUSED:
	printf("DEBUG: Connection refused.\n");
	break;
      case ENOTCONN:
	printf("DEBUG: Not connected.\n");
	break;
      case ENOTSOCK:
	printf("DEBUG: Argument skt is not a socket.\n");
	break;
      case EAGAIN:
	printf("DEBUG: Operation would block or timed out.\n");
	break;
      case EINTR:
	printf("DEBUG: Interrupt received.\n");
	break;
      case EFAULT:
	printf("DEBUG: Recieve buffer points outside address space.\n");
	break;
      case EINVAL:
	printf("DEBUG: Invalid argument passed.\n");
	break;
      default:
	printf("DEBUG: Not sure (%d).\n", errno);
      }
      break;
    }
    
    if(c=='\n')
      break;
    
    IRCC(x)->recv_buf[i] = c;
  }
  IRCC(x)->recv_buf[i]=0;
  
  if(n == -1) {
    IRCC(x)->connected=0;
    irce = parse_event(NULL);
  } else 
    irce = parse_event(IRCC(x)->recv_buf);
  
  if(irce) {
    res = handle_event(vc,IRCC(x),irce);
    FREE(irce);
  }

  return res;
}

VCSI_OBJECT event_ready_irc_conn(VCSI_CONTEXT vc,
				 VCSI_OBJECT x) {

  fd_set rfds;
  struct timeval tv;
  int i;

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"objected passed to irc-event-ready? is of the wrong type",x);

  if(!IRCC(x)->connected)
    return vc->false;

  FD_ZERO(&rfds);
  FD_SET(IRCC(x)->skt, &rfds);
   
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  i = select(IRCC(x)->skt+1, &rfds, NULL, NULL, &tv);
   
  if(i || i == -1) /* So a disconnect get triggered */
    return vc->true;
  
  return vc->false;
}

/* Parse raw string and execute commands
 * (irc-command-parse ircc "/join #sass")
 * (irc-command-parse ircc "#sass" "/me lala")
 */
VCSI_OBJECT command_parse_irc_conn(VCSI_CONTEXT vc,
				   VCSI_OBJECT args,
				   int length) {
  VCSI_OBJECT ircc, chan, s;
  char *ln, *ln2;
  char str[1024];

  if(length == 2) {
    ircc = car(vc,args);
    s = cadr(vc,args);
    chan = NULL;
    
    if(!same_type_user_def(vc,ircc,irc_conn_type))
      error(vc,"objected passed to irc-command-parse is of the wrong type",
	    ircc);
    
    check_arg_type(vc,s,STRING,"irc-command-parse");
  } else if(length == 3) {
    ircc = car(vc,args);
    chan = cadr(vc,args);
    s = caddr(vc,args);

    
    if(!same_type_user_def(vc,ircc,irc_conn_type))
      error(vc,"objected passed to irc-command-parse is of the wrong type",
	    ircc);

    check_arg_type(vc,chan,STRING,"irc-command-parse");    
    check_arg_type(vc,s,STRING,"irc-command-parse");
  } else 
    return error(vc,"improper arguments to irc-command-parse",args);

  memset(str,0,1024);
  strncpy(str,STR(s),1023);
  
  if(!strcasecmp(str,"/connect")) {
    return connect_irc_conn(vc,ircc);
  } else if(!strcasecmp(str,"/disconnect")) {
    return disconnect_irc_conn(vc,ircc);
  } else if(!strncasecmp(str,"/join ",6) && strlen(str) > 6) {
    ln = str + 6;
    return join_irc_conn(vc,ircc,make_string(vc,ln));
  } else if(!strncasecmp(str,"/part ",6) && strlen(str) > 6) {
    ln = str + 6;
    return part_irc_conn(vc,ircc,make_string(vc,ln));
  } else if(!strcasecmp(str,"/part") && chan) {
    return part_irc_conn(vc,ircc,chan);
  } else if(!strncasecmp(str,"/nick ",6) && strlen(str) > 6) {
    ln = str + 6;
    return nick_irc_conn(vc,ircc,make_string(vc,ln));
  } else if(!strncasecmp(str,"/msg ",5) && strlen(str) > 5) {
    ln = str + 5;
    if((ln2=strchr(ln,' '))) {
      *ln2='\0';
      ln2++;
      fake_event(vc,IRCC(ircc),MSG,NULL,ln,IRCC(ircc)->nick,NULL,ln2);
      return privmsg_irc_conn(vc,ircc,make_string(vc,ln),make_string(vc,ln2));
    }
  } else if(!strncasecmp(str,"/ctcp ",5) && strlen(str) > 5) {
    ln = str + 5;
    if((ln2=strchr(ln,' '))) {
      *ln2='\0';
      ln2++;
      return ctcp_irc_conn(vc,ircc,make_string(vc,ln),make_string(vc,ln2));
    }
  } else if(!strncasecmp(str,"/me ",4)  && strlen(str) > 4 && chan) {
      ln = str + 4;
      fake_event(vc,IRCC(ircc),ACTION,STR(chan),NULL,IRCC(ircc)->nick,NULL,ln);
      return me_irc_conn(vc,ircc,chan,make_string(vc,ln));
  } else if(!strncasecmp(str,"/notice ",8)  && strlen(str) > 8 && chan) {
    ln = str + 8;
    return notice_irc_conn(vc,ircc,chan,make_string(vc,ln));
  } else if(!strncasecmp(str,"/raw ",5) && strlen(str) > 5) {
    ln = str + 5;
    return raw_irc_conn(vc,ircc,make_string(vc,ln));
  } else if(!strcasecmp(str,"/names")) {
    if(chan)
      names_irc_conn(vc,make_list(vc,2,ircc,chan),2);
    else
      names_irc_conn(vc,cons(vc,chan,NULL),1);
  } else if(!strncasecmp(str,"/ison ",6) && strlen(str) > 6) {
    ln = str + 6;
    return ison_irc_conn(vc,ircc,make_string(vc,ln));
  } else if(!strncasecmp(str,"/away ",6) && strlen(str) > 6) {
    ln = str + 6;
    return away_irc_conn(vc,make_list(vc,2,ircc,make_string(vc,ln)),2);
  } else if(!strcasecmp(str,"/away")) {
    return away_irc_conn(vc,cons(vc,ircc,NULL),1);
  } else if(!strcasecmp(str,"/back")) {
    return back_irc_conn(vc,ircc);
  } else if(!strncasecmp(str,"/topic ",7) && strlen(str) > 7 && chan) {
    ln = str + 7;
    return topic_irc_conn(vc,make_list(vc,3,ircc,chan,make_string(vc,ln)),3);
  } else if(!strcasecmp(str,"/topic") && chan) {
    return topic_irc_conn(vc,make_list(vc,2,ircc,chan),2);
  } else if(chan) {
    if(STR(chan)[0] == '#')
      fake_event(vc,IRCC(ircc),PUBLIC,STR(chan),NULL,
		 IRCC(ircc)->nick,NULL,str);
    else
      fake_event(vc,IRCC(ircc),MSG,NULL,STR(chan),IRCC(ircc)->nick,NULL,str);
    return privmsg_irc_conn(vc,ircc,chan,s);
  }

  return vc->false;
}

/* IRC Test Functions */
VCSI_OBJECT irc_connq(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    return vc->false;
  return vc->true;
}

VCSI_OBJECT connected_irc_connq(VCSI_CONTEXT vc, 
				VCSI_OBJECT x) {

  if(!same_type_user_def(vc,x,irc_conn_type))
    error(vc,"object passed to irc-connected? is of the wrong type",x);

  if(IRCC(x)->connected)
    return vc->true;
  return vc->false;
}

/* Module required functions */
void print_irc_conn(VCSI_CONTEXT vc,
		    INFOSTD_DYN_STR res, 
		    VCSI_OBJECT obj) {

  infostd_dyn_str_printf(vc->res_extra,"#[irc %s:%d \"%s\" (%s)]",
			 IRCC(obj)->server,
			 IRCC(obj)->port,
			 IRCC(obj)->nick,
			 (IRCC(obj)->connected) ? 
			 "connected" : "disconnected");
  
  infostd_dyn_str_strcat(res,vc->res_extra->buff);
}

void mark_irc_conn(VCSI_CONTEXT vc,
		   VCSI_OBJECT obj, 
		   int flag) {

  /* mark all the handlers */
  gc_mark(vc,IRCC(obj)->on_notice,flag);
  gc_mark(vc,IRCC(obj)->on_error,flag);
  gc_mark(vc,IRCC(obj)->on_public,flag);
  gc_mark(vc,IRCC(obj)->on_msg,flag);
  gc_mark(vc,IRCC(obj)->on_action,flag);
  gc_mark(vc,IRCC(obj)->on_ctcp,flag);

  gc_mark(vc,IRCC(obj)->on_ctcp_ping,flag);
  gc_mark(vc,IRCC(obj)->on_ctcp_version,flag);

  gc_mark(vc,IRCC(obj)->on_join,flag);
  gc_mark(vc,IRCC(obj)->on_part,flag);
  gc_mark(vc,IRCC(obj)->on_quit,flag);
  gc_mark(vc,IRCC(obj)->on_disconnect,flag);

  gc_mark(vc,IRCC(obj)->on_nick_change,flag);
  gc_mark(vc,IRCC(obj)->on_nick_taken,flag);
  gc_mark(vc,IRCC(obj)->on_kick,flag);

  gc_mark(vc,IRCC(obj)->on_names,flag);
  gc_mark(vc,IRCC(obj)->on_topic,flag);

  gc_mark(vc,IRCC(obj)->on_is_away,flag);
  gc_mark(vc,IRCC(obj)->on_away,flag);
  gc_mark(vc,IRCC(obj)->on_back,flag);

  gc_mark(vc,IRCC(obj)->on_ison,flag);
}

void free_irc_conn(VCSI_CONTEXT vc,
		   VCSI_OBJECT obj) {

  if(IRCC(obj)->connected) {
    /* disconnect */
    close(IRCC(obj)->skt);
  }

  FREE(IRCC(obj));
}

void module_init(VCSI_CONTEXT vc) {
  irc_conn_type = make_user_def_type(vc, (char*)module_name, print_irc_conn, 
				     mark_irc_conn, free_irc_conn);

  set_int_proc(vc,"irc?",PROC1,irc_connq);  
  set_int_proc(vc,"make-irc",PROC4,init_irc_conn);
  set_int_proc(vc,"irc-handler-set!",PROC3,set_handler_irc_conn);
  set_int_proc(vc,"irc-handle-one-event",PROC1,handle_one_event_irc_conn);
  set_int_proc(vc,"irc-event-ready?",PROC1,event_ready_irc_conn);
  set_int_proc(vc,"irc-connect",PROC1,connect_irc_conn);
  set_int_proc(vc,"irc-disconnect",PROC1,disconnect_irc_conn);
  set_int_proc(vc,"irc-connected?",PROC1,connected_irc_connq);
  set_int_proc(vc,"irc-secure",PROC2,secure_irc_conn);
  set_int_proc(vc,"irc-unsecure",PROC2,unsecure_irc_conn);
  set_int_proc(vc,"irc-raw",PROC2,raw_irc_conn);
  set_int_proc(vc,"irc-quit",PROC2,quit_irc_conn);
  set_int_proc(vc,"irc-join",PROC2,join_irc_conn);
  set_int_proc(vc,"irc-part",PROC2,part_irc_conn);
  set_int_proc(vc,"irc-nick",PROC2,nick_irc_conn);
  set_int_proc(vc,"irc-notice",PROC3,notice_irc_conn);
  set_int_proc(vc,"irc-privmsg",PROC3,privmsg_irc_conn);
  set_int_proc(vc,"irc-me",PROC3,me_irc_conn);
  set_int_proc(vc,"irc-ctcp",PROC3,ctcp_irc_conn);
  set_int_proc(vc,"irc-topic",PROCOPT,topic_irc_conn);
  set_int_proc(vc,"irc-names",PROCOPT,names_irc_conn);
  set_int_proc(vc,"irc-kick",PROCOPT,kick_irc_conn);
  set_int_proc(vc,"irc-away",PROCOPT,away_irc_conn);
  set_int_proc(vc,"irc-back",PROC1,back_irc_conn);
  set_int_proc(vc,"irc-ison",PROC2,ison_irc_conn);

  set_int_proc(vc,"irc-command-parse",PROCOPT,command_parse_irc_conn);

  set_int_proc(vc,"irc-ping-reply",PROC3,ping_reply_irc_conn);
  set_int_proc(vc,"irc-version-reply",PROC3,version_reply_irc_conn);
  
  /* To Do:
     oper?, mode, list, invite, info?, who, whois, whowas?, userhost
  */

}

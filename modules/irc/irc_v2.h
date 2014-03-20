/* IRC Connection */
typedef struct str_irc_conn {
  /* server */
  char server[128];
  long port;

  /* nick and name */
  char nick[30];
  char name[128];

#ifdef WITH_IRC_SECURE
  /* security */
  int secure_mode;
  char secure_passwd[30];
#endif

  ///////////////////////////////////////////////////////
  /* Idle and away counters */
  unsigned long idle;

  unsigned long away;
  char away_msg[128];

  /* Status changes Queue */
  INFOSTD_CQUEUE squeue;
  int squeue_enabled;

  /* Internal memory of channels and nicks */
  char last_msg[30];

  ///////////////////////////////////////////////////////

  /* call backs */

  /* messages */
  VCSI_OBJECT on_system; /* system notification */
  VCSI_OBJECT on_notice; /* general notices */
  VCSI_OBJECT on_error; /* error notices */
  VCSI_OBJECT on_public; /* public msg */
  VCSI_OBJECT on_msg; /* private msg */
  VCSI_OBJECT on_action; /* action */
  VCSI_OBJECT on_ctcp; /* generic ctcp */

  /* CTCPs */
  VCSI_OBJECT on_ctcp_ping; /* ping */
  VCSI_OBJECT on_ctcp_version; /* version ping */

  /* server / channel stuff */
  VCSI_OBJECT on_connect; /* when connected to the server */
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
  VCSI_OBJECT on_topic_who; /* when the topic changes */

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

typedef enum irc_event_type {
  NOTICE, ERROR, PING, PONG, PUBLIC, MSG, ACTION, CTCP, CTCP_PING, 
  CTCP_VERSION, CONNECT, JOIN, PART, QUIT, DISCONNECT, NICK_CHANGE, 
  NICK_TAKEN, KICK, NAMES, TOPIC, TOPIC_WHO, IS_AWAY, AWAY, BACK, 
  ISON, SYSTEM
} IRC_EVENT_TYPE;

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

/* status changes */
typedef enum irc_status_type {
  STATUS_NONE, STATUS_IDLE, STATUS_NOT_IDLE, STATUS_AWAY, STATUS_BACK, 
  STATUS_QUIT, STATUS_NICK_CHANGED, STATUS_JOINED, STATUS_PARTED,
} IRC_STATUS;

/* user defined type stuff */
VCSI_USER_DEF irc_conn_type;
#define IRCC(x) ((IRC_CONN)(*x).data.userdef.ud_value)


IRC_EVENT parse_event(char* etext);
VCSI_OBJECT handle_ping_event(VCSI_CONTEXT vc, 
			      IRC_CONN ircc, 
			      IRC_EVENT irce);

int irc_raw_ping(IRC_CONN ircc);

VCSI_OBJECT handle_event(VCSI_CONTEXT vc, 
			 IRC_CONN ircc, 
			 IRC_EVENT irce);

void fake_event(VCSI_CONTEXT vc,
		IRC_CONN ircc,
		IRC_EVENT_TYPE type,
		char *channel,
		char *target,
		char *source,
		char *hostmask,
		char *text);

IRC_STATUS irc_status_pop(IRC_CONN ircc);

void irc_status_push(IRC_CONN ircc, IRC_STATUS s);

VCSI_OBJECT init_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT server, 
			  VCSI_OBJECT port,
			  VCSI_OBJECT nick, 
			  VCSI_OBJECT name);

VCSI_OBJECT connect_irc_conn(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x);

VCSI_OBJECT disconnect_irc_conn(VCSI_CONTEXT vc, 
				VCSI_OBJECT x);

VCSI_OBJECT enable_status_queue_irc_conn(VCSI_CONTEXT vc, 
					 VCSI_OBJECT x);

VCSI_OBJECT disable_status_queue_irc_conn(VCSI_CONTEXT vc, 
					  VCSI_OBJECT x);

VCSI_OBJECT pop_status_queue_irc_conn(VCSI_CONTEXT vc,
                                      VCSI_OBJECT x);

#ifdef WITH_IRC_SECURE
VCSI_OBJECT secure_irc_conn(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y);

VCSI_OBJECT unsecure_irc_conn(VCSI_CONTEXT vc, 
			      VCSI_OBJECT x, 
			      VCSI_OBJECT y);
#endif

VCSI_OBJECT raw_irc_conn(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x, 
			 VCSI_OBJECT y);

VCSI_OBJECT quit_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y);

VCSI_OBJECT join_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y);

VCSI_OBJECT part_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y);

VCSI_OBJECT nick_irc_conn(VCSI_CONTEXT vc,
			  VCSI_OBJECT x,
			  VCSI_OBJECT y);

VCSI_OBJECT notice_irc_conn(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y, 
			    VCSI_OBJECT z);

VCSI_OBJECT privmsg_irc_conn(VCSI_CONTEXT vc, 
			     VCSI_OBJECT x, 
			     VCSI_OBJECT y, 
			     VCSI_OBJECT z);

VCSI_OBJECT me_irc_conn(VCSI_CONTEXT vc, 
			VCSI_OBJECT x, 
			VCSI_OBJECT y, 
			VCSI_OBJECT z);

VCSI_OBJECT ctcp_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y, 
			  VCSI_OBJECT z);

VCSI_OBJECT topic_irc_conn(VCSI_CONTEXT vc, 
			   VCSI_OBJECT args, 
			   int length);

VCSI_OBJECT names_irc_conn(VCSI_CONTEXT vc,
			   VCSI_OBJECT args,
			   int length);

VCSI_OBJECT kick_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT args, 
			  int length);

VCSI_OBJECT away_irc_conn(VCSI_CONTEXT vc,
			  VCSI_OBJECT args,
			  int length);

VCSI_OBJECT away_msg_irc_conn(VCSI_CONTEXT vc,
                              VCSI_OBJECT x);

VCSI_OBJECT back_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x);

VCSI_OBJECT ison_irc_conn(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x, 
			  VCSI_OBJECT y);

VCSI_OBJECT ping_reply_irc_conn(VCSI_CONTEXT vc, 
				VCSI_OBJECT x, 
				VCSI_OBJECT y,
				VCSI_OBJECT z);

VCSI_OBJECT version_reply_irc_conn(VCSI_CONTEXT vc, 
				   VCSI_OBJECT x, 
				   VCSI_OBJECT y,
				   VCSI_OBJECT z);

VCSI_OBJECT set_handler_irc_conn(VCSI_CONTEXT vc, 
				 VCSI_OBJECT x, 
				 VCSI_OBJECT y, 
				 VCSI_OBJECT z);

VCSI_OBJECT handle_one_event_irc_conn(VCSI_CONTEXT vc, 
				      VCSI_OBJECT x);

VCSI_OBJECT event_ready_irc_conn(VCSI_CONTEXT vc,
				 VCSI_OBJECT x);

VCSI_OBJECT command_parse_irc_conn(VCSI_CONTEXT vc,
				   VCSI_OBJECT args,
				   int length);


void irc_set_away(IRC_CONN ircc);

void irc_reset_idle(IRC_CONN ircc);

VCSI_OBJECT irc_connq(VCSI_CONTEXT vc, 
		      VCSI_OBJECT x);

VCSI_OBJECT connected_irc_connq(VCSI_CONTEXT vc, 
				VCSI_OBJECT x);

void print_irc_conn(VCSI_CONTEXT vc,
		    INFOSTD_DYN_STR res, 
		    VCSI_OBJECT obj);

void mark_irc_conn(VCSI_CONTEXT vc,
		   VCSI_OBJECT obj, 
		   int flag);

void free_irc_conn(VCSI_CONTEXT vc,
		   VCSI_OBJECT obj);

void irc_module_init(VCSI_CONTEXT vc);

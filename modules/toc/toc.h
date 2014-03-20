#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <vcsi.h>

#define TOC_VERSION "0x03"

#define TOC_TYPE_SIGNON    1
#define TOC_TYPE_DATA      2
#define TOC_TYPE_ERROR     3
#define TOC_TYPE_SIGNOFF   4
#define TOC_TYPE_KEEPALIVE 5

#define TOC_FLAPON "FLAPON\r\n\r\n"
#define TOC_ROAST "Tic/Toc"

#define TOC_SERVER_HOST "toc.oscar.aol.com"
#define TOC_SERVER_PORT 9898
#define TOC_AUTH_HOST "login.oscar.aol.com"
#define TOC_AUTH_PORT 5190
#define TOC_LANGUAGE "english"

#define TOC_STATE_OFFLINE 0
#define TOC_STATE_FLAPON 1
#define TOC_STATE_SIGNON_REQUEST 2
#define TOC_STATE_ONLINE 3
#define TOC_STATE_PAUSE 4

#define TOC_MAX_MSG_LEN    2048
#define TOC_HDR_LEN        6
#define TOC_HDR_SIGNON_LEN 8

#define TOC_UC_AOL		0x02
#define TOC_UC_ADMIN	        0x04
#define TOC_UC_UNCONFIRMED	0x08
#define TOC_UC_NORMAL	        0x10

/* Buddy List / Group Structures */
typedef struct str_toc_buddy {
  /* Name, normalized */
  char name[256];
  /* Nice name */
  char nice_name[256];
  
  /* Online or Offline */
  unsigned char online;

  /* Evil amount */
  int evil;

  /* Signon time */;
  time_t signon;

  /* Minutes Idle */
  int idle;

  /* Away? */
  unsigned char away;

  struct str_toc_buddy* next;
} *TOC_BUDDY;
#define SIZE_TOC_BUDDY (sizeof(struct str_toc_buddy))

typedef struct str_toc_buddy_group {
  char name[256];

  TOC_BUDDY buddies;
  struct str_toc_buddy_group* next;
} *TOC_BUDDY_GROUP;
#define SIZE_TOC_BUDDY_GROUP (sizeof(struct str_toc_buddy_group))

/* The connection structure */
typedef struct str_toc_conn {
  
  /* Connected? */
  unsigned char connected;

  /* Current state */
  unsigned char state;

  /* Current permission mode */
  unsigned char mode;

  /* Username and password */
  unsigned char username[256];
  unsigned char passwd[256];
  unsigned long code;
  
  /* Sequence number */
  unsigned short sequence;

  /* Buffers */
  unsigned char send_buf[2064];
  unsigned short send_len;

  unsigned char recv_buf[2064];
  unsigned short recv_len;

  unsigned char tmp_buf[2064];
  unsigned short tmp_len;

  /* Buddy Groups */
  TOC_BUDDY_GROUP buddies;

  /* Thee permit/deny lists */
  TOC_BUDDY_GROUP permit;
  TOC_BUDDY_GROUP deny;

  /* Handlers */
  VCSI_OBJECT on_im;
  VCSI_OBJECT on_update_buddy;

  /* Non fatal errors */
  VCSI_OBJECT on_error;

  /* The actual socket */
  int skt;

} *TOC_CONN;
#define SIZE_TOC_CONN (sizeof(struct str_toc_conn))

VCSI_USER_DEF toc_conn_type;
#define TOCC(x) ((TOC_CONN)(*x).data.userdef.ud_value)

enum toc_event_type {
  TOC_EV_UNKNOWN, TOC_EV_SIGNON_REQUEST, TOC_EV_SIGNON, TOC_EV_CONFIG, 
  TOC_EV_NICK, TOC_EV_IM, TOC_EV_UPDATE_BUDDY, TOC_EV_ERROR, TOC_EV_EVILED, 
  TOC_EV_CHAT_JOIN, TOC_EV_CHAT_IN, TOC_EV_CHAT_UPDATE_BUDDY, 
  TOC_EV_CHAT_INVITE, TOC_EV_CHAT_LEFT, TOC_EV_GOTO_URL, TOC_EV_DIR_STATUS, 
  TOC_EV_ADMIN_NICK_STATUS, TOC_EV_ADMIN_PASSWD_STATUS, TOC_EV_PAUSE, 
  TOC_EV_RVOUS_PROPOSE, TOC_EV_KEEPALIVE
};

typedef struct str_toc_event {
  enum toc_event_type type;
  /* The payload */
  char data[2048];
  /* Payload Length */
  int length;
  /*
    ...
  */
} *TOC_EVENT;
#define SIZE_TOC_EVENT (sizeof(struct str_toc_event))

/***** TOC Buddy Functions *****/
TOC_BUDDY toc_buddy_init(char* name);

TOC_BUDDY toc_buddy_update(TOC_BUDDY tocb, 
			   char* nice_name,
			   unsigned char online,
			   int evil,
			   time_t signon,
			   int idle,
			   unsigned char away);

TOC_BUDDY_GROUP toc_buddy_group_init(char* name);

TOC_BUDDY toc_buddy_group_add(TOC_BUDDY_GROUP tocg,
			      TOC_BUDDY tocb);

TOC_BUDDY_GROUP toc_buddy_group_add_group(TOC_CONN tocc,
					  TOC_BUDDY_GROUP tocg);

void toc_buddy_group_remove_buddy(TOC_BUDDY_GROUP tocg,
				  TOC_BUDDY tocb);

void toc_buddy_group_remove_all(TOC_BUDDY_GROUP tocg);


void toc_buddy_group_remove(TOC_BUDDY_GROUP tocg);

TOC_BUDDY_GROUP toc_buddy_group_find_group(TOC_CONN tocc, char* name);

TOC_BUDDY toc_buddy_group_find(TOC_BUDDY_GROUP tocg,
			       char* name);

TOC_BUDDY toc_buddy_group_find_all(TOC_CONN tocc,
				   char* name);

void toc_config_parse(TOC_CONN tocc, 
		      char* config);

void toc_config_save(TOC_CONN tocc);

TOC_BUDDY toc_update_buddy_parse(TOC_CONN tocc,
				 char* update);

/***** TOC Event Functions ****/

TOC_EVENT toc_read_event(TOC_CONN tocc);


/***** TOC Internal Functions *****/
const char* toc_normalize_username(char *name);

const char* toc_strip_html(char* text);

void toc_roast_password(TOC_CONN tocc);

unsigned char* toc_header_fill(TOC_CONN tocc,
			       unsigned char type,
			       int data_length);

unsigned char* toc_header_fill_signon(TOC_CONN tocc);

int toc_send(TOC_CONN tocc);

int toc_send_text(TOC_CONN tocc,
		  unsigned char* data);

int toc_send_printf(TOC_CONN tocc,
		    const char* format,
		    ...);

/***** VCSI Functions *****/
VCSI_OBJECT toc_handler_set(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x, 
			    VCSI_OBJECT y, 
			    VCSI_OBJECT z);

VCSI_OBJECT toc_handle_one_event(VCSI_CONTEXT vc,
				 VCSI_OBJECT x);

VCSI_OBJECT toc_event_ready(VCSI_CONTEXT vc,
			    VCSI_OBJECT x);

VCSI_OBJECT toc_init(VCSI_CONTEXT vc, 
		     VCSI_OBJECT username,
		     VCSI_OBJECT passwd);

VCSI_OBJECT toc_signon(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x);

VCSI_OBJECT toc_connect(VCSI_CONTEXT vc,
			VCSI_OBJECT x);


VCSI_OBJECT toc_disconnect(VCSI_CONTEXT vc,
			   VCSI_OBJECT x);

VCSI_OBJECT toc_gen_buddy_list(VCSI_CONTEXT vc,
			       VCSI_OBJECT x,
			       int mode);

VCSI_OBJECT toc_buddy_list(VCSI_CONTEXT vc,
			   VCSI_OBJECT x);

VCSI_OBJECT toc_buddy_list_online(VCSI_CONTEXT vc,
				  VCSI_OBJECT x);

VCSI_OBJECT toc_buddy_list_offline(VCSI_CONTEXT vc,
				   VCSI_OBJECT x);

VCSI_OBJECT toc_buddy_list_available(VCSI_CONTEXT vc,
				     VCSI_OBJECT x);

VCSI_OBJECT toc_buddy_list_away(VCSI_CONTEXT vc,
				VCSI_OBJECT x);

VCSI_OBJECT toc_normalize_name(VCSI_CONTEXT vc,
			       VCSI_OBJECT x);

VCSI_OBJECT toc_buddy_add(VCSI_CONTEXT vc,
			  VCSI_OBJECT x,
			  VCSI_OBJECT y);

VCSI_OBJECT toc_im(VCSI_CONTEXT vc,
		   VCSI_OBJECT x,
		   VCSI_OBJECT y,
		   VCSI_OBJECT z);

/***** Module Required Function *****/
void toc_print(VCSI_CONTEXT vc,
	       INFOSTD_DYN_STR res,
	       VCSI_OBJECT obj);

void toc_mark(VCSI_CONTEXT vc,
	      VCSI_OBJECT obj, 
	      int flag);

void toc_free(VCSI_CONTEXT vc,
	      VCSI_OBJECT obj);

void module_init(VCSI_CONTEXT vc);

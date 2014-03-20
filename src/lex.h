typedef struct str_vcsi_gstr {
   char* str;
   int len;
   int pos;
} *VCSI_GSTR;
#define SIZE_VCSI_GSTR (sizeof(struct str_vcsi_gstr))

typedef struct str_vcsi_lexnode {
   char* str;
   struct str_vcsi_lexnode* next;
} *VCSI_LEXNODE;
#define SIZE_VCSI_LEXNODE (sizeof(struct str_vcsi_lexnode))

typedef struct str_vcsi_lexlist {
  VCSI_LEXNODE list;
  VCSI_LEXNODE tail;
  VCSI_LEXNODE curr;
  
  VCSI_GSTR gstr;

  INFOSTD_DYN_STR buffer;
} *VCSI_LEXLIST;
#define SIZE_VCSI_LEXLIST (sizeof(struct str_vcsi_lexlist))
#define SIZE_VCSI_LEXLIST_BUFFER 255

/* Our parsing functions */
int vcsi_gstr_getc(VCSI_GSTR gs);
void vcsi_gstr_ugetc(VCSI_GSTR gs);
void lex_buf_insert(VCSI_LEXLIST lexlist, char c);
void lex_clear_buffer(VCSI_LEXLIST lexlist);
void lex_insert(VCSI_LEXLIST lexlist);
void lex_free(VCSI_LEXLIST lexlist);
void lex_parse(VCSI_LEXLIST lexlist);

VCSI_OBJECT lex_trans_quote(VCSI_CONTEXT vc, 
			    VCSI_LEXLIST lexlist, 
			    VCSI_LEXNODE list, 
			    char* str);
VCSI_OBJECT lex_make_vector(VCSI_CONTEXT vc, 
			    VCSI_LEXLIST lexlist,
			    VCSI_LEXNODE list);
VCSI_OBJECT parse_token(VCSI_CONTEXT vc, 
			VCSI_LEXLIST lexlist, 
			VCSI_LEXNODE list);
VCSI_OBJECT parse_ldot(VCSI_CONTEXT vc,
		       VCSI_LEXLIST lexlist,
		       VCSI_LEXNODE list);
VCSI_OBJECT parse_lparen(VCSI_CONTEXT vc, 
			 VCSI_LEXLIST lexlist,
			 VCSI_LEXNODE list);
VCSI_OBJECT parse_lexlist(VCSI_CONTEXT vc, 
			  VCSI_LEXLIST lexlist);
VCSI_OBJECT parse_text(VCSI_CONTEXT vc, 
		       char *text);

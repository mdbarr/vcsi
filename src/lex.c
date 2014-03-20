#include "vcsi.h"
/* Generic String get char, and unget char functions */
int vcsi_gstr_getc(VCSI_GSTR gs) {
  int c;

  if(!gs)
    return EOF;

  if(gs->pos < gs->len) {
    c = gs->str[gs->pos];
    gs->pos++;
    return c;
  }
  return EOF;

}

void vcsi_gstr_ugetc(VCSI_GSTR gs) {
  if(!gs)
    return;

  if(gs->pos > 0)
    gs->pos--;
}

/* Functions dealing with creation of Linked List of Tokens */
void lex_buf_insert(VCSI_LEXLIST lexlist, 
		    char c) {

  infostd_dyn_str_addchar(lexlist->buffer,c);
}

void lex_clear_buffer(VCSI_LEXLIST lexlist) {
  
  infostd_dyn_str_clear(lexlist->buffer);
}

void lex_insert(VCSI_LEXLIST lexlist) {

  VCSI_LEXNODE newnode;
#ifdef DEBUG
  printf("Lex: %s\n",lexlist->buffer->buff);
  fflush(stdout);
#endif

  newnode = MALLOC(SIZE_VCSI_LEXNODE);
  newnode->str = strdup(lexlist->buffer->buff);

  if(lexlist->list == NULL) {
    lexlist->list = newnode;
    lexlist->tail = lexlist->list;
    lexlist->list->next = NULL;
  } else {
    lexlist->tail->next = newnode;
    lexlist->tail = newnode;
    lexlist->tail->next = NULL;
  }
}

void lex_free(VCSI_LEXLIST lexlist) {

  VCSI_LEXNODE tmp;

  if(!lexlist)
    return;

  while(lexlist->list != NULL) {
    tmp = lexlist->list->next;
    FREE((char*)lexlist->list->str);
    FREE(lexlist->list);
    lexlist->list = tmp;
  }

  if(lexlist->gstr) {
    FREE(lexlist->gstr->str);
    FREE(lexlist->gstr);
  }

  if(lexlist->buffer)
    infostd_dyn_str_free(lexlist->buffer);

  FREE(lexlist);
}

/* Parsing related functions */
void lex_parse(VCSI_LEXLIST lexlist) {
  int c;
  int instr = 0;
  int esc = 0;

  lex_clear_buffer(lexlist);
  c = vcsi_gstr_getc(lexlist->gstr);
  while(c != EOF) {
    if(c == '\\') {
      esc = 1;
      lex_buf_insert(lexlist,c);
    } else if((c == '(' || c == ')') && !esc && !instr) {
      if(lexlist->buffer->length != 0) {
	lex_insert(lexlist);
	lex_clear_buffer(lexlist);
      }

      lex_buf_insert(lexlist,c);
      lex_insert(lexlist);
      lex_clear_buffer(lexlist);
    } else if(c == '\"' && !esc) {
      if(lexlist->buffer->length != 0 && !instr) {
	lex_insert(lexlist);
	lex_clear_buffer(lexlist);
      }
	  
      lex_buf_insert(lexlist,c);
      if(instr == 1)  {
	instr = 0;
	lex_insert(lexlist);
	lex_clear_buffer(lexlist);
      } else
	instr = 1;
	     
     } else if(c == '#' && lexlist->buffer->length == 0 && !esc) {
      lex_buf_insert(lexlist,c);
      c = vcsi_gstr_getc(lexlist->gstr);
      if(c == '(') {
	lex_buf_insert(lexlist,c);
	lex_insert(lexlist);
	lex_clear_buffer(lexlist);
      } else 
       	vcsi_gstr_ugetc(lexlist->gstr);
     } else if(c == '.' && lexlist->buffer->length == 0 && !esc) {
      lex_buf_insert(lexlist,c);

      c = vcsi_gstr_getc(lexlist->gstr);
      if(!isspace(c))
	lex_buf_insert(lexlist,c);
      else {
	if(c != EOF)
	  vcsi_gstr_ugetc(lexlist->gstr);

	lex_insert(lexlist);
	lex_clear_buffer(lexlist);	     
      }
       
     } else if(c == '\'' && lexlist->buffer->length == 0 && !esc) {
      lex_buf_insert(lexlist,c);	     
      lex_insert(lexlist);
      lex_clear_buffer(lexlist);	     
    } else if(c == '`' && lexlist->buffer->length == 0 && !esc) {
      lex_buf_insert(lexlist,c);
      lex_insert(lexlist);
      lex_clear_buffer(lexlist);
    } else if(c == ',' && lexlist->buffer->length == 0 && !esc) {
      lex_buf_insert(lexlist,c);
      c = vcsi_gstr_getc(lexlist->gstr);
      if(c == '@')
	lex_buf_insert(lexlist,c);
      else if(c != EOF)
	vcsi_gstr_ugetc(lexlist->gstr);
	     
      lex_insert(lexlist);
      lex_clear_buffer(lexlist);
    } else if(isspace(c) && !instr) {
      if(lexlist->buffer->length != 0) {
	lex_insert(lexlist);
	lex_clear_buffer(lexlist);
      }
    }      
    else
      lex_buf_insert(lexlist,c);
	
    if(esc && c != '\\')
      esc = 0;
        
    c = vcsi_gstr_getc(lexlist->gstr);
  }
  if(lexlist->buffer->length != 0)
    lex_insert(lexlist);
}

VCSI_OBJECT lex_trans_quote(VCSI_CONTEXT vc, 
			    VCSI_LEXLIST lexlist,
			    VCSI_LEXNODE list, 
			    char* str) {

  return cons(vc,make_symbol(vc,str),cons(vc,parse_token(vc,lexlist,list->next),NULL));
}

VCSI_OBJECT lex_make_vector(VCSI_CONTEXT vc, 
			    VCSI_LEXLIST lexlist,
			    VCSI_LEXNODE list) {
		       
  return list_vector(vc,parse_lparen(vc,lexlist, list->next));
}

VCSI_OBJECT parse_token(VCSI_CONTEXT vc, 
			VCSI_LEXLIST lexlist,
			VCSI_LEXNODE list) {
  int i;

  lexlist->curr = list;

  if(!list || !list->str)
    return NULL;
  if(list->str[0] == '(')
    return parse_lparen(vc,lexlist, list->next);
  if(!strcmp(list->str,"#("))
    return lex_make_vector(vc,lexlist, list);
  if(!strcmp(list->str,"\'"))
    return lex_trans_quote(vc,lexlist, list,"quote");
  if(!strcmp(list->str,"`"))
    return lex_trans_quote(vc,lexlist, list,"quasiquote");
  if(!strcmp(list->str,","))
    return lex_trans_quote(vc,lexlist, list,"unquote");
  if(!strcmp(list->str,",@"))
    return lex_trans_quote(vc,lexlist, list,"unquote-splicing");
  /* Parse of actual symbols into different type objects... */
  if(!strcmp(list->str,"#t"))
    return make_boolean(vc,1);
  if(!strcmp(list->str,"#f"))
    return make_boolean(vc,0);

  if(is_char(list->str))
    return make_char(vc,list->str);
  if(is_string(list->str))
    return make_string(vc,list->str);
   
  i = is_num(list->str);
  if(i == 1)
    return make_long_str(vc,list->str);
  else if(i == 2)
    return make_float_str(vc,list->str);
  else if(i == 3)
    return make_rat(vc,list->str);
  else if(i == 4)
    return make_num(vc,list->str);
   
  return make_symbol(vc,list->str);
}

VCSI_OBJECT parse_ldot(VCSI_CONTEXT vc,
		       VCSI_LEXLIST lexlist,
		       VCSI_LEXNODE list) {

  VCSI_OBJECT tmp;
  lexlist->curr = list;

  if(!list)
    error(vc,"bad dot syntax",NULL); /* c */
  if(!list->next)
    error(vc,"bad dot syntax",NULL); /* ) */

  if(list->next->str[0] != ')')
    error(vc,"bad dot syntax",NULL); /* ) */

  tmp = parse_token(vc,lexlist,list);
  lexlist->curr = list->next;;
  
  return tmp;
}

VCSI_OBJECT parse_lparen(VCSI_CONTEXT vc, 
			 VCSI_LEXLIST lexlist,
			 VCSI_LEXNODE list) {
  VCSI_OBJECT tmp1, tmp2;
  
  lexlist->curr = list;

  if(list == NULL || list->str[0] == ')')
    return NULL;
  
  /* Stupid C, and Normal Order make this necassary */
  tmp1 = parse_token(vc,lexlist,lexlist->curr);

  if(lexlist->curr != NULL && lexlist->curr->next != NULL && 
     !strcmp(lexlist->curr->next->str,".")) {
    tmp2 = parse_ldot(vc,lexlist,lexlist->curr->next->next);
  } else if(lexlist->curr != NULL && lexlist->curr->next != NULL)
    tmp2 = parse_lparen(vc,lexlist,lexlist->curr->next);
  else
    tmp2 = NULL;

  return cons(vc,tmp1,tmp2);
}

VCSI_OBJECT parse_lexlist(VCSI_CONTEXT vc, 
			  VCSI_LEXLIST lexlist) {
  VCSI_LEXNODE list;
  
  if(!lexlist)
    return NULL;
  
  list = lexlist->list;

  if(list == NULL)
    return NULL;
  if(!strcmp(list->str,"\'"))
    return lex_trans_quote(vc,lexlist, list,"quote");
  if(!strcmp(list->str,"#("))
    return lex_make_vector(vc,lexlist, list);
  if(!strcmp(list->str,"`"))
    return lex_trans_quote(vc,lexlist, list,"quasiquote");
  if(!strcmp(list->str,","))
    return lex_trans_quote(vc,lexlist, list,"unquote");
  if(!strcmp(list->str,",@"))
    return lex_trans_quote(vc,lexlist, list,"unquote-splicing");
  if(list->str[0] == '(')
    return parse_lparen(vc,lexlist, list->next);
  if(list->str[0] == ')')
    return error(vc,"Unmatched close parenthesis",NULL);
  if(!strcmp(list->str,"."))
    return error(vc,"bad dot syntax",NULL);

  return parse_token(vc,lexlist,list);
}

VCSI_OBJECT parse_text(VCSI_CONTEXT vc, 
		       char *text) {
  VCSI_OBJECT newobj;

  VCSI_LEXLIST lexlist = (VCSI_LEXLIST)MALLOC(SIZE_VCSI_LEXLIST);
  memset(lexlist,0,SIZE_VCSI_LEXLIST);

  lexlist->buffer = infostd_dyn_str_init(100);

  lexlist->gstr=(VCSI_GSTR)MALLOC(SIZE_VCSI_GSTR);
  memset(lexlist->gstr,0,SIZE_VCSI_GSTR);

  lexlist->gstr->len = strlen(text);
  lexlist->gstr->str=strdup(text);
  lexlist->gstr->pos=0;

  lex_parse(lexlist);

  newobj = parse_lexlist(vc,lexlist);

  lex_free(lexlist);

  return newobj;
}

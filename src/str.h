void str_init(VCSI_CONTEXT vc);
VCSI_OBJECT charq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT stringq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
int is_char(char* str);
VCSI_OBJECT make_char(VCSI_CONTEXT vc, char* str);
VCSI_OBJECT make_char_int(VCSI_CONTEXT vc, int i);
int is_string(char* str);
VCSI_OBJECT make_string(VCSI_CONTEXT vc, char* str);
VCSI_OBJECT make_raw_string(VCSI_CONTEXT vc, char* str);

VCSI_OBJECT chareq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT charless(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT chargreater(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT charlesseq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT chargreatereq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

VCSI_OBJECT chareqci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT charlessci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT chargreaterci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT charlesseqci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT chargreatereqci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

VCSI_OBJECT char_alpha(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT char_numeric(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT char_ws(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT char_upper(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT char_lower(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT char_int(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT int_char(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT chartoupper(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT chartolower(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT str_make(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT string(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT string_symbol(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_len(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_ref(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_set(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y, 
		       VCSI_OBJECT z);
VCSI_OBJECT string_eq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_eqci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_less(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_greater(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_lesseq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_greatereq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_lessci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_greaterci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_lesseqci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_greatereqci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_sub(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y, 
		       VCSI_OBJECT z);
VCSI_OBJECT string_sub_i(VCSI_CONTEXT vc, VCSI_OBJECT x, int y, int z);
VCSI_OBJECT string_append(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT string_list(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT list_string(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_copy(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_fill(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

/* extensions to string handling */
void str_ext_init(VCSI_CONTEXT vc);
VCSI_OBJECT string_downcase(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_upcase(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_eval(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT string_split(VCSI_CONTEXT vc, VCSI_OBJECT pat, VCSI_OBJECT str);
VCSI_OBJECT string_join(VCSI_CONTEXT vc,
			VCSI_OBJECT joiner,
			VCSI_OBJECT slist);
VCSI_OBJECT string_tokenize(VCSI_CONTEXT vc,        
			    VCSI_OBJECT tokens,
			    VCSI_OBJECT str);

/* String handler substitutes */
#ifndef HAVE_STRSEP
char *strsep(char **stringp, 
	     const char *delim);
#endif

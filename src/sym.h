VCSI_SYMBOL_TABLE symbol_table_init(void);
void symbol_table_free(VCSI_SYMBOL_TABLE st);

VCSI_SYMBOL sym_create(char* name);
void sym_insert(VCSI_SYMBOL_TABLE st, 
		VCSI_SYMBOL item);
void sym_remove(VCSI_SYMBOL_TABLE st, 
		VCSI_SYMBOL item);
void sym_print(VCSI_SYMBOL_TABLE st);
VCSI_SYMBOL sym_lookup(VCSI_SYMBOL_TABLE st,
		       char* name);
char* sym_add(VCSI_SYMBOL_TABLE st,
	      char* name);
void sym_free(VCSI_SYMBOL_TABLE st,
	      char* name);

int sym_r5rs_valid(char* name);

void sym_init(VCSI_CONTEXT vc);

VCSI_OBJECT symbolq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT make_symbol(VCSI_CONTEXT vc, char* str);
VCSI_OBJECT symbol_cons(VCSI_CONTEXT vc, char *name, VCSI_OBJECT vcell);
VCSI_OBJECT copy_symbol(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT symbol_to_string(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT make_gensym(VCSI_CONTEXT vc, VCSI_OBJECT x);

VCSI_OBJECT alloc_obj(VCSI_CONTEXT vc, VCSI_OBJECT_TYPE type);
void* dmalloc(int size);
void dfree(void* ptr);

VCSI_OBJECT errorq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT pairq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT nullq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT booleanq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT procq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT listq(VCSI_CONTEXT vc, VCSI_OBJECT obj);

VCSI_OBJECT eq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT eqv(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT memq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT r_equal(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT equal(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT member(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT assq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT assoc(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
int obj_compare_i(VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT obj_compare(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

VCSI_OBJECT protect_var(VCSI_CONTEXT vc, char* var);

VCSI_OBJECT make_protected(char* str);
void free_protected(VCSI_OBJECT x);

VCSI_OBJECT make_boolean(VCSI_CONTEXT vc, unsigned short val);

VCSI_OBJECT make_proc(VCSI_CONTEXT vc, 
		      char* name, 
		      VCSI_OBJECT_TYPE type, 
		      VCSI_OBJECT (*proc)());
VCSI_OBJECT make_internal(VCSI_CONTEXT vc, char *name);
VCSI_OBJECT make_special(VCSI_CONTEXT vc, char *name);
VCSI_OBJECT make_closure(VCSI_CONTEXT vc, 
			 VCSI_OBJECT code, 
			 VCSI_OBJECT env);
VCSI_OBJECT make_lambda(VCSI_CONTEXT vc, 
			VCSI_OBJECT args, 
			VCSI_OBJECT code);
VCSI_OBJECT make_thunk(VCSI_CONTEXT vc, 
		       VCSI_OBJECT exp, 
		       VCSI_OBJECT env);
VCSI_OBJECT make_quoted(VCSI_CONTEXT vc, 
			VCSI_OBJECT x);
VCSI_OBJECT make_list(VCSI_CONTEXT vc,
		      int count, ...);
VCSI_OBJECT make_syntax(VCSI_CONTEXT vc, 
			VCSI_OBJECT pattern,
			VCSI_OBJECT env);

VCSI_OBJECT check_pattern_validity(VCSI_CONTEXT vc,
				   VCSI_OBJECT pattern);

VCSI_OBJECT find_pattern_match(VCSI_CONTEXT vc, 
			       VCSI_OBJECT syntax,
			       VCSI_OBJECT pattern,
			       VCSI_OBJECT newenv);

VCSI_OBJECT list_pattern_match(VCSI_CONTEXT vc,
			       VCSI_OBJECT syntax,
			       VCSI_OBJECT pattern,
			       VCSI_OBJECT literals);

VCSI_OBJECT transform_template(VCSI_CONTEXT vc,
			       VCSI_OBJECT syntax,
			       VCSI_OBJECT pattern,			      
			       VCSI_OBJECT bindings,
			       VCSI_OBJECT newenv);

VCSI_OBJECT syntax_set_scope(VCSI_CONTEXT vc,
			     VCSI_OBJECT x,
			     VCSI_OBJECT env);

VCSI_OBJECT make_hash(VCSI_CONTEXT vc);
void free_hash(VCSI_CONTEXT vc,
	       VCSI_OBJECT x);
VCSI_HASH_TAB init_tab_hash(VCSI_OBJECT key,
			    VCSI_OBJECT info);
void free_tab_hash(VCSI_HASH_TAB vht);
unsigned int hash_hash(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x);
VCSI_OBJECT hash_lookup_i(VCSI_CONTEXT vc,
			  VCSI_OBJECT h,
			  VCSI_OBJECT key);
VCSI_OBJECT hash_lookup(VCSI_CONTEXT vc,
			VCSI_OBJECT h,
			VCSI_OBJECT key);
VCSI_OBJECT hash_set(VCSI_CONTEXT vc,
		     VCSI_OBJECT h,
		     VCSI_OBJECT key,
		     VCSI_OBJECT info);
VCSI_OBJECT hash_add(VCSI_CONTEXT vc,
		     VCSI_OBJECT h,
		     VCSI_OBJECT key,
		     VCSI_OBJECT info);
VCSI_OBJECT hash_delete(VCSI_CONTEXT vc,
			VCSI_OBJECT h,
			VCSI_OBJECT key);
VCSI_OBJECT hash_keys(VCSI_CONTEXT vc,
		      VCSI_OBJECT h);
void hash_mark(VCSI_CONTEXT vc,
	       VCSI_OBJECT h,
	       int flag);
int same_type_user_def(VCSI_CONTEXT vc, VCSI_OBJECT obj, VCSI_USER_DEF type);
VCSI_USER_DEF lookup_user_def(VCSI_CONTEXT vc,
			      char* name);
VCSI_USER_DEF make_user_def_type(VCSI_CONTEXT vc, 
				 char* name,
				 void (*udp)(),
				 void (*udm)(),
				 void (*udf)());
VCSI_OBJECT make_user_def(VCSI_CONTEXT vc, void* val, VCSI_USER_DEF ut);
int user_def_set_eval(VCSI_CONTEXT vc,
		      VCSI_OBJECT obj,
		      VCSI_OBJECT_TYPE type, 
		      VCSI_OBJECT (*proc)());
 

VCSI_OBJECT setvar(VCSI_CONTEXT vc, 
		   VCSI_OBJECT var,
		   VCSI_OBJECT val,
		   VCSI_OBJECT env); 

VCSI_OBJECT set_int_proc(VCSI_CONTEXT vc, 
			 char* name, 
			 VCSI_OBJECT_TYPE type, 
			 VCSI_OBJECT (*proc)()); 

VCSI_OBJECT envlookup(VCSI_CONTEXT vc, 
		      VCSI_OBJECT var, 
		      VCSI_OBJECT env); 

VCSI_OBJECT envlookup_str(VCSI_CONTEXT vc, 
			  char* var,
			  VCSI_OBJECT env);

VCSI_OBJECT extend_env(VCSI_CONTEXT vc, 
		       char* func_name,
		       VCSI_OBJECT vars,
		       VCSI_OBJECT vals, 
		       VCSI_OBJECT env); 

VCSI_OBJECT setcar(VCSI_CONTEXT vc, 
		   VCSI_OBJECT var, 
		   VCSI_OBJECT val, 
		   VCSI_OBJECT env); 

VCSI_OBJECT setcdr(VCSI_CONTEXT vc, 
		   VCSI_OBJECT var, 
		   VCSI_OBJECT val, 
		   VCSI_OBJECT env); 

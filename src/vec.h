void vec_init(VCSI_CONTEXT vc);
VCSI_OBJECT new_vector(VCSI_CONTEXT vc, int size, VCSI_OBJECT init);
VCSI_OBJECT make_vector(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT vector(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT vectorq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT vector_len(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT vector_ref(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT vector_set(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x, 
		       VCSI_OBJECT y, 
		       VCSI_OBJECT z);
VCSI_OBJECT list_vector(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT vector_list(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT vector_fill(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

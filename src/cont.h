void cont_init(VCSI_CONTEXT vc);

VCSI_OBJECT make_continuation(VCSI_CONTEXT vc, 
			      VCSI_OBJECT tmp,
			      VCSI_OBJECT exp);

int within_dynamic_extent(VCSI_DYNWIND context, VCSI_DYNWIND extent);
VCSI_OBJECT call_w_cc(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT cont_grow(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT cont_throw(VCSI_CONTEXT vc, VCSI_OBJECT tmp);

VCSI_OBJECT make_values(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT call_with_values(VCSI_CONTEXT vc, 
			     VCSI_OBJECT producer,
			     VCSI_OBJECT consumer);

VCSI_OBJECT dynamic_wind(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);

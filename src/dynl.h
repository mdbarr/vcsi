#ifdef WITH_DYNAMIC_LOAD
void dynl_init(VCSI_CONTEXT vc);
VCSI_OBJECT dyn_load_opt(VCSI_CONTEXT vc, VCSI_OBJECT x, int reload);
VCSI_OBJECT dyn_load(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT dyn_reload(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT module_loaded(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT module_list(VCSI_CONTEXT vc);
#endif

/* Garbage Collection functions */
void gc_mark(VCSI_CONTEXT vc, VCSI_OBJECT start, int flag);
void gc_mark_object(VCSI_CONTEXT vc, VCSI_OBJECT obj, int flag);
void gc_sweep(VCSI_CONTEXT vc);
void gc_collect_obj(VCSI_CONTEXT vc, VCSI_OBJECT pos);
void gc_collect(VCSI_CONTEXT vc);

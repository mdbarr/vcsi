#ifdef WITH_THREADS
void thread_init(VCSI_CONTEXT vc);

VCSI_OBJECT threadq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT thread_startedq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT thread_runningq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT thread_doneq(VCSI_CONTEXT vc, VCSI_OBJECT x);

VCSI_OBJECT make_thread(VCSI_CONTEXT vc, 
			VCSI_OBJECT code, 
			VCSI_OBJECT env);
void thread_free(VCSI_THREAD t);

void* thread_run(void* x);

VCSI_OBJECT thread_start(VCSI_CONTEXT vc, VCSI_OBJECT threadptr);
VCSI_OBJECT call_w_new_thread(VCSI_CONTEXT vc, 
			      VCSI_OBJECT args, 
			      VCSI_OBJECT env);
VCSI_OBJECT thread_join(VCSI_CONTEXT vc, VCSI_OBJECT x);
/*VCSI_OBJECT thread_kill(VCSI_CONTEXT vc, VCSI_OBJECT x);*/
VCSI_OBJECT thread_value(VCSI_CONTEXT vc, VCSI_OBJECT x);

VCSI_OBJECT mutexq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT make_mutex(VCSI_CONTEXT vc);
VCSI_OBJECT mutex_lock(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT mutex_unlock(VCSI_CONTEXT vc, VCSI_OBJECT x);
#endif

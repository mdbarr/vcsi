#include "vcsi.h"

#ifdef WITH_THREADS

void thread_init(VCSI_CONTEXT vc) {

  set_int_proc(vc,"thread?",PROC1,threadq);
  set_int_proc(vc,"thread-started?",PROC1,thread_startedq);
  set_int_proc(vc,"thread-running?",PROC1,thread_runningq);
  set_int_proc(vc,"thread-done?",PROC1,thread_doneq);

  set_int_proc(vc,"make-thread",PROCENV,make_thread);

  set_int_proc(vc,"thread-start",PROC1,thread_start);
  set_int_proc(vc,"thread-join",PROC1,thread_join);
  /*set_int_proc(vc,"thread-kill",PROC1,thread_kill);*/
  set_int_proc(vc,"thread-value",PROC1,thread_value);

  set_int_proc(vc,"call-with-new-thread",PROCENV,call_w_new_thread);

  set_int_proc(vc,"mutex?",PROC1,mutexq);
  set_int_proc(vc,"make-mutex",PROC0,make_mutex);
  set_int_proc(vc,"mutex-lock",PROC1,mutex_lock);
  set_int_proc(vc,"mutex-unlock",PROC1,mutex_unlock);

}

VCSI_OBJECT threadq(VCSI_CONTEXT vc, 
		    VCSI_OBJECT x) {

  if(TYPEP(x,THREAD))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT thread_startedq(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x) {

  check_arg_type(vc,x,THREAD,"thread-started?");

  if(THREADPTR(x)->status != VCSI_THREAD_NOT_STARTED)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT thread_runningq(VCSI_CONTEXT vc, 
			    VCSI_OBJECT x) {

  check_arg_type(vc,x,THREAD,"thread-running?");

  if(THREADPTR(x)->status == VCSI_THREAD_RUNNING)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT thread_doneq(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x) {

  check_arg_type(vc,x,THREAD,"thread-done?");

  if(THREADPTR(x)->status == VCSI_THREAD_DONE)
    return THREADPTR(x)->retval;
  return vc->false;
}  

VCSI_OBJECT make_thread(VCSI_CONTEXT vc, 
			VCSI_OBJECT code, 
			VCSI_OBJECT env) {
  VCSI_OBJECT tmp;
  VCSI_THREAD thr_tmp;
   
  tmp = alloc_obj(vc,THREAD);

  thr_tmp = (VCSI_THREAD) MALLOC(SIZE_VCSI_THREAD);
  memset(thr_tmp,0,SIZE_VCSI_THREAD);

  thr_tmp->thread = (pthread_t*)MALLOC(sizeof(pthread_t));

  /* Find master context for cloning */
  vc = vcsi_context_find_master(vc);

  thr_tmp->vc = vcsi_clone_context(vc);
  thr_tmp->code = code;
  thr_tmp->env = env;
  thr_tmp->status = VCSI_THREAD_NOT_STARTED;

  /* Add to Thread list */
  pthread_mutex_lock(vc->thread_mutex);

  thr_tmp->next = vc->thread_list;
  vc->thread_list = thr_tmp;  

  pthread_mutex_unlock(vc->thread_mutex);

  THREADPTR(tmp) = thr_tmp;

  return tmp;
}

void thread_free(VCSI_THREAD t) {
  if(!t)
    return;
  /* Free the thread */
  FREE(t->thread);
  
  /* Free the VCSI_CONTEXT */
  vcsi_context_free(t->vc);
  
  /* Free the VCSI_THREAD object */
  FREE(t);
}

void* thread_run(void* x) {
  VCSI_CONTEXT vc;
  VCSI_THREAD tmp;
  VCSI_OBJECT p;
  struct sigaction sa; 
  sa.sa_flags = 0; 

  tmp = (VCSI_THREAD)x;
    
  /* Block certain signals */
  sigemptyset (&sa.sa_mask);
  sigaddset(&sa.sa_mask,SIGABRT);
  sigaddset(&sa.sa_mask,SIGALRM);
  sigaddset(&sa.sa_mask,SIGINT);
  sigprocmask(SIG_BLOCK,&sa.sa_mask,NULL); 

  /* Clone the Context */
  vc = tmp->vc;

  /* Setup the environment */
  if(tmp->env)
    DYNROOT(vc->root_wind) = tmp->env;

  /* setup the stack */
  DYNSTCK(vc->root_wind) = &p;
  DYNSTCKE(vc->root_wind) = &p;

  /* Execute the code */
  if(!sigsetjmp(DYNERR(vc->root_wind),1))
    tmp->retval = eval(vc,tmp->code,DYNROOT(vc->root_wind));
  else
    tmp->retval = vc->errobj;
  
  vc->got_error=0;

  tmp->status = VCSI_THREAD_DONE;
  
  /*pthread_exit((void*)0);*/

  return (void*)tmp->retval;
}

VCSI_OBJECT thread_start(VCSI_CONTEXT vc, 
			 VCSI_OBJECT t) {

  VCSI_THREAD tmp;

  check_arg_type(vc,t,THREAD,"thread-start");

  tmp = THREADPTR(t);

  /* Check the status */
  if(tmp->status != VCSI_THREAD_NOT_STARTED)
    error(vc,"thread has already been started",t);

  /* Set the Thread as running */
  tmp->status = VCSI_THREAD_RUNNING;

  /* Start the Thread */
  if(pthread_create(tmp->thread,NULL,thread_run,(void*)tmp)) {
    tmp->status = VCSI_THREAD_DONE;
    error(vc,"couldn't start thread",t);
  }

  return t;  
}

VCSI_OBJECT call_w_new_thread(VCSI_CONTEXT vc, 
			      VCSI_OBJECT args, 
			      VCSI_OBJECT env) {

  VCSI_OBJECT tmp;
  tmp = make_thread(vc,CAR(args),env);

  if(tmp) {
    if((tmp = thread_start(vc,tmp)) != NULL)
      return tmp;
  }
  return vc->false;
}

VCSI_OBJECT thread_join(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {

  VCSI_THREAD tmp;

  check_arg_type(vc,x,THREAD,"thread-join");
   
  tmp = THREADPTR(x);
   
  if(pthread_join(*tmp->thread,NULL) == 0)
    return tmp->retval;
  return vc->false;   
}

/*
VCSI_OBJECT thread_kill(VCSI_CONTEXT vc, 
			VCSI_OBJECT x) {
  
  VCSI_THREAD tmp;

  check_arg_type(vc,x,THREAD,"thread-kill");
   
  tmp = THREADPTR(x);
   
  if(pthread_kill(*tmp->thread,SIGINT) == 0) {
    tmp->status = VCSI_THREAD_DONE;
    tmp->retval = vc->false;
    return vc->true;
  }
  return vc->false;   
}
*/

VCSI_OBJECT thread_value(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x) {
  
  check_arg_type(vc,x,THREAD,"thread-value");
  if(THREADPTR(x)->status != VCSI_THREAD_DONE)
    error(vc,"thread still running",x);
  
  return THREADPTR(x)->retval;
}



VCSI_OBJECT mutexq(VCSI_CONTEXT vc, 
		   VCSI_OBJECT x) {

  if(TYPEP(x,MUTEX))
    return vc->true;
  return vc->false;
}

VCSI_OBJECT make_mutex(VCSI_CONTEXT vc) {

  VCSI_OBJECT tmp = alloc_obj(vc,MUTEX);
  MUTEXPTR(tmp) = (pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
  pthread_mutex_init(MUTEXPTR(tmp),NULL);

  return tmp;
}

VCSI_OBJECT mutex_lock(VCSI_CONTEXT vc, 
		       VCSI_OBJECT x) {

  check_arg_type(vc,x,MUTEX,"mutex-lock");

  if(pthread_mutex_lock(MUTEXPTR(x)) == 0)
    return vc->true;
  return vc->false;
}

VCSI_OBJECT mutex_unlock(VCSI_CONTEXT vc, 
			 VCSI_OBJECT x) {

  check_arg_type(vc,x,MUTEX,"mutex-unlock");

  if(pthread_mutex_unlock(MUTEXPTR(x)) == 0)
    return vc->true;
  return vc->false;
}
#endif

#include "vcsi.h"
/* Garbage Collection Stuff */
void gc_mark_object(VCSI_CONTEXT vc, 
		    VCSI_OBJECT obj, 
		    int flag) {
  int i;

 gc_mark_obj_loop:   

#ifdef DEBUG
    printf("Marking Object %s\n",print_obj(vc,vc->result,obj,1));
#endif

  if(obj == NULL)
    return;

  if(obj->gc == flag)
    return;

  switch TYPE(obj) {
  case FREED:
    break;
  case SYMBOL:
    obj->gc = flag;

    if(SCOPE(obj))
      gc_mark(vc,SCOPE(obj),flag);

    obj = VCELL(obj);
    goto gc_mark_obj_loop;
    break;
  case CONS:
    obj->gc = flag;
    gc_mark(vc,CAR(obj),flag);
    gc_mark(vc,CDR(obj),flag);
    break;
  case CLOSURE:
  case MACRO:
    obj->gc = flag;
    if(CLSCODE(obj))
      gc_mark(vc,CLSCODE(obj),flag);

    if(CLSENV(obj)) {
      obj = CLSENV(obj);
      goto gc_mark_obj_loop;
    }
    break;
  case SYNTAX:
    obj->gc = flag;
    if(LITERALS(obj))
      gc_mark(vc,LITERALS(obj),flag);
    if(PATTERNS(obj))
      gc_mark(vc,PATTERNS(obj),flag);
    if(SYNENV(obj))
      gc_mark(vc,SYNENV(obj),flag);
    break;
  case PROMISED:
  case THUNK:
    obj->gc = flag;
    if(THNKENV(obj)) {
      THNKENV(obj)->gc = flag;
      gc_mark(vc,THNKENV(obj),flag);
    }
  case PROMISEF:
    obj->gc = flag;
    if(THNKEXP(obj)) {
      obj = THNKEXP(obj);
      goto gc_mark_obj_loop;
    }
    break;
  case VECTOR:
    obj->gc = flag;
    for(i=0;i<VSIZE(obj);i++) {
      if(VELEM(obj)[i] != NULL)
	gc_mark(vc,VELEM(obj)[i],flag);
    }
    break;
  case CONTINUATION:
    obj->gc = flag;
    if(CONTEXP(obj)) {
      obj = CONTEXP(obj);
      goto gc_mark_obj_loop;
    }
  case VALUES:
    obj->gc = flag;
    if(VVALS(obj)) {
      obj = VVALS(obj);
      goto gc_mark_obj_loop;
    }
    break;
  case REGEXPN:
    obj->gc = flag;
    break;
     
#ifdef WITH_THREADS
  case THREAD:
    obj->gc = flag;
    if(THREADPTR(obj)->status == VCSI_THREAD_DONE) {
      obj = THREADPTR(obj)->retval;
      goto gc_mark_obj_loop;
    } else {
      gc_mark(vc,THREADPTR(obj)->code,flag);
      gc_mark(vc,THREADPTR(obj)->env,flag);
    }
    break;
#endif
  case HASH:
    obj->gc=flag;
    hash_mark(vc,obj,flag);
    break;

  case USERDEF:
    obj->gc=flag;
    if(UDTYPE(obj) && UDTYPE(obj)->ud_mark)
      UDTYPE(obj)->ud_mark(vc,obj,flag);
    if(UDTYPE(obj) && UDTYPE(obj)->ud_eval)
      gc_mark(vc,UDTYPE(obj)->ud_eval,flag);
    break;

  default:
    obj->gc = flag;
    break;
  }
}

void gc_mark(VCSI_CONTEXT vc, 
	     VCSI_OBJECT start, 
	     int flag) {

  VCSI_OBJECT tmp=NULL, var=NULL;
 
  if(!start)
    return;

  for(tmp=start;TYPEP(tmp,CONS);tmp=CDR(tmp)) {
    tmp->gc = flag;

    var = CAR(tmp);

    gc_mark_object(vc,var,flag);
  }

  if(tmp != NULL)
    gc_mark_object(vc,tmp,flag);
}

void gc_mark_stack(VCSI_CONTEXT vc, 
		   VCSI_OBJECT* start, 
		   VCSI_OBJECT* end) {
  VCSI_OBJECT* tmp;
  VCSI_OBJECT p;
  long i;
  long c;
   
  if(start > end) {
    tmp = start;
    start = end;
    end = tmp;
  }
  c = end - start;

  for(i=0;i<c;i++) {
    p = start[i];
    if((p >= vc->heap) && (p <= vc->heap_end) && 
       (((((char*)p) - ((char*)vc->heap)) % SIZE_VCSI_OBJECT)) == 0 
       && !TYPEP(p,FREED))
      gc_mark(vc,p,1);
  }
}

void gc_sweep(VCSI_CONTEXT vc) {
  VCSI_OBJECT pos;
   
  for(pos=vc->heap;pos<=vc->heap_end;pos++) {
    if(TYPEP(pos,FREED))
      continue;
#ifdef WITH_THREADS
    else if(TYPEP(pos,THREAD) && THREADPTR(pos)->status!=VCSI_THREAD_DONE)
      continue;
#endif
    else if(pos->gc == 0) {
      vc->gc_collected++;
      gc_collect_obj(vc,pos);
      CDR(pos) = vc->freeobj;
      vc->freeobj = pos;
      pos->type = FREED;
    }
    pos->gc = 0;
  }
}

void gc_collect_obj(VCSI_CONTEXT vc, 
		    VCSI_OBJECT pos) {
   
  if(TYPEP(pos,STRING) && STR(pos) != NULL) {
    FREE(STR(pos));
  }
  else if(TYPEP(pos,SYMBOL) && SNAME(pos) != NULL) {
    sym_free(vc->symbol_table,SNAME(pos));
  }
  else if(TYPEP(pos,PORTI) || TYPEP(pos,PORTO) || TYPEP(pos,PORTB)) {
    close_port(vc,pos);	     
    FREE(PORTP(pos));
  }
  else if(TYPEP(pos,CONTINUATION)) {
    FREE(CONTJMP(pos));
  }
#ifdef WITH_THREADS
  else if(TYPEP(pos,THREAD)) {
    thread_free(THREADPTR(pos));
    THREADPTR(pos)=NULL;   
  } else if(TYPEP(pos,MUTEX)) {
    FREE(MUTEXPTR(pos));
    MUTEXPTR(pos)=NULL;
  }
#endif
  else if(TYPEP(pos,REGEXPN)) {
    regfree(REGEXPNV(pos));
    FREE(REGEXPNV(pos));
  }
  else if(TYPEP(pos,HASH)) {
    free_hash(vc,pos);
  }
  else if(TYPEP(pos,USERDEF)) {
    if(UDTYPE(pos) && UDTYPE(pos)->ud_free) {
      UDTYPE(pos)->ud_free(vc,pos);
    }
  }
  
  /* clear the object entirely */
  memset(pos,0,SIZE_VCSI_OBJECT);
}

void gc_collect(VCSI_CONTEXT vc) {
  VCSI_CONTEXT_LIST ltmp;
  VCSI_DYNWIND dtmp;
#ifdef WITH_THREADS
  VCSI_THREAD thr_tmp, prev;

  /* Lock the heap */
  pthread_mutex_lock(vc->gc_critical_mutex);
   
  /* block signals? */  

  /* Mark all the Threads and their stacks */
  for(prev=NULL,thr_tmp = vc->thread_list;
      thr_tmp;
      prev=thr_tmp,thr_tmp=thr_tmp->next) {

    if(thr_tmp->status != VCSI_THREAD_DONE) {
      gc_mark(vc,thr_tmp->code,1);
      gc_mark(vc,thr_tmp->env,1);
    } else { 
      /* Remove from the thread list */
      pthread_mutex_lock(vc->thread_mutex);
      
      if(thr_tmp == vc->thread_list)
	vc->thread_list = thr_tmp->next;
      else
	prev->next = thr_tmp->next;

      pthread_mutex_unlock(vc->thread_mutex);
    }
  }
#endif  

  /* Mark the objects to save */
  gc_mark(vc,DYNROOT(vc->root_wind),1);

  /* Mark our current stack - 
     using local pointer since it is in the same thread */
  gc_mark_stack(vc,DYNSTCK(vc->root_wind),DYNSTCKE(vc->root_wind));

  /* Mark the stack of all context, and their sub-winds */
  for(ltmp=vc->context_list;ltmp;ltmp=ltmp->next) {
    for(dtmp=ltmp->context->root_wind;dtmp;dtmp=dtmp->next)
      gc_mark_stack(vc,DYNSTCK(dtmp),DYNSTCKE(dtmp));
  }
  /* Make sure the default ports get saved */
  vc->port_in->gc = 1;
  vc->port_out->gc = 1;
   
  /* Sweep away the crap */
  gc_sweep(vc);
   
#ifdef WITH_THREADS
  pthread_mutex_unlock(vc->gc_critical_mutex);
#endif
}

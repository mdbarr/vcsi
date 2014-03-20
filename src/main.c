#include "vcsi.h"

#ifdef WITH_READLINE

#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif /* HAVE_READLINE_READLINE_H */

#ifdef HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif /* HAVE_READLINE_HISTORY_H */

#endif /* WITH_READLINE */

#define EMACS_ESC 27

int t_on;
FILE * transcipt;
static int emacs_mode = 0;
sigjmp_buf abort_jmp;
VCSI_CONTEXT vcsi_global_context;
VCSI_SYMBOL_TABLE vcsi_global_symbol_table;

void sig_abort(int sig) {    
  /* Return to our abort context */ 
  siglongjmp(abort_jmp,1); 
}  

void sig_init(void)  { 
  struct sigaction sa; 
  sa.sa_flags = 0; 

  /* Unblock any blocked signals we care about */
  sigemptyset (&sa.sa_mask);
  sigaddset(&sa.sa_mask,SIGINT);
  sigaddset(&sa.sa_mask,SIGABRT);
  sigaddset(&sa.sa_mask,SIGALRM);
  sigprocmask(SIG_UNBLOCK,&sa.sa_mask,NULL); 
 
  /* Set the abort signal handlers */
  sa.sa_handler = sig_abort; 
  sigemptyset (&sa.sa_mask); 
  sigaction(SIGINT, &sa, NULL); 

  sa.sa_handler = sig_abort;  
  sigemptyset (&sa.sa_mask);  
  sigaction(SIGABRT, &sa, NULL);

  sa.sa_handler = sig_abort;  
  sigemptyset (&sa.sa_mask);  
  sigaction(SIGALRM, &sa, NULL);
} 

int exists(char *s) {
  struct stat st;
  if ((!s)||(!*s)) return 0;
  if (stat(s,&st)==-1) return 0;
  return 1;
}

VCSI_OBJECT transcript_on(VCSI_CONTEXT vc, 
			  VCSI_OBJECT x) {
  if(!TYPEP(x,STRING))
    return error(vc,"object passed to transcript-oin",x);
  if(SLEN(x) == 0)
    return error(vc,"invalid filename",NULL);
  transcipt = fopen(STR(x),"w");
  if(transcipt == NULL)
    return error(vc,"unable to open file",x);
  t_on = 1;
  return vc->true;
}

VCSI_OBJECT transcript_off(VCSI_CONTEXT vc) {
  fclose(transcipt);
  t_on = 0;
  return NULL;
}

char* sym_gen(char* name, 
	      int state) {

  static VCSI_SYMBOL tmp;
  static int i,len;
  char* match;
   
  if(state == 0) {
    len = strlen(name);
    tmp = vcsi_global_symbol_table->table;
    i = 0;
  }
   
  while(i < vcsi_global_symbol_table->count && tmp) {
    if(!strncasecmp(tmp->name,name,len) && tmp->ref > 0 &&
       envlookup_str(vcsi_global_context,tmp->name,
		     vcsi_global_context->the_global_env)) {

      match = strdup(tmp->name);
      i++;
      tmp = tmp->next;
      return match;
    } else {
      i++;
      tmp = tmp->next;
    }
  }
  return (char*)NULL;
}


void rep_loop_emacs(VCSI_CONTEXT vc) {
  char* res;

  fprintf(PORTFP(vc->port_curr_out),"\n       Vaguely Coherent Scheme Interpreter\n              Ver. %sE\n\n",vcsi_version); 
  fflush(PORTFP(vc->port_curr_out)); 
  while(1) 
    { 
      if(sigsetjmp(abort_jmp,1))  
	{ 
	  sig_init(); 
	  fprintf(PORTFP(vc->port_curr_out),"\n;Abort\n"); 
	  continue; 
	} 
     
      fprintf(PORTFP(vc->port_curr_out)," ]=> "); 
      fflush(PORTFP(vc->port_curr_out)); 
      if(get_pmatch(vc,PORTFP(vc->port_curr_in)) == -1) 
	break; 
      if(t_on) 
	fprintf(transcipt," ]=> %s\n",vc->input->buff); 
      res = rep(vc,vc->input->buff); 
      if(vc->got_error)
	{
	  fprintf(PORTFP(vc->port_curr_out),"\n;Error: %s. See errobj for more details.\n",STR(CAR(VCELL(vc->errobj))));
	}
      else 
	fprintf(PORTFP(vc->port_curr_out),"\n%s\n",res); 
      fflush(PORTFP(vc->port_curr_out)); 
      if(t_on) 
	fprintf(transcipt,"%s\n",res);
    } 
}

void rep_loop(VCSI_CONTEXT vc) {
  char *tmp = NULL, *res;
#ifdef WITH_READLINE
  char *cont, *h_last=NULL;
#endif

  fprintf(PORTFP(vc->port_curr_out),"\n       [31mV[0maguely [31mC[0moherent [31mS[0mcheme [31mI[0mnterpreter\n              Ver. %s\n\n",vcsi_version);
  
  while(1) {

    if(sigsetjmp(abort_jmp,1)) {
      sig_init();
      fprintf(PORTFP(vc->port_curr_out),"\n[31m;Abort[0m\n");
      continue;
    }
     
#ifdef WITH_READLINE	      
    rl_instream = PORTFP(vc->port_curr_in);
    rl_outstream = PORTFP(vc->port_curr_out);
      
    tmp = readline(" ]=> ");
#else
    fprintf(PORTFP(vc->port_curr_out)," [01m]=> [0m");
	
    if(get_pmatch(vc,PORTFP(vc->port_curr_in)) == -1)
      break;
    tmp = vc->input->buff;
#endif

    if(t_on)
      fprintf(transcipt," ]=> %s\n",tmp);

#ifdef WITH_READLINE
    if(!tmp)
      break;

    while(!pmatchq(tmp)) { 
      cont = readline("     "); 
      if(!cont) {
	tmp = NULL;
	fprintf(PORTFP(vc->port_curr_out),"\n");
	break;
      }

      res = (char*)MALLOC(strlen(tmp) + strlen(cont) + 2);
      memset(res,0,strlen(tmp) + strlen(cont) + 2);
      if(strlen(tmp) > 0) {
	strcpy(res,tmp);
	strcat(res," ");
      }

      if(strlen(cont) > 0)
	strcat(res,cont);
      tmp = res;
    } 

    if(!tmp)
      continue;

    if(*tmp) { /* Must strdup to prevent xmalloc errors */
      if(h_last) {
	if(strcmp(h_last,tmp)!=0) {
	  h_last = strdup(tmp);
	  add_history(h_last); 
	}
      } else {
	h_last = strdup(tmp);
	add_history(h_last); 
      }
    }	  
    if(t_on)
      fprintf(transcipt," ]=> %s\n",vc->input->buff);

    res = rep(vc,tmp);

    free(tmp);

    tmp = NULL;
#else

    res = rep(vc,tmp);
#endif

    if(vc->got_error) {
      fprintf(PORTFP(vc->port_curr_out),"[31m;Error:[0m %s. See errobj for more details.\n",STR(CAR(VCELL(vc->errobj))));
    }
    else
      fprintf(PORTFP(vc->port_curr_out),"%s\n",res);

    if(t_on)
      fprintf(transcipt,"%s\n",res);
  }
}

void sh_rep_loop(VCSI_CONTEXT vc, char *filename) {
  FILE* fp;
  char crap[256];
  char *tmp;
   
  fp = fopen(filename,"r");
  fscanf(fp,"%s",crap);

  if(sigsetjmp(abort_jmp,1))  
    quit(vc);

  while(get_pmatch(vc,fp) != -1) {
    tmp = rep(vc,vc->input->buff);
    if(t_on)
      fprintf(transcipt,"%s\n",tmp);
  }
}

void pipe_rep_loop(VCSI_CONTEXT vc, char *filename) {
  FILE* fp;
  char *tmp;

  fp = fopen(filename,"r");

  if(sigsetjmp(abort_jmp,1))  
    quit(vc);

  printf(";;     Vaguely Coherent Scheme Interpreter\n;;            Ver. %s\n;;\n",vcsi_version); 

  while(get_pmatch(vc,fp) != -1) {
    if(!(strlen(vc->input->buff) == 1 && vc->input->buff[0] == ' ')) {
      printf("]=> %s\n", vc->input->buff);
      tmp = rep(vc,vc->input->buff);
      if(vc->got_error) {
	printf("; Error: %s.\n",STR(CAR(VCELL(vc->errobj))));
      }
      printf("%s\n",tmp);
    }
  }
}

/* Main... */
int main(int argc, char** argv) {
  VCSI_CONTEXT vc;
  t_on = 0;

#ifdef WITH_READLINE
  rl_readline_name = "vcsi";
   
  rl_completion_entry_function = (rl_compentry_func_t*)sym_gen;

  rl_completer_word_break_characters = " \t\n\"\\$;|&{(";
#endif
   
  vc = vcsi_init(0);
  vcsi_global_context = vc;
  vcsi_global_symbol_table = vc->symbol_table;
   
  set_int_proc(vc,"transcript-on",PROC1,transcript_on);
  set_int_proc(vc,"transcript-off",PROC0,transcript_off);
   
  sig_init();

  if(argc == 2) {
    if(!strcmp(argv[1],"-emacs")) { /* emacs mode switch */
      emacs_mode = 1;
    } else if(exists(argv[1])) { /* shebang notation */
      sh_rep_loop(vc,argv[1]);
      quit(vc);
    }
  } else if(argc == 3) { /* piped ouput from a file */
    if(!strncmp(argv[1],"-p",2) && exists(argv[2])) { 
      pipe_rep_loop(vc,argv[2]);
      quit(vc);
    }
  }
 
  if(getenv("EMACS")) /* running inside emacs? */
    emacs_mode=1;
  
  if(emacs_mode)
    rep_loop_emacs(vc); 
  else 
    rep_loop(vc);
  quit(vc);
  
  return 0;
}

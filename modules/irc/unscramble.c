/* Pathetic word unscrambler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <vcsi.h>

const char module_name[] = "unscramble";

#define WORDS "/usr/dict/words"

VCSI_OBJECT unscramble(VCSI_CONTEXT vc,
		       VCSI_OBJECT x) {
  FILE *f;
  char word[100], w2[100];
  char* sc;
  int slen,wlen,i,j,c;

  check_arg_type(vc,x,STRING,"unscramble");

  sc=STR(x);
  slen=SLEN(x);

  f=fopen(WORDS,"r");
  if(!f)
    return vc->false;

  while(fgets(word,100,f)) {
    if(word[strlen(word)-1] == '\n')
      word[strlen(word)-1]='\0';

      wlen=strlen(word);
      strncpy(w2,word,100);

      if(slen==wlen) {
	for(i=0,c=0;i<slen;i++) {
	  for(j=0;j<wlen;j++) {
	    if(tolower(sc[i])==tolower(word[j])) {
	      c++;
	      word[j] = '*';
	      j=wlen;
	    }
	  }

	  if(c==wlen) {
	    fclose(f);
	    return make_string(vc,w2);
	  }
	}	
      }                
  }
  
  fclose(f);
  return vc->false;
}

void module_init(VCSI_CONTEXT vc) { 
  /* eadityr --> dietary */
  /* geieiulnd --> guideline */
  set_int_proc(vc,"unscramble",PROC1,unscramble);
}

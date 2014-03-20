#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "lzw.h"

char* dictionary[256]; /* Max size of a char */
int dict_len = 0;

char tmp_buf[1024];
char tmp_dict[1024];
char buf[1024];

void dict_init(void)
{
   int i;
   char buf[2];
   char c;

   for(i=0;i<1024;i++)
     dictionary[i] = NULL;
   
   for(i=0,c=' ';c<='z';i++,c++)
     {
	buf[0] = c;
	buf[1] = 0;
	
	dictionary[i] = (char*)strdup(buf);
     }
   dict_len = i;
}

int dict_lookup(char* s)
{
   int i;
   for(i=0;i<dict_len;i++)
     {
	if(!strcmp(dictionary[i],s))
	  return i;
     }
   return -1;
}

void dict_insert(char* s)
{
   dictionary[dict_len] = (char*)strdup(s);
   dict_len++;
   if(dict_len > 255)
     printf("Dictionary size exceeded");
}

void buf_cat(char* s,int c)
{
   int i;
   
   i = strlen(s);
   s[i] = c;
   s[i+1] = 0;
}

void dict_print(void)
{
   int i;   
   for(i=0;i<dict_len;i++)
     printf("%s\n",dictionary[i]);
}

char* lzw_compress(char* s)
{
   int slen;
   int x, i, j, c;

   bzero(buf,1024);
   
   dict_init();
   
   slen = strlen(s);
   
   for(x=0,i=0;i<slen;x++)
     {	
	bzero(tmp_buf,1024);
	bzero(tmp_dict,1024);
	
	for(j=i;j<slen;j++)
	  {
	     buf_cat(tmp_dict,s[j]);
	     if(dict_lookup(tmp_dict) >= 0)
	       {
		  buf_cat(tmp_buf,s[j]);
		  i++;
	       }
	     else
	       break;
	  }
	dict_insert(tmp_dict);
	
	c = dict_lookup(tmp_buf);
	buf[x] = c+1;
     }
   buf[x] = 0;
   return (char*)strdup(buf);
}

char* lzw_decompress(char* s)
{
   int i, c, x, slen;
   
   dict_init();
      
   bzero(buf,1024);
   
   slen = strlen(s);
   
   bzero(tmp_buf,1024);
   bzero(tmp_dict,1024);
   
   for(i=0;i<slen;i++)
     {
	x = s[i];
       
	bzero(tmp_buf,1024);
	
	if(x > dict_len)
	  return NULL;
	
	strcat(buf,dictionary[x-1]);
	buf_cat(tmp_dict,dictionary[x-1][0]);
	if(dict_lookup(tmp_dict) == -1)
	  {
	     dict_insert(tmp_dict);
	     bzero(tmp_buf,1024);
	     strcpy(tmp_buf,tmp_dict);
	     bzero(tmp_dict,1024);
	     c = strlen(tmp_buf)-1;
	     strcat(tmp_dict,dictionary[x-1]);

	  }
     }
   return (char*)strdup(buf);
}

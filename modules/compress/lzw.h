void dict_init(void);
int dict_lookup(char* s);
void dict_insert(char* s);
void buf_cat(char* s,int c);
char* lzw_compress(char* s);
char* lzw_decompress(char* s);

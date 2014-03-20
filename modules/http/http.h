VCSI_OBJECT make_http_request(VCSI_CONTEXT vc);
void print_http_request(VCSI_CONTEXT vc ,INFOSTD_DYN_STR res, VCSI_OBJECT obj);
void mark_http_request(VCSI_CONTEXT vc, VCSI_OBJECT obj, int flag);
void free_http_request(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT eval_http_request(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);

VCSI_OBJECT http_simple_get(VCSI_CONTEXT vc, VCSI_OBJECT x);

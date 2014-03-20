/* Initialization and utilization functions */
typedef struct str_vcsi_context *VCSI_CONTEXT;

VCSI_CONTEXT vcsi_init(unsigned long heap_size);

char* rep(VCSI_CONTEXT vc, char* input);

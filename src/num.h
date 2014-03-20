void num_init(VCSI_CONTEXT vc);
int is_num(char* str);
VCSI_OBJECT check_exact(VCSI_CONTEXT vc, char* str, int* e);
VCSI_OBJECT make_num(VCSI_CONTEXT vc, char* str);
VCSI_OBJECT make_long(VCSI_CONTEXT vc, long num);
VCSI_OBJECT make_long_str(VCSI_CONTEXT vc, char* str);
VCSI_OBJECT make_float(VCSI_CONTEXT vc, float num);
VCSI_OBJECT make_float_str(VCSI_CONTEXT vc, char* s);
VCSI_OBJECT make_rat_int(VCSI_CONTEXT vc, long numer, long denom);
VCSI_OBJECT make_rat(VCSI_CONTEXT vc, char* str);
VCSI_OBJECT numberq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT realq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT ratq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT intq(VCSI_CONTEXT vc, VCSI_OBJECT obj);
VCSI_OBJECT get_nums(VCSI_CONTEXT vc, 
		     VCSI_OBJECT x, 
		     VCSI_OBJECT y, 
		     float *i, 
		     float *j, 
		     float* l, 
		     float* m);
VCSI_OBJECT multiply(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT divide(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT add(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT subtract(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT lessthan(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT greaterthan(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT lesseq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT greatereq(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT exactq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT inexactq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT zeroq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT posq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT negq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT oddq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT evenq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_max(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT num_min(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT num_abs(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_quotient(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT num_remainder(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT num_modulo(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
int compute_gcd(int a, int b);
VCSI_OBJECT num_gcd(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT num_lcm(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT num_numer(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_denom(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_floor(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_ceil(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_trunc(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_round(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_rationalize(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT num_exp(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_log(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_sin(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_cos(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_tan(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_asin(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_acos(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_atan(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT num_sqrt(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_expt(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT exact_inexact(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT inexact_exact(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT num_string(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT string_num(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);

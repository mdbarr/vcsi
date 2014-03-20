void port_init(VCSI_CONTEXT vc);

VCSI_OBJECT make_port_fp(VCSI_CONTEXT vc, 
			 FILE* fp, 
			 char* name, 
			 VCSI_OBJECT_TYPE type);
VCSI_OBJECT make_port(VCSI_CONTEXT vc, 
		      char* path, 
		      VCSI_OBJECT_TYPE type);

VCSI_OBJECT close_port(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT call_w_i_file(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT call_w_o_file(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT portiq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT portoq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT get_curr_in(VCSI_CONTEXT vc);
VCSI_OBJECT get_curr_out(VCSI_CONTEXT vc);
VCSI_OBJECT with_i_f_file(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT with_o_t_file(VCSI_CONTEXT vc, VCSI_OBJECT args, VCSI_OBJECT env);
VCSI_OBJECT open_input_file(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT open_output_file(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT close_input_port(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT close_output_port(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT open_input_pipe(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_read(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_read_string(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_read_char(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_peek_char(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT eofq(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_char_ready(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_write(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT port_display(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
VCSI_OBJECT port_newline(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT port_write_char(VCSI_CONTEXT vc, VCSI_OBJECT args, int length);
			    

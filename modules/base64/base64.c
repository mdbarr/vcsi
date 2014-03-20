#include <vcsi.h>
#include <stdint.h>
#include <stdlib.h>
#include "base64.h"

const char module_name[] = "base64";

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static int mod_table[] = {0, 2, 1};

VCSI_OBJECT base64_encode(VCSI_CONTEXT vc, VCSI_OBJECT x) {
  VCSI_OBJECT ret = NULL;
  unsigned char* encoded_data = NULL;
  int input_length, output_length;
  char* data;
  int i, j;

  if(!TYPEP(x,STRING))
    return error(vc,"Invalid type to base64-encode",x);

  data = STR(x);
  input_length = strlen(data);
  output_length = ((input_length - 1) / 3) * 4 + 4;
  
  encoded_data = MALLOC(output_length + 1);
  memset(encoded_data,0,output_length + 1);
  
  for(i = 0, j = 0; i < input_length;) {  
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  for(i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[output_length - 1 - i] = '=';
  
  ret = make_string(vc,encoded_data);
  FREE(encoded_data);

  return ret;
}

VCSI_OBJECT base64_decode(VCSI_CONTEXT vc, VCSI_OBJECT x) {
  VCSI_OBJECT ret = NULL;
  unsigned char *decoded_data = NULL;
  int input_length, output_length;
  char decoding_table[256];
  char* data;
  int i, j;

  if(!TYPEP(x,STRING))
    return error(vc,"Invalid type to base64-decode",x);

  data = STR(x);
  input_length = strlen(data);

  if(input_length % 4 != 0)
    return error(vc,"Invalid data to base64-decode",x);

  for(i = 0; i < 0x40; i++)
    decoding_table[encoding_table[i]] = i;

  output_length = input_length / 4 * 3;
  if(data[input_length - 1] == '=') (output_length)--;
  if(data[input_length - 2] == '=') (output_length)--;

  decoded_data = MALLOC(output_length + 1);
  memset(decoded_data,0,output_length + 1);

  for(i = 0, j = 0; i < input_length;) {
    uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

    uint32_t triple = (sextet_a << 3 * 6)
      + (sextet_b << 2 * 6)
      + (sextet_c << 1 * 6)
      + (sextet_d << 0 * 6);

    if(j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if(j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if(j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  ret = make_string(vc,decoded_data);
  FREE(decoded_data);
  
  return ret;
}

void module_init(VCSI_CONTEXT vc) { 
   set_int_proc(vc,"base64-encode",PROC1,base64_encode);
   set_int_proc(vc,"base64-decode",PROC1,base64_decode);
}


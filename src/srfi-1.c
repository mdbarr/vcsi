/* Scheme Request for Implementation 1 - List Library */
#include "vcsi.h"
void srfi_1_init(VCSI_CONTEXT vc) {
  load_library_file(vc,"srfi-1.scm");
}

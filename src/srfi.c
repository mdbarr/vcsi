/* Scheme Request for Implementation */
#include "vcsi.h"

void srfi_init(VCSI_CONTEXT vc) {
  /* SRFI-0 cond-expand */
  srfi_0_init(vc);

  /* SRFI-1 List Library */
  srfi_1_init(vc);

  /* SRFI-8 Receive */
  srfi_8_init(vc);

  /* SRFI-28 Format Strings */
  srfi_28_init(vc);
}

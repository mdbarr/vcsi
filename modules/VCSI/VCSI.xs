#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../../src/vcsi_api.h"

MODULE = VCSI		PACKAGE = VCSI		

VCSI_CONTEXT
vcsi_init(heap_size)
	long heap_size

char *
rep(vc,input)
	VCSI_CONTEXT vc
	char *	input


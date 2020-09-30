#include "cpu/exec/template-start.h"

#define instr stos

make_helper(concat(stos_, SUFFIX)) {

	swaddr_write (reg_l(R_EDI),DATA_BYTE,reg_l(R_EAX));
	if (cpu.DF == 0)reg_l (R_EDI) += DATA_BYTE;
	else reg_l (R_EDI) -= DATA_BYTE;

	print_asm("stos");
	return 1;
}



#include "cpu/exec/template-end.h"

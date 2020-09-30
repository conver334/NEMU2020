#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_, SUFFIX)) {
	swaddr_t s1 = REG(R_EAX), s2 = swaddr_read(reg_l(R_EDI), DATA_BYTE);
	uint32_t res = s1 - s2;
	if (cpu.DF == 0) reg_l(R_EDI) += DATA_BYTE;
	else reg_l(R_EDI) -= DATA_BYTE;
	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = s1 < s2;
	cpu.ZF = !res;
    cpu.OF = ((s1 >> len) != (s2 >> len) && (s2 >> len) == cpu.SF);
	int i, flag=1;
	cpu.SF = res>>len;
    for(i=0;i<8;i++){
        if(res&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
	print_asm("scas%s", str(SUFFIX));

	return 1;
}

#include "cpu/exec/template-end.h"
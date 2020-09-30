#include "cpu/exec/template-start.h"

#define instr cmps

make_helper(concat(cmps_, SUFFIX)){
	DATA_TYPE src,dest;

	src = swaddr_read (reg_l (R_ESI),DATA_BYTE);
	dest = swaddr_read (reg_l (R_EDI),DATA_BYTE);

	DATA_TYPE result = dest - src;
	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = dest < src;
	cpu.SF= result >> len;
    	int s1,s2;
	s1=dest>>len;
	s2=src>>len;
	cpu.OF=(s1 != s2 && s2 == cpu.SF) ;
	cpu.ZF=!result;
	int i, flag=1;
    for(i=0;i<8;i++){
        if(result&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
	if (cpu.DF == 0)REG (R_EDI) += DATA_BYTE,REG (R_ESI) += DATA_BYTE;
	else REG (R_EDI) -= DATA_BYTE,REG (R_ESI) -= DATA_BYTE;
	print_asm("cmps");
	return 1;
}



#include "cpu/exec/template-end.h"

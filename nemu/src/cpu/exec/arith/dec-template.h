#include "cpu/exec/template-start.h"

#define instr dec

static void do_execute () {
	DATA_TYPE result = op_src->val - 1;
	OPERAND_W(op_src, result);

	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = op_src->val < 1;
	cpu.SF=result >> len;

    int op1,op2;
	op1=op_src->val>>len;
	op2=0;
    cpu.OF=(op1 != op2 && op2 == cpu.SF) ;
	cpu.ZF=!result;

    int i, flag=1;
    for(i=0;i<8;i++){
        if(result&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
	print_asm_template1();
}

make_instr_helper(rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
#endif

#include "cpu/exec/template-end.h"

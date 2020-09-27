#include "cpu/exec/template-start.h"

#define instr sub

static void do_execute() {
	DATA_TYPE result = op_dest->val - op_src->val;
    OPERAND_W(op_dest, result);

	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = op_dest->val < op_src->val;
	cpu.SF=result >> len;
    int op1,op2;
	op1=op_dest->val>>len;
	op2=op_src->val>>len;
    cpu.OF=(op1 != op2 && op2 == cpu.SF) ;
    cpu.ZF=!result;
    
	int i, flag=1;
    for(i=0;i<8;i++){
        if(result&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
	print_template2();
}
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)


#include "cpu/exec/template-end.h"

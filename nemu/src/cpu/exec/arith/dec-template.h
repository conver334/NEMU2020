#include "cpu/exec/template-start.h"

#define instr dec

static void do_execute () {
	DATA_TYPE result = op_src->val - 1;
	OPERAND_W(op_src, result);

	/* TODO: Update EFLAGS. */
	// panic("please implement me");
	
    
    cpu.CF = (op_src -> val < 1);

	int len = DATA_BYTE * 8;
	cpu.SF = result >> (len - 1);

	int sign1, sign2;
    sign1 = (op_src -> val) >> (len - 1);
    sign2 = 0;
    cpu.OF = (sign1 != sign2 && sign2 == cpu.SF);
    
    cpu.ZF = !result;
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result & 1);

	print_asm_template1();
}

make_instr_helper(rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
#endif

#include "cpu/exec/template-end.h"

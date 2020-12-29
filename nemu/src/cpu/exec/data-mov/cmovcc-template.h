#include "cpu/exec/template-start.h"

make_helper(concat(cmove_, SUFFIX)){
	int len = concat(decode_rm2r_, SUFFIX)(eip + 1);
	if (cpu.ZF == 1) OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
	return len+1;
}
make_helper(concat(cmovle_,SUFFIX)){
	int len = concat(decode_rm2r_, SUFFIX)(eip + 1);
	if ((cpu.SF ^ cpu.OF) | cpu.ZF) OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
	return len+1;
}

#include "cpu/exec/template-end.h"
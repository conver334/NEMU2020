#include "cpu/exec/template-start.h"

#define instr shr

static void do_execute () {
	DATA_TYPE src = op_src->val;
	DATA_TYPE dest = op_dest->val;

	uint8_t count = src & 0x1f;
	

	/* TODO: Update EFLAGS. */
	
	cpu.CF= dest&1;
	dest >>= count;
	OPERAND_W(op_dest, dest);
	int len = (DATA_BYTE << 3) - 1;
	cpu.OF=src>>len;
	cpu.SF=dest >> len;
	cpu.ZF=!dest;
	
    int i, flag=1;
    for(i=0;i<8;i++){
        if(dest&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
	print_asm_template2();
	print_asm_template2();
}

make_instr_helper(rm_1)
make_instr_helper(rm_cl)
make_instr_helper(rm_imm)

#include "cpu/exec/template-end.h"

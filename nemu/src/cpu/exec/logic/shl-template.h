#include "cpu/exec/template-start.h"

#define instr shl

static void do_execute () {
	DATA_TYPE src = op_src->val;
	DATA_TYPE dest = op_dest->val;

	

	/* TODO: Update EFLAGS. */
	int len = (DATA_BYTE << 3) - 1;
	cpu.CF=dest>>len;
	uint8_t count = src & 0x1f;
	dest <<= count;
	OPERAND_W(op_dest, dest);
	
	cpu.OF=((dest>>len)!=cpu.CF);
	cpu.SF=dest >> len;
	cpu.ZF=!dest;
	
    int i, flag=1;
    for(i=0;i<8;i++){
        if(dest&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
	print_asm_template2();
}

make_instr_helper(rm_1)
make_instr_helper(rm_cl)
make_instr_helper(rm_imm)

#include "cpu/exec/template-end.h"

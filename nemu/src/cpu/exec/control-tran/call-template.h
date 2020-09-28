#include "cpu/exec/template-start.h"

#define instr call

make_helper (concat(call_i_, SUFFIX))
{
	//concat string splicing  
	//eip where the program is now
	int len = concat(decode_i_, SUFFIX) (eip + 1);
	//esp stack top
	reg_l (R_ESP) -= DATA_BYTE;
	//push the return address onto the stack [1.the stack needle moves down,then write ].
	swaddr_write (reg_l (R_ESP) , 4 , cpu.eip + len);
	printf("%x\n",cpu.eip + len);
	//after decode
	//op_src immediate information in the instruction
	//op_dest register information
	DATA_TYPE_S jumplen = op_src->val;
	//jump to the target address
	cpu.eip +=jumplen;
	print_asm("call %x",cpu.eip + 1 + len);
	return len + 1;
}
make_helper (concat(call_rm_, SUFFIX))
{
	int len = concat(decode_rm_, SUFFIX) (eip + 1);
	reg_l (R_ESP) -= DATA_BYTE;
	swaddr_write (reg_l (R_ESP) , 4 , cpu.eip + len);
	DATA_TYPE_S jumplen = op_src->val;
	cpu.eip = jumplen - len - 1;
	print_asm("call %x",jumplen);
	return len + 1;
}



#include "cpu/exec/template-end.h"

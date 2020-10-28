#include "cpu/exec/template-start.h"

#define instr je

static void do_execute(){
    DATA_TYPE_S displacement = op_src -> val;
    print_asm("je: %x", cpu.eip + 1 + DATA_BYTE + displacement);
    if(cpu.ZF == 1) cpu.eip += displacement;
}
make_instr_helper(i)

// make_helper(concat(je_i_, SUFFIX)) {
// 	int len = concat(decode_i_, SUFFIX) (eip + 1);
// 	print_asm("je 0x%x", (DATA_TYPE_S)op_src->imm + cpu.eip + len + 1);
// 	if(cpu.ZF == 1) cpu.eip += (DATA_TYPE_S)op_src->imm;
// 	return len + 1;
// }

#include "cpu/exec/template-end.h"
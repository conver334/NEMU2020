#include "cpu/exec/template-start.h"

#define instr adc

static void do_execute(){
    DATA_TYPE result = op_dest -> val + op_src -> val + cpu.CF;
    int len = DATA_BYTE * 8;
    cpu.CF = (result < (op_dest -> val));
    cpu.SF = result >> (len - 1);
    int sign1, sign2;
    sign1 = (op_dest -> val) >> (len - 1);
    sign2 = (op_src -> val) >> (len - 1);
    cpu.OF = (sign1 == sign2 && sign1 != cpu.SF);
    cpu.ZF = !result;
    OPERAND_W(op_dest, result);
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result & 1);
    print_asm_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
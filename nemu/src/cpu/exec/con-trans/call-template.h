#include "cpu/exec/template-start.h"

#define instr call

make_helper (concat(call_i_, SUFFIX)){
    int displacement_len = concat(decode_i_, SUFFIX) (eip + 1);
    reg_l(R_ESP) -= DATA_BYTE;
    swaddr_write(reg_l(R_ESP), 4, cpu.eip + displacement_len + 1);
    DATA_TYPE_S displacement = op_src -> val;
    print_asm("call %x", cpu.eip + 1 + displacement_len + displacement);
    cpu.eip += displacement;
    return 1 + displacement_len;
}

make_helper(concat(call_rm_, SUFFIX)){
    int displacement_len = concat(decode_rm_, SUFFIX) (eip + 1);
    reg_l(R_ESP) -= DATA_BYTE;
    MEM_W(reg_l(R_ESP), cpu.eip + displacement_len + 1);
    cpu.eip = (DATA_TYPE_S) op_src -> val - displacement_len - 1;
    print_asm("call %s", op_src -> str);
    return 1 + displacement_len;
}


#include "cpu/exec/template-end.h"
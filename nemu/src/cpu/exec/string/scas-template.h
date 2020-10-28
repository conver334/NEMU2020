#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_m_, SUFFIX)){
    current_sreg = R_ES;

    uint32_t num1 = REG(R_EAX);
    uint32_t num2 = MEM_R(reg_l(R_EDI));

    uint32_t result = num1 - num2;

    if(cpu.DF == 0) reg_l(R_EDI) += DATA_BYTE;
    else reg_l(R_EDI) -= DATA_BYTE;

    int len = DATA_BYTE * 8;
    cpu.ZF = !result;
    cpu.SF = result >> (len - 1);
    cpu.CF = (num1 < num2);
    int sign1 = (num1 >> (len - 1)), sign2 = (num2 >> (len - 1));
    cpu.OF = (sign1 != sign2 && sign2 == cpu.SF);
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    result &= 1;
    cpu.PF = !result;

    print_asm("scas");
    return 1;

}

#include "cpu/exec/template-end.h"
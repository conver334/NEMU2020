// #include "cpu/exec/template-start.h"

// #define instr scas

// make_helper(concat(scas_, SUFFIX)) {
// 	current_sreg = R_ES;

// 	DATA_TYPE dest = REG(R_EAX);
// 	DATA_TYPE src = MEM_R(cpu.edi);;
// 	DATA_TYPE result = dest - src;

// 	update_eflags_pf_zf_sf((DATA_TYPE_S)result);
// 	cpu.eflags.CF = result > dest;
// 	cpu.eflags.OF = MSB((dest ^ src) & (dest ^ result));

// 	cpu.edi += (cpu.eflags.DF ? -DATA_BYTE : DATA_BYTE);

// 	print_asm("scas" str(SUFFIX) " %%es:(%%edi),%%%s", REG_NAME(R_EAX));
// 	return 1;
// }

// #include "cpu/exec/template-end.h"
//--my pa2
#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_m_, SUFFIX)){
    current_sreg = R_ES;

    uint32_t num1 = REG(R_EAX);
    uint32_t num2 = MEM_R(reg_l(R_EDI));

    uint32_t result = num1 - num2;

    if(cpu.eflags.DF == 0) reg_l(R_EDI) += DATA_BYTE;
    else reg_l(R_EDI) -= DATA_BYTE;

    int len = DATA_BYTE * 8;
    cpu.eflags.ZF = !result;
    cpu.eflags.SF = result >> (len - 1);
    cpu.eflags.CF = (num1 < num2);
    int sign1 = (num1 >> (len - 1)), sign2 = (num2 >> (len - 1));
    cpu.eflags.OF = (sign1 != sign2 && sign2 == cpu.eflags.SF);
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    result &= 1;
    cpu.eflags.PF = !result;

    print_asm("scas");
    return 1;

}

#include "cpu/exec/template-end.h"
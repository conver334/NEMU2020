// #include "cpu/exec/template-start.h"

// #define instr cmp

// static void do_execute() {
// 	//cmp opr1 opr2  =>  opr1-opr2
// 	DATA_TYPE result = op_dest->val - op_src->val;
// 	int len = (DATA_BYTE << 3) - 1;
// 	//carry flag
// 	cpu.CF = op_dest->val < op_src->val;
// 	//sf==1 opr1 < opr2
// 	cpu.SF=result >> len;
// 	//overflow the answer's symbol dosen't equal to the first
//     int op1,op2;
// 	op1=op_dest->val>>len;
// 	op2=op_src->val>>len;
//     cpu.OF=(op1 != op2 && op2 == cpu.SF) ;
// 	cpu.ZF=!result;
//     //zf == 1 opr1==opr2	
// 	result ^= result >>4;
// 	result ^= result >>2;
// 	result ^= result >>1;
// 	cpu.PF=!(result & 1);
//     print_template2();
// }

// #if DATA_BYTE == 2 || DATA_BYTE == 4
// make_instr_helper(si2rm)
// #endif

// make_instr_helper(i2a)
// make_instr_helper(i2rm)
// make_instr_helper(r2rm)
// make_instr_helper(rm2r)

// #include "cpu/exec/template-end.h"
#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
	DATA_TYPE result = op_dest->val - op_src->val;
	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = op_dest->val < op_src->val;
	cpu.SF=result >> len;
    	int s1,s2;
	s1=op_dest->val>>len;
	s2=op_src->val>>len;
    	cpu.OF=(s1 != s2 && s2 == cpu.SF) ;
    	cpu.ZF=!result;
	result ^= result >>4;
	result ^= result >>2;
	result ^= result >>1;
	cpu.PF=!(result & 1);
	print_template2();
}
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)


#include "cpu/exec/template-end.h"

#include "cpu/exec/template-start.h"

#define instr je


static void do_execute() {
	DATA_TYPE_S value = op_src->val;
	if (cpu.ZF == 1)cpu.eip +=value;
}
make_instr_helper(i)


#include "cpu/exec/template-end.h"

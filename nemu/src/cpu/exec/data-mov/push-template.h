#include "cpu/exec/template-start.h"

#define instr push

static void do_execute(){
    //1. R[ESP]<-- R[ESP]-4
    //2. send a word or double word from the register to the stack indicated by ESP
    reg_l(R_ESP)-=4;
    swaddr_write(reg_l (R_ESP),4,op_src->val);
    print_template1();
}

make_instr_helper(rm)
make_instr_helper(r)
#include "cpu/exec/template-end.h"
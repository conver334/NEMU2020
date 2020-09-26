#include "cpu/exec/template-start.h"
#define instr test

static void do_execute(){
    //The TEST operation sets the flags CF and OF to zero.
    //The SF is set to the most significant bit of the result of the AND
    //If the result is 0,ZF is set tp 1,otherwise set to 0.
    //The parity flag is set to XNOR of the least significant byte of the result, 1 if the number of ones in that byte is even,0 otherwise. 
    cpu.CF=0;
    cpu.OF=0;
    DATA_TYPE result = op_dest->val & op_src->val;
    int len = (DATA_BYTE<<3)-1;
    cpu.SF=result>>len;
    cpu.ZF=!result;
    int i, flag=1;
    for(i=0;i<8;i++){
        if(result&(1<<i))flag=flag^1;
    }
    cpu.PF=flag;
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
#include "cpu/exec/template-end.h"
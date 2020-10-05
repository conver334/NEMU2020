#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
	//general registers.
     union{
        union {
			uint32_t _32;
			uint16_t _16;
			uint8_t _8[2];
		} gpr[8];

	/* Do NOT change the order of the GPRs' definitions. */

		struct{
            uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
		};
    };
	//status and instruction register 
	union 
	{
		//bit field
		//total length:32 bit
		struct{
			uint32_t CF:	1;
			uint32_t :	1;// this bit is filled with 0
			//unnamed bit field,used for filling or adjusting position
			uint32_t PF:	1;
			uint32_t :	1;
			uint32_t AF:	1;
			uint32_t :	1;
			uint32_t ZF:	1;
			uint32_t SF:	1;
			uint32_t TF:	1;
			uint32_t IF:	1;
			uint32_t DF:	1;
			uint32_t OF:	1;
			uint32_t IOPL:	2;
			uint32_t NT:	1;
			uint32_t :	1;
			uint32_t RF:	1;
			uint32_t VM:	1;
			uint32_t :	14;
		};
		uint32_t eflags;
	};
	//instruction pointer
	swaddr_t eip;
     

} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
	assert(index >= 0 && index < 8);
	return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];
extern const char* regef[];

extern uint32_t get_reg_by_str(bool *success, char *e);

#define cf_sub(dest, src) cpu.CF = dest < src
#define cf_add(dest, src) cpu.CF = ((dest + src) < dest)
#define sf_add(dest, src) cpu.SF = ((dest + src) < 0)
#define sf_sub(dest, src) cpu.SF = ((dest - src) < 0)
#define of_add(dest, src) cpu.OF = (sign_bit32(dest) == sign_bit32(src) && sign_bit32(dest) != cpu.SF) ;
#define of_sub(dest, src) cpu.OF= (sign_bit32(dest) != sign_bit32(src) && sign_bit32(src) == cpu.SF) ;
#define zf_add(dest, src) cpu.ZF = ((dest + src) == 0)
#define zf_sub(dest, src) cpu.ZF = ((dest - src) == 0)
#define pf_add(dest, src) int NEVER_USE = (dest + src); NEVER_USE ^= NEVER_USE >>4; NEVER_USE ^= NEVER_USE >>2; NEVER_USE ^= NEVER_USE >>1; cpu.PF=!(NEVER_USE & 1);
#define pf_sub(dest, src) int NEVER_USE = (dest - src); NEVER_USE ^= NEVER_USE >>4; NEVER_USE ^= NEVER_USE >>2; NEVER_USE ^= NEVER_USE >>1; cpu.PF=!(NEVER_USE & 1);
#define eadd(dest, src) cf_add(dest, src); sf_add(dest, src); of_add(dest, src); zf_add(dest, src); pf_add(dest, src);
#define esub(dest, src) cf_sub(dest, src); sf_sub(dest, src); of_sub(dest, src); zf_sub(dest, src); pf_sub(dest, src);

#endif

#include "monitor/monitor.h"
#include "cpu/helper.h"
#include "monitor/watchpoint.h"
#include <setjmp.h>
#include "cpu/reg.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

int nemu_state = STOP;

int exec(swaddr_t);

char assembly[80];
char asm_buf[128];
void raise_intr(uint8_t);
/* Used with exception handling. */
jmp_buf jbuf;

static inline void push_r2stack(int val){
	current_sreg = R_SS;
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, val);
}

void raise_intr(uint8_t NO){
	Assert((NO<<3)<=cpu.idtr.limit,"Wrong Interrupt id!");
	Gate_Descriptor now_gate;
	idt_desc = &now_gate;

	lnaddr_t addr = cpu.idtr.base + (NO << 3);
	idt_desc -> part1 = lnaddr_read(addr, 4);
	idt_desc -> part2 = lnaddr_read(addr + 4, 4);

	push_r2stack(cpu.eflags);
	push_r2stack(cpu.cs.selector);
	push_r2stack(cpu.eip);

	cpu.cs.selector = idt_desc -> selector;

	current_sreg = R_CS;
	sreg_load(R_CS);
	cpu.eip = cpu.cs.base + idt_desc -> offset1 + (idt_desc -> offset2 << 16);

	longjmp(jbuf, 1);

	return ;
}

void print_bin_instr(swaddr_t eip, int len) {
	int i;
	int l = sprintf(asm_buf, "%8x:   ", eip);
	for(i = 0; i < len; i ++) {
		l += sprintf(asm_buf + l, "%02x ", instr_fetch(eip + i, 1));
	}
	sprintf(asm_buf + l, "%*.s", 50 - (12 + 3 * len), "");
}

/* This function will be called when an `int3' instruction is being executed. */
void do_int3() {
	printf("\nHit breakpoint at eip = 0x%08x\n", cpu.eip);
	nemu_state = STOP;
}

/* Simulate how the CPU works. */
void cpu_exec(volatile uint32_t n) {
	if(nemu_state == END) {
		printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
		return;
	}
	nemu_state = RUNNING;

#ifdef DEBUG
	volatile uint32_t n_temp = n;
#endif

	setjmp(jbuf);

	for(; n > 0; n --) {
#ifdef DEBUG
		swaddr_t eip_temp = cpu.eip;
		if((n & 0xffff) == 0) {
			/* Output some dots while executing the program. */
			fputc('.', stderr);
		}
#endif

		/* Execute one instruction, including instruction fetch,
		 * instruction decode, and the actual execution. */
		int instr_len = exec(cpu.eip);

		cpu.eip += instr_len;

#ifdef DEBUG
		print_bin_instr(eip_temp, instr_len);
		strcat(asm_buf, assembly);
		Log_write("%s\n", asm_buf);
		if(n_temp < MAX_INSTR_TO_PRINT) {
			printf("%s\n", asm_buf);
		}
#endif

		/* TODO: check watchpoints here. */
		if(check_watchpoints()) nemu_state = STOP;

#ifdef HAS_DEVICE
		extern void device_update();
		device_update();
#endif

		if(nemu_state != RUNNING) { return; }
		//device interrupt
		if(cpu.INTR & cpu.IF) {
			uint32_t i8259_query_intr();
			void i8259_ack_intr();
			void raise_intr(uint8_t NO);
			uint32_t intr_no = i8259_query_intr();
			i8259_ack_intr();
			raise_intr(intr_no);
		}
	}

	if(nemu_state == RUNNING) { nemu_state = STOP; }
}

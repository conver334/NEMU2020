#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "memory.h"
#include <elf.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	if(args == NULL){
		cpu_exec(1);
	}
	else {
		int num, ok;
		ok = sscanf(args, "%d", &num);
		if(ok == 1) cpu_exec(num);
		else printf("parameter fault\n");
	}
	return 0;
}

static int cmd_info(char *args) {
	if(args == NULL || strlen(args) != 1){
		printf("parameter fault\n");
	}
	else {
		if(args[0] == 'r'){
			int i;
			for(i = R_EAX; i <= R_EDI; i++){
				printf("%s   %x\n", regsl[i], reg_l(i));
			}
		}
		else if(args[0] == 'w'){
			print_wp();
		}
		else {
			printf("parameter fault\n");
		}
	}
	return 0;
}

static int cmd_x(char *args) {
	current_sreg = R_DS;
	int num, addr;
	char saddr[32];
	int ok = sscanf(args, "%d %s", &num, saddr);
	if(ok != 2){
		printf("parameter fault\n");
	}
	else {
		bool success = true;
		addr = expr(saddr, &success);
		int i, j;
		for(i = addr; i < addr + num * 4; i += 4){
			printf("%08x\t", i);
			for(j = 0; j <= 3; j++){
				printf("%02x%c", swaddr_read(i + j, 1), j == 3 ? '\n' : ' ');
			}
		}
	}
	return 0;
}

static int cmd_p(char *args) {
	if(args == NULL) {
		printf("No expression entered\n");
	}
	else {
		int ans; bool ok = true;
		ans = expr(args, &ok);
		if(ok) printf("%u\n", ans);
	}
	return 0;
}

static int cmd_w(char *args) {
	if(args == NULL) {
		printf("No expression entered\n");
	}
	else {
		bool ok = true;
		int val = expr(args, &ok);
		if(ok){
			WP* wp = new_wp();
			strcpy(wp -> str, args);
			wp -> value = val;
			printf("wp_pool id:%d is used for watchpoint, expression = %s\n", wp -> NO, wp -> str);
		}
	}
	return 0;
}

static int cmd_d(char* args) {
	int num;
	int ok = sscanf(args, "%d", &num);
	if(ok != 1) {
		printf("input invalid\n");
	}
	else {
		delete_wp(num);
	}
	return 0;
}

void get_Function_From_Address(swaddr_t address, char *str);

static int cmd_bt(char *args) {
	current_sreg = R_SS;
	swaddr_t cur_ebp = reg_l(R_EBP), cur_ret = cpu.eip;
	int cnt = 0, i;
	char name[50];
	while(cur_ebp) {
		get_Function_From_Address(cur_ret, name);
		if(name[0] == '\0') break;
		printf("#%d 0x%x: %s (", ++cnt, cur_ret, name);
		for(i = 0; i <= 3; i++) {
			printf("%d%c", swaddr_read(cur_ebp + 8 + i * 4, 4), i == 3 ? ')' : ',');
		}
		cur_ret = swaddr_read(cur_ebp + 4, 4), cur_ebp = swaddr_read(cur_ebp, 4);
		printf("\n");
	}
	return 0;
}
hwaddr_t page_translate_without_assert(lnaddr_t addr, int* success);
static int cmd_page(char *args){
	if(args == NULL) {
		printf("Nothing entered\n");
	}
	else {
		uint32_t addr;
		sscanf(args, "%x", &addr);
		int ok = 0;
		uint32_t ans = page_translate_without_assert(addr, &ok);
		if(ok == 0){
			printf("0x%08x\n", ans);
		}
		else if(ok == 1){
			printf("first level page fault\n");
		}
		else if(ok == 2){
			printf("second level page fault\n");
		}
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{"si", "one step execute", cmd_si},
	{"info", "get information", cmd_info},
	{"x", "memory scan", cmd_x},
	{"p", "calculate expression", cmd_p},
	{"w", "use watchpoint", cmd_w},
	{"d", "delete watchpoint", cmd_d},
	{"bt", "print stackframe", cmd_bt},
	{"page", "print result of page translation", cmd_page},

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}

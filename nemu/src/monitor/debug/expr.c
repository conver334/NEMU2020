#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "elf.h"

enum {
	NOTYPE = 256, HEX, DEC, REG, MARK, 
	LP, RP, 
	NEG, DEREF, NOT, 
	PROD, DIV, 
	PLUS, SUB, 
	EQ, NEQ, 
	AND, 
	OR

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{" +",	NOTYPE},				// spaces
	{"0[xX][0-9a-fA-F]+", HEX}, // hex
	{"[0-9]+", DEC},            // dec
	{"\\$[a-zA-Z]+", REG},          // register  
	{"[a-zA-Z][A-Za-z0-9_]*", MARK},          // mark
	
	{"\\(", LP},						// left parenthese
	{"\\)", RP},						// right parenthese

	{"\\*", PROD},						// prod
	{"/", DIV},						// div

	{"\\+", PLUS},					// plus
	{"\\-", SUB},						// sub

	{"==", EQ},						// equal
	{"!=", NEQ},						// not equal

	{"\\!", NOT},						// not

	{"&&", AND},						//and

	{"\\|\\|", OR}						//or
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;
int priority[32];

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				if(rules[i].token_type == NOTYPE) break;
				tokens[nr_token].type = rules[i].token_type;
				strncpy(tokens[nr_token].str, substr_start, substr_len);
				tokens[nr_token].str[substr_len] = '\0';
				int last = nr_token - 1;
				switch (tokens[nr_token].type) {
					case SUB:
						if(nr_token == 0 || (tokens[last].type != RP && tokens[last].type != DEC && tokens[last].type != REG && tokens[last].type != HEX)){
							tokens[nr_token].type = NEG;						
						}
						break;
					case PROD:
						if(nr_token == 0 || (tokens[last].type != RP && tokens[last].type != DEC && tokens[last].type != REG && tokens[last].type != HEX)){
							tokens[nr_token].type = DEREF;						
						}
						break;
					default:
						break;
				}
				++nr_token;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

struct type_to_priority{
	int type, priority;
} type_to_priority_table[] = {
	{DEC, 0}, {HEX, 0}, {REG, 0},
	{LP, 1}, {RP, 1},
	{NEG, 2}, {DEREF, 2}, {NOT, 2},
	{PROD, 3}, {DIV, 3},
	{PLUS, 4}, {SUB, 4},
	{EQ, 5}, {NEQ, 5},
	{AND, 6},
	{OR, 7}
};

#define NR_TYPE_TO_PRIORITY (sizeof(type_to_priority_table) / sizeof(type_to_priority_table[0]) )

void get_tokens_priority(){
	int i, j;
	for(i = 0; i < nr_token; i++) {
		for(j = 0; j < NR_TYPE_TO_PRIORITY; j++) {
			if(type_to_priority_table[j].type == tokens[i].type) {
				priority[i] = type_to_priority_table[j].priority;
				break;
			}
		}
	}
	return ;
}

int get_register_value(char *e, bool *ok){
	int i, len = strlen(e);
	for(i = 1; i < len; i++) {
		if(e[i] >= 'A' && e[i] <= 'Z') e[i] = (e[i] - 'A') + 'a';
	}
	for(i = R_EAX; i <= R_EDI; i++) {
		if(strcmp(e + 1, regsl[i]) == 0) {
			*ok = true;
			return reg_l(i);
		}
	}
	for(i = R_AX; i <= R_DI; i++) {
		if(strcmp(e + 1, regsw[i]) == 0) {
			*ok = true;
			return reg_w(i);
		}
	}
	for(i = R_AL; i <= R_BH; i++) {
		if(strcmp(e + 1, regsb[i]) == 0) {
			*ok = true;
			return reg_b(i);
		}
	}
	if(strcmp(e + 1, "eip") == 0) {
		*ok = true;
		return cpu.eip;
	}
	*ok = false;
	return 0;	
}

int dereference(int num){
	current_sreg = R_DS;
	return swaddr_read(num, 4);
}

bool check_parentheses(int l, int r){
	if(tokens[l].type != LP || tokens[r].type != RP) return false;
	bool ok = true;
	int cur, cnt = 0;
	for(cur = l + 1; cur <= r - 1; cur++) {
		if(tokens[cur].type == LP) ++cnt;
		else if(tokens[cur].type == RP) --cnt;
		if(cnt < 0) {
			ok = false; break;
		}
	}
	if(cnt != 0) ok = false;
	return ok;
}

int get_dominant_operator_id(int l, int r){
	int ans = -1, cur_max_priority = -1, cur_left_parenthese = 0, i;
	for(i = l; i <= r; i++){
		if(priority[i] == 0) continue;
		if(tokens[i].type == LP) cur_left_parenthese++;
		else if(tokens[i].type == RP) cur_left_parenthese--;
		if(cur_left_parenthese > 0) continue;
		if(cur_max_priority <= priority[i]){
			cur_max_priority = priority[i];
			ans = i;
		}
	}
	if(ans != -1 && priority[ans] == 2){
		cur_max_priority = -1; cur_left_parenthese = 0;
		for(i = l; i <= r; i++){
			if(priority[i] == 0) continue;
			if(tokens[i].type == LP) cur_left_parenthese++;
			else if(tokens[i].type == RP) cur_left_parenthese--;
			if(cur_left_parenthese > 0) continue;
			if(cur_max_priority < priority[i]){
				cur_max_priority = priority[i];
				ans = i;
			}
		}
	}
	return ans;
}

uint32_t get_Address_From_Mark(char *mark, bool *success);

int eval(int l, int r, bool *success){
	if(l > r){
		printf("bad expression\n");
		*success = false;
		return 0;
	}
	else if(l == r){
		int ans; bool ok;
		switch (tokens[l].type) {
		case DEC:
			sscanf(tokens[l].str, "%d", &ans);
			return ans;
		case HEX:
			sscanf(tokens[l].str, "%x", &ans);
			return ans;
		case REG:
			ok = false;
			ans = get_register_value(tokens[l].str, &ok);
			if(ok) return ans;
			else{
				printf("register %s not exists\n", tokens[l].str);
				*success = false;
				return 0;
			}
		case MARK:
			return get_Address_From_Mark(tokens[l].str, success);
		default:
			printf("bad expression\n");
			*success = false;
			return 0;
		}
	}
	else if(check_parentheses(l, r) == true){
		return eval(l + 1, r - 1, success);
	}
	else {
		int dominant_operator_id = get_dominant_operator_id(l, r);
		if(dominant_operator_id == -1){
			printf("bad expression\n");
			*success = false;
			return 0;
		}
		else if(priority[dominant_operator_id] == 2){
			if(dominant_operator_id != l) {
				printf("bad expression\n");
				*success = false;
				return 0;
			}
			else {
				int num = eval(dominant_operator_id + 1, r, success);
				if(!*success) return 0;
				switch (tokens[dominant_operator_id].type) {
					case NEG:
						num = -num;
						break;
					case NOT:
						num = !num;
						break;
					case DEREF:
						num = dereference(num);
						break;
					default:
						break;
				}
				return num;
			}
		}
		else {
			int num1 = eval(l, dominant_operator_id - 1, success);
			if(!*success) return 0;
			int num2 = eval(dominant_operator_id + 1, r, success);
			if(!*success) return 0;
			switch (tokens[dominant_operator_id].type) {
				case PROD:
					return num1 * num2;
				case DIV:
					if(num2 == 0) {
						printf("divided by zero\n"); 
						*success = false;
						return 0;
					}
					return num1 / num2;
				case PLUS:
					return num1 + num2;
				case SUB:
					return num1 - num2;
				case EQ:
					return num1 == num2 ? 1 : 0;
				case NEQ:
					return num1 != num2 ? 1 : 0;
				case AND:
					return (int) (num1 && num2);
				case OR:
					return (int) (num1 || num2);
				default:
					printf("unknown error\n");
					*success = false;
					return 0;
			}
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		printf("expression not recognized by the regex\n");
		return 0;
	}
	get_tokens_priority();
	/* TODO: Insert codes to evaluate the expression. */
	// panic("please implement me");
	return eval(0, nr_token - 1, success);
}


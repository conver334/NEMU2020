#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#define typ(index) tokens[index].type
#define tstr(index) tokens[index].str
enum {	
	NOTYPE = 256, REG=257,DEC=258,HEX=259,
	LK='(',RK=')',
	DEREF=260,NEG=261,NOT='!',
	MU='*',EXCE='/',MI='-',AD='+',
	EQ='=',NEQ=271,
	AND='&',OR='|'
	/* TODO: Add more token types */
};
int ope_rank[300];
int quan[300];
void init(){
	ope_rank[DEC]=0,ope_rank[HEX]=0,ope_rank[REG]=0;
	ope_rank[LK]=2,ope_rank[RK]=2;
	ope_rank[DEREF]=3,ope_rank[NEG]=3,ope_rank[NOT]=3,
	ope_rank['*']=4,ope_rank['/']=4;
	ope_rank['+']=5,ope_rank['-']=5;
	ope_rank['=']=6, ope_rank[NEQ]=6;
	ope_rank['&']=7,ope_rank['|']=7;
	int i='0';
	for(; i < 58; i++){
		quan[i]=i-48;
	}
	for( i = 'a'; i <= 'z'; i++){
		quan[i]=i - 'a' + 10;
	}
	for( i = 'A'; i <= 'Z'; i++){
		quan[i]=i - 'A' + 10;
	}
}
static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", AD},					// plus
	{"==", EQ},						// equal
	{"!=",NEQ},			
	{"0[Xx][0-9A-Fa-f]+",HEX},
	{"[0-9]+",DEC},
	{"\\(",LK},
	{"\\)",RK},
	{"\\*",MU},
	{"-",MI},
	{"/",EXCE},
	{"\\$[a-zA-Z]+",REG},
	{"\\!", NOT},
	{"&&", AND},
	{"\\|\\|", OR}
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

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 * NOTYPE = 256, EQ=259,NUM=257, ALP=258,LK='(',RK=')',MU='*',MI='-',AD='+',EXCE='/'
				 */
				// printf("qwq %d\n",rules[i].token_type);
				switch(rules[i].token_type) {
					case NOTYPE:break;
					case EQ:break;
					default: tokens[++nr_token].type=rules[i].token_type;
							  if(substr_len<=32){
								  strncpy(tokens[nr_token].str,substr_start,substr_len);  
								//   tokens[nr_token].str[substr_len]='\0';
								//   printf("%d   %s\n",nr_token,tstr(nr_token));
							  }					  
				}
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
bool global_success=true;
int getdominant(int p, int q){
	int nowmin=-1,nowp=-1, i;
	for( i = p; i <= q; i++){
		if(ope_rank[typ(i)] >= nowmin){
			nowmin = ope_rank[typ(i)];
			nowp = i;
		}
	}
	return nowp;
}
bool exp_legi(int p, int q){
	int i,tail=0;
	for( i = p; i <= q; i++){
		if(typ(i) == '(')tail++;
		else if(typ(i) == ')'){
			if(tail==0)return 0;
			else tail--;
		}
	}
	if(tail == 0)return 1;
	return 0;
}
bool check_parentheses(int p, int q){
	if(!exp_legi(p,q)){
		global_success = false;//illegal expression
	}
	else {
		if(typ(p)!='('||typ(q)!=')')return 0;
		if(exp_legi(p+1,q-1))return 1;
	}
	return 0;
}
// const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
// const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
// const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
// #define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)  
// #define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
// #define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

int eval(int p,int q){
	if(p>q){
		global_success=false;
	}
	else if(p==q){
		int num=0, len=strlen(tstr(q)), i;
		if(len>=3&&tstr(q)[0]=='$'){
			for( i=0;i<8;i++){
				if(strcmp(regsl[i],tstr(q)+1)==0){
		
					return reg_l(i);
				}
			}
			for( i=0;i<8;i++){
				if(strcmp(regsw[i],tstr(q)+1)==0){
					return reg_w(i);
				}
			}
			for( i=0;i<8;i++){
				if(strcmp(regsb[i],tstr(q)+1)==0){
					return reg_b(i);
				}
			}
			if(strcmp("eip",tstr(q)+1)==0){
				return cpu.eip;
			}
		}
		switch (typ(q))
		{
		case HEX: sscanf(tstr(q),"%x",&num);
			break;
		case DEC: sscanf(tstr(q),"%d",&num);
			break;
		default:
			printf("bad expression\n");
			global_success=false;
			break;
		}
		// if(len>1&&(tstr(q)[1]=='x'||tstr(q)[1]=='X')){
		// 	for( i=2;i<len;i++){
		// 		num=num*16+quan[(int)tstr(q)[i]];
		// 	}
		// }
		// for( i=0;i<len;i++){
		// 	num=num*10+quan[(int)tstr(q)[i]];
		// }
		// printf("hahah2  %d\n",num);
		return num;
	}
	else if(check_parentheses(p,q) == true){
		return eval(p+1,q-1);
	}
	else {
		int op,val1,val2;
		op=getdominant(p,q);
		val2=eval(op+1,q);
		if(typ(op)==NEG)return -1*val2;
		if(typ(op)==DEREF){
			return swaddr_read(val2,4);
		}
		if(typ(op)==NOT)return (!val2);
		val1=eval(p,op-1);
		switch(typ(op)){
			case '+':return val1+val2;
			case '-':return val1-val2;
			case '*':return val1*val2;
			case '/':return (val1/val2);
			case AND:return (val1&&val2);
			case OR:return (val1||val2);
			case EQ:return (val1==val2);
			case NEQ:return (val1!=val2);
			default:global_success=false; // dont know assert
		}
	}
	return 0;
}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	init();
	int i;
	for(i = 0; i < nr_token; i++){
		if(tokens[i].type == '*' && (i == 0 ||ope_rank[tokens[i - 1].type]>0)){
			tokens[i].type = DEREF;
		}
		else if(tokens[i].type == '-' && (i == 0 ||ope_rank[tokens[i - 1].type]>0)){
			tokens[i].type = NEG;
		}
	}
	int ans=eval(1,nr_token);
	if(global_success){
		return ans;
		//printf("The ans of expr is %d\n",);
	}
	else {
		*success = false;
	}	
	return 0;
}
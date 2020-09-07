#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#define typ(index) tokens[index].type
enum {
	NOTYPE = 256, EQ=259,NUM=257, ALP=258,LK='(',RK=')',MU='*',MI='-',AD='+',EXCE='/'

	/* TODO: Add more token types */

};
int ope_rank[300];
void init(){
	ope_rank['*']=3,ope_rank['/']=3;
	ope_rank['+']=2,ope_rank['-']=2;
	ope_rank[257]=300,ope_rank[258]=300,ope_rank[256]=300;ope_rank[259]=300;
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
	{"[0-9]+",NUM},
	{"\\(",LK},
	{"\\)",RK},
	{"\\*",MU},
	{"-",MI},
	{"[A-Za-z]",ALP},
	{"/",EXCE}
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

				switch(rules[i].token_type) {
					case NUM: tokens[++nr_token].type=rules[i].token_type;
							  if(substr_len<=32){
								  strncpy(tokens[nr_token].str,substr_start,substr_len);  
								  tokens[nr_token].str[substr_len]='\0';
							  }
							  break;
					case ALP: tokens[++nr_token].type=rules[i].token_type;
							if(substr_len<=32){
								  strncpy(tokens[nr_token].str,substr_start,substr_len);	
								  tokens[nr_token].str[substr_len]='\0';		  
							}	
							break;
					case NOTYPE:break;
					case EQ:break;
					default: tokens[++nr_token].type=rules[i].token_type;
					tokens[nr_token].str[substr_len]='\0';
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
	int nowmin=1000,nowp=-1, i;
	for( i = p; i <= q; i++){
		if(typ(i) == '('){
			while(typ(i) != ')') i++;
			continue;
		}
		if(ope_rank[typ(i)] <= nowmin){
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
int eval(int p,int q){
	if(p>q){
		global_success=false;
	}
	else if(p==q){
		int num=0, len=strlen(tokens[q].str), i;
		for( i=0;i<len;i++){
			num=num*10+tokens[q].str[i]-'0';
		}
		// printf("hahah2  %d\n",num);
		return num;
	}
	else if(check_parentheses(p,q) == true){
		return eval(p+1,q-1);
	}
	else {
		int op,val1,val2;
		op=getdominant(p,q);
		val1=eval(p,op-1);
		val2=eval(op+1,q);
		switch(typ(op)){
			case '+':return val1+val2;
			case '-':return val1-val2;
			case '*':return val1*val2;
			case '/':return (val1/val2);
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
	int ans=eval(1,nr_token);
	if(global_success){
		printf("The ans of expr is %d\n",ans);
	}
	else {
		*success = false;
	}	
	return 0;
}
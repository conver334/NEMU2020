#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "trap.h"

typedef int FLOAT;
#define ll long long 
#define getflag(x) (x>>31)&1
#define getsign(x) (x?-1:1)
#define l16 (1<<16)

static inline int F2int(FLOAT a) {
	int flag=getflag(a);
	int tmp=a*getsign(flag);
	return (tmp>>16)*getsign(flag);
}

static inline FLOAT int2F(int a) {
	int flag=getflag(a);
	int tmp=a*getsign(flag);

	return (tmp<<16)*getsign(flag);
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
	return (a*b);
}

static inline FLOAT F_div_int(FLOAT a, int b) {
	return (a/b);
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT sqrt(FLOAT);
FLOAT pow(FLOAT, FLOAT);

// used when calling printf/sprintf to format a FLOAT argument
#define FLOAT_ARG(f) (long long)f

void init_FLOAT_vfprintf(void);

#endif

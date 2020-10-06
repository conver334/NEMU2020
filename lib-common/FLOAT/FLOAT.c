#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	return (FLOAT)((ll)a*(ll)b>>16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */

	int flag1=getflag(a),flag2=getflag(b),flag3;
	flag3 = getsign(flag1)*getsign(flag2);
	int tmp1=a*getsign(flag1),tmp2=b*getsign(flag2);
	int ans=tmp1/tmp2,mod=tmp1%tmp2;
	int i;
	for(i=0;i<16;i++){
		ans<<=1;
		mod<<=1;
		if(mod>=tmp2){
			mod-=tmp2;
			ans|=1;
		}
	}
	return ans*flag3;
}

FLOAT f2F(float a) {
	/* You should figure out how to convert `a' into FLOAT without
	 * introducing x87 floating point instructions. Else you can
	 * not run this code in NEMU before implementing x87 floating
	 * point instructions, which is contrary to our expectation.
	 *
	 * Hint: The bit representation of `a' is already on the
	 * stack. How do you retrieve it to another variable without
	 * performing arithmetic operations on it directly?
	 */

	int intf,flag,E,e,m;
	intf = *((int*)&a);//get the int form of a
	flag = getflag(intf);
	E = (intf>>23)&0xff;
	//m contains 23 bits
	m = (intf&0x7fffff);
	FLOAT res = m;
	e =E-0x7f;//normalization of floating point numbers
	if(!E){
		if(!m)return 0;
		else e=1-E;
	}
	else if(E==0xff){
		if(flag)return -1;
		else return (-1)^(1<<31);                                                 
	}
	else res|=(1<<23);
	if(e>7)res<<=e-7;
	else res>>=-e+7;
	return (res*getsign(flag));
}

FLOAT Fabs(FLOAT a) {
	return a*getsign(getflag(a));
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}


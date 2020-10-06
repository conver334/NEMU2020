#include "trap.h"
int n,L,f[1010];
struct node{
	int s,t;
}a[1010];
int max(int x,int y){
if(x>y)return x;
else return y;
}
int main(){
	n=16,L=10;
	int i,j;
	for( i=1;i<=n;++i)a[i].s=1, a[i].t=1;
	int ans = 0,w;

	for(i = 1; i <= n; i++){
		w=a[i].s+a[i].t-1;
        for(j = L; j >=w; j--)
        {
            f[j]=max(f[j-a[i].t]+1,f[j]);
        }
    }
	for( i=0;i<=L;i++)ans=max(f[i],ans);
	nemu_assert(ans==10);
	return 0;
}
/*
10
16 10
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
1 1
*/

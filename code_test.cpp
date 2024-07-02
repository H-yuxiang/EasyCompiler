#include<bits/stdc++.h>
using namespace std;
#define ll long long
const int maxn = (int)2e5+5;
const int modnum = (int)1e9+7;
const int _INT_MAX = 0x7fffffff;

int a[2][2];
int program(int a,int b,int c)
{
	int i;
	int j;
	i=2;
	if(a>(b+c))
	{
		j=a+(b*c+1);
	}
	else
	{
		j=a;
	}	
	while(i<=100)
	{
		i=i*2;
	}
	return i+j;
}
int demo(int a)
{
	a=a+2;
	return a*2;
}
int main() {
    a[0][0]=3;
	a[0][1]=a[0][0]+1;
	a[1][0]=a[0][1]+7;
	a[1][1]=program(a[0][0],a[0][1],demo(a[1][0]));
    cout<<a[1][1]<<endl;
    return 0;
}



//https://www.acwing.com/problem/content/3/
//2025307070614 陈鹏屹
//2025307070630 沈旭燃
//2025307070643 赵哲
#include <stdio.h>
#define max(a,b) ((a)>(b) ? (a):(b))
int main(){
    int v[1008],w[1008];
    int f[1008][1008]={0};
    int N,V;
    scanf("%d%d",&N,&V);
    for(int i=1;i<=N;i++){
        scanf("%d%d",&v[i],&w[i]);
    }
    for(int i=1;i<=N;i++){
        for(int j=1;j<=V;j++){
            if(j<v[i]) f[i][j]=f[i-1][j];
            else{
                f[i][j]=max(f[i-1][j],f[i][j-v[i]]+w[i]);
            }
        }
    }
    printf("%d",f[N][V]);
    return 0;
}

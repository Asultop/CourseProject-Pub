//https://www.acwing.com/problem/content/2/
//2025307070614 陈鹏屹
//2025307070630 沈旭燃
//2025307070643 赵哲
#include <stdio.h>
int max(int a,int b){
    return a>b?a:b;
}
int main(){
    int v[1008],w[1008];//体积,价值
	int f[1008][1008]={0};//f[i][j], j体积下前i个物品的最大价值 
    int n,m;
    scanf("%d%d",&n,&m);
    for(int i=1;i<=n;i++){
        scanf("%d%d",&v[i],&w[i]);
    }
    for(int i=1;i<=n;i++){
        for(int j=1;j<=m;j++){
            if(j<v[i]) f[i][j]=f[i-1][j];//  当前背包容量装不进第i个物品，则价值等于前i-1个物品
            else{
                f[i][j]=max(f[i-1][j],f[i-1][j-v[i]]+w[i]);// 能装，需进行决策是否选择第i个物品
            }
        }
    }
    printf("%d",f[n][m]);
    return 0;
}

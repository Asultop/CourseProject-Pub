#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int rcount = 5;
int main(){
    printf("√> 找到 %d 条题目：\n", rcount);
    printf("|      -------- 题目列表 (%d) ---------       |\n", rcount);
    printf("|ID\t\t标题\t\t难度\t|\n");
    char anoymous[]="中";
    printf("strlen = %ld\n",strlen(anoymous));

    const char* RED = "\x1b[31m";
    printf("strlen(RED) = %ld\n",strlen(RED));
}
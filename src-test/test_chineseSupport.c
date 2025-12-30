#include "../src-extends/chineseSupport.h"
#include "stdio.h"
#include "string.h"
int main(){
    char testStr1[]="中文测试";
    printf("测试字符串: %s\n", testStr1);
    printf("长度：%ld\n", get_real_Length(testStr1,NULL));
}
#include "../src-extends/chineseSupport.h"
#include "stdio.h"
#include "string.h"
int main(){
    char testStr1[]="中文测试";
    printf("测试字符串: %s\n", testStr1);
    printf("长度：%ld\n", get_real_Length(testStr1,NULL));
    const char* RED = "\x1b[31m";
    printf("长度(含颜色码): %ld\n", get_real_Length(RED,NULL));
    printf("|%s|\n",getSpaceContent("测试",10,MARGIN_CENTER));
    printf("|%s|\n",getSpaceContent("测试",10,MARGIN_LEFT));
    printf("|%s|\n",getSpaceContent("测试",10,MARGIN_RIGHT));
    printf("|%s|\n",getSpaceContent("Test",10,MARGIN_CENTER));
    printf("|%s|\n",getSpaceContent("Test",10,MARGIN_LEFT));
    printf("|%s|\n",getSpaceContent("Test",10,MARGIN_RIGHT));
    printf("|%s|\n",getSpaceContent("测试Test",10,MARGIN_CENTER));
    printf("|%s|\n",getSpaceContent("测试Test",10,MARGIN_LEFT));
    printf("|%s|\n",getSpaceContent("测试Test",10,MARGIN_RIGHT));


}
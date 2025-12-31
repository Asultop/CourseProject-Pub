#include "../src-extends/releaseRuntime.h"
#include <stdio.h>
int main(){
    const char* outputDir = "./OutputDir__Released";
    int result = releaseRuntimeResources(outputDir);
    if (result == 0) {
        printf("√> 资源释放成功，文件已输出到目录：%s\n", outputDir);
    } else {
        printf("x> 资源释放失败，错误代码：%d\n", result);
    }
    return 0;
}
#include "../src-extends/codeRender.h"
#include <string.h>
#include <stdio.h>
int main(int argc, char* argv[]){
    if(argc == 1){
        const char* testfile = "src-test/test_codeRender.c";
        codeRender_worker(testfile);
        puts("");
        return 0;
    }
    else {
        for(int i=1;i<argc;i++){
            printf("====== 渲染文件：%s ======\n", argv[i]);
            codeRender_worker(argv[i]);
            puts("\n====== 文件结束 ======\n");
        }
        return 0;
    }
    
}
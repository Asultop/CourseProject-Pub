#include "../src-extends/codeRender.h"
#include <string.h>
#include <stdio.h>
int main(){
    const char* testfile = "src-test/test_codeRender.c";
    codeRender_worker(testfile);
    puts("");
    return 0;
}
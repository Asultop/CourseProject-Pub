#define SCREEN_MARGIN_LEFT 1
#include "../src-extends/screenManager.h"
#include <stdio.h>
#include <math.h>
#include "../src-extends/Def.h"
#include "../src-extends/chineseSupport.h"

char * rainbowStringGenerator(size_t length){ // 带 ANSI 真彩色
    if(length == 0) return NULL;
    const char* baseFormat = "\x1b[38;2;%d;%d;%dm%c";
    size_t unitLen = strlen(baseFormat) + 12; // 12 是 RGB 数字和字符的最大长度估计
    char * result = (char*)malloc(unitLen * length + 5); // 多分配一些空间以防万一
    if(!result) return NULL;
    result[0] = '\0';
    for(size_t i = 0; i < length; i++){
        int r = (int)(127 + 128 * sin(0.3 * i + 0));
        int g = (int)(127 + 128 * sin(0.3 * i + 2));
        int b = (int)(127 + 128 * sin(0.3 * i + 4));
        char ch = 'A' + (char)(i % 26);
        char buf[64];
        snprintf(buf, sizeof(buf), baseFormat, r, g, b, ch);
        strcat(result, buf);
    }
    strcat(result, "\x1b[0m"); // 重置颜色
    return result;
}
char * rainbowizeString(const char * input){
    if(!input) return NULL;
    size_t len = strlen(input);
    char * result = (char*)malloc(len * 32 + 5); // 预留足够空间
    if(!result) return NULL;
    result[0] = '\0';
    char ** tokens = processRawChar(input);
    if(!tokens) {
        free(result);
        return NULL;
    }
    for(size_t i = 0; tokens[i] != NULL && strcmp(tokens[i], "EOL") != 0; i++){
        int r = (int)(127 + 128 * sin(0.3 * i + 0));
        int g = (int)(127 + 128 * sin(0.3 * i + 2));
        int b = (int)(127 + 128 * sin(0.3 * i + 4));
        char buf[128];
        snprintf(buf, sizeof(buf), "\x1b[38;2;%d;%d;%dm%s\x1b[0m", r, g, b, tokens[i]);
        strcat(result, buf);
    }
    freeProcessedChars(tokens);
    return result;
}
int main(){

    printHeader();
    printContent("This is a test content line.");
    printContent("中文测试内容行");
    printFooter();
    puts("");
    printHeader();
    printLeft("Left Aligned Line Test");
    printLeft("左对齐测试行");
    printLeft("混合对齐 Mix Alignment");
    printDivider();
    printRight("Right Aligned Line Test");
    printRight("右对齐测试行");
    printRight("混合对齐 Mix Alignment");
    printDivider();
    printCenter("Centered Line Test");
    printCenter("居中行测试");
    printCenter("混合居中 Mix Centeralization");
    printFooter();
    puts("");
    printHeader();
    printLeft(rainbowizeString("Left Colorful Line Test"));
    printLeft(rainbowizeString("左对齐彩色行测试"));
    printLeft(rainbowizeString("混合对齐 Mix Alignment"));
    printDivider();
    printRight(rainbowizeString("Right Colorful Line Test"));
    printRight(rainbowizeString("右对齐彩色行测试"));
    printRight(rainbowizeString("混合对齐 Mix Alignment"));
    printDivider();
    printCenter(rainbowizeString("Centered Colorful Line Test"));
    printCenter(rainbowizeString("居中彩色行测试"));
    printCenter(rainbowizeString("混合居中 Mix Centeralization"));
    printFooter();
    return 0;
}
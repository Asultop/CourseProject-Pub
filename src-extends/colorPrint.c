#include "colorPrint.h"
#include "Def.h"
#include <time.h>

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
// char * rainbowizeString(const char * input){
//     if(!input) return NULL;
//     size_t len = strlen(input);
//     char * result = (char*)malloc(len * 32 + 5); // 预留足够空间
//     if(!result) return NULL;
//     result[0] = '\0';
//     char ** tokens = processRawChar(input);
//     if(!tokens) {
//         free(result);
//         return NULL;
//     }
//     for(size_t i = 0; tokens[i] != NULL && strcmp(tokens[i], "EOL") != 0; i++){
//         int r = (int)(127 + 128 * sin(0.3 * i + 0));
//         int g = (int)(127 + 128 * sin(0.3 * i + 2));
//         int b = (int)(127 + 128 * sin(0.3 * i + 4));
//         char buf[128];
//         snprintf(buf, sizeof(buf), "\x1b[38;2;%d;%d;%dm%s\x1b[0m", r, g, b, tokens[i]);
//         strcat(result, buf);
//     }
//     freeProcessedChars(tokens);
//     return result;
// }
char * rainbowizeString(const char * input){

    if(!RTXON)
        return input ? strdup(input) : NULL;

    if(!input) return NULL;
    size_t len = strlen(input);
    size_t alloc = len * 16 + 5;
    char * result = (char*)malloc(alloc);
    if(!result) return NULL;
    result[0] = '\0';
    char ** tokens = processRawChar(input);
    if(!tokens) {
        free(result);
        return NULL;
    }
    size_t pos = 0;
    #ifdef RANDOM_RTX_OFFSET
    srand((unsigned int)time(NULL));
    int random_offset = rand() % 100;
    #else
    int random_offset = 0;
    #endif

    return justRainbowizeString(input, random_offset);
}
char * justRainbowizeString(const char * input,int seed){

    if(!input) return NULL;
    size_t len = strlen(input);
    size_t alloc = len * 16 + 5;
    char * result = (char*)malloc(alloc);
    if(!result) return NULL;
    result[0] = '\0';
    char ** tokens = processRawChar(input);
    if(!tokens) {
        free(result);
        return NULL;
    }
    size_t pos = 0;

    for(size_t i = 0; tokens[i] != NULL && strcmp(tokens[i], "EOL") != 0; i++){
        int r = (int)(127 + 128 * sin(0.1 * (i + seed) + 0));
        int g = (int)(127 + 128 * sin(0.1 * (i + seed) + 2));
        int b = (int)(127 + 128 * sin(0.1 * (i + seed) + 4));
        /* 安全追加：确保不越界 */
        if (pos + 32 >= alloc) {
            /* 扩容 */
            size_t new_alloc = alloc * 2;
            char *tmp = (char*)realloc(result, new_alloc);
            if (!tmp) break;
            result = tmp; alloc = new_alloc;
        }
        int n = snprintf(result + pos, (alloc - pos), "\x1b[38;2;%d;%d;%dm%s\x1b[0m", r, g, b, tokens[i]);
        if (n < 0) break;
        pos += (size_t)n;
        if (pos >= alloc) pos = alloc - 1;
    }
    freeProcessedChars(tokens);
    return result;
}
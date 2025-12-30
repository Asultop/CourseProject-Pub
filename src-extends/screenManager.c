#include "screenManager.h"
#include <stdio.h>
#include <stdlib.h>

extern void cleanBuffer(){
    char c;
    while((c = getchar()) != '\n' && c != EOF);
}
extern void cleanScreen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
extern void pauseScreen(void) {
    printf("⎵> 按 Enter 键继续...\n");
    fflush(stdout);
    cleanBuffer();
    getchar();
}
void printSplashScreen(){
    puts("╔════════════════════════════════════════╗");
    puts("║        欢迎使用 —— 在线评测系统        ║");
    puts("╠════════════════════════════════════════╣");
    puts("║             请选择操作                 ║");
    puts("╠────────────────────────────────────────╣");
    puts("║ 1. 登录        2. 注册                 ║");
    puts("║ 3. 修改密码    4. 删除用户             ║");
    puts("║                                        ║");
    puts("╠────────────────────────────────────────╣");
    puts("║ 0. 退出                                ║");
    puts("╚════════════════════════════════════════╝");
    printf("⇒ 请输入选项：");
}
void printMainScreen(const char * username){
    cleanScreen();
    printf("╔════════════════════════════════════════╗\n");
    printf("║  ACM 竞赛管理与训练系统                ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║ 用户：%s\n", username);
    puts(  "╠────────────────────────────────────────╣");
    puts(  "║ 1. ACM 竞赛简介                        ║");
    puts(  "║ 2. ACM 题库                            ║");
    puts(  "║                                        ║");
    puts(  "╠────────────────────────────────────────╣");
    puts(  "║ 0. 返回                                ║");
    puts(  "╚════════════════════════════════════════╝");
    printf("⇒ 请输入选项：");
}
void printACMDetailScreen(){
    puts("╔════════════════════════════════════════╗");
    puts("║            ACM 竞赛简介                ║");
    puts("╠════════════════════════════════════════╣");
    puts("║ 1. 参赛规则                            ║");
    puts("║ 2. 评分标准                            ║");
    puts("║ 3. 赛事构成                            ║");
    puts("║ 4. 赛事介绍                            ║");
    puts("║ 5. 历届获奖                            ║");
    puts("╠────────────────────────────────────────╣");
    puts("║ 0. 返回主菜单                          ║");
    puts("╚════════════════════════════════════════╝");
    printf("⇒ 请输入选项：");
}
void printHeader(){
    printf("╔");
    for(int i = 0; i < SCREEN_CHAR_WIDTH -2; i++){
        printf("═");
    }
    printf("╗\n");
}
void printFooter(){
    printf("╚");
    for(int i = 0; i < SCREEN_CHAR_WIDTH -2; i++){
        printf("═");
    }
    printf("╝\n");
}
void printDivider(){
    printf("╠");
    for(int i = 0; i < SCREEN_CHAR_WIDTH -2; i++){
        printf("─");
    }
    printf("╣\n");
}
int count_utf8_chinese(const char *str) {
    int count = 0;
    int i = 0;
    if (str == NULL) return 0;

    while (str[i] != '\0') {
        // UTF-8中文字符首字节范围：0xE0~0xEF（1110xxxx）
        if ((unsigned char)str[i] >= 0xE0 && (unsigned char)str[i] <= 0xEF) {
            // 检查后续两个字节是否符合UTF-8规则（10xxxxxx）
            if ((unsigned char)str[i+1] >= 0x80 && (unsigned char)str[i+1] <= 0xBF &&
                (unsigned char)str[i+2] >= 0x80 && (unsigned char)str[i+2] <= 0xBF) {
                count++;
                i += 3;  // 跳过中文字符的3个字节
                continue;
            }
        }
        // 非中文字符：按UTF-8规则跳过对应字节
        if ((unsigned char)str[i] < 0x80) {
            i += 1;  // ASCII字符（单字节）
        } else if ((unsigned char)str[i] < 0xE0) {
            i += 2;  // 双字节UTF-8字符（非中文）
        } else {
            i += 3;  // 三字节UTF-8字符（非中文）
        }
    }
    return count;
}
int count_gbk_chinese(const char *str) {
    int count = 0;
    int i = 0;
    if (str == NULL) return 0;

    while (str[i] != '\0') {
        // GBK中文字符第一个字节 ≥ 0x80（128）
        if ((unsigned char)str[i] >= 0x80) {
            count++;
            i += 2;  // 跳过中文字符的第二个字节
        } else {
            i += 1;  // ASCII字符，仅跳过当前字节
        }
    }
    return count;
}
void printContent(const char* content){
    // 存在中文
    if(content == NULL){
        printf("║ %-*s ║\n", SCREEN_CHAR_WIDTH - 4, "");
        return;
    }
    // 检测终端字符集类型
    if(getenv("LANG") != NULL && (strstr(getenv("LANG"), "UTF-8") != NULL || strstr(getenv("LANG"), "utf8") != NULL)){
        // UTF-8 编码
        int count=count_utf8_chinese(content);
        // 计算实际宽度
        int actualWidth=SCREEN_CHAR_WIDTH - 4 + count;
        printf("║ %-*s ║\n", actualWidth, content);
        return;
    }
    int count=count_utf8_chinese(content);
    printf("中文数量 %d",count);
    // 计算实际宽度
    int actualWidth=SCREEN_CHAR_WIDTH - 4 + count;
    printf("║ %-*s ║\n", actualWidth, content);
}
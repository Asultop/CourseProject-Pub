#include "screenManager.h"
#include "chineseSupport.h"
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
    // puts("╔════════════════════════════════════════╗");
    // puts("║        欢迎使用 —— 在线评测系统        ║");
    // puts("╠════════════════════════════════════════╣");
    // puts("║             请选择操作                 ║");
    // puts("╠────────────────────────────────────────╣");
    // puts("║ 1. 登录        2. 注册                 ║");
    // puts("║ 3. 修改密码    4. 删除用户             ║");
    // puts("║                                        ║");
    // puts("╠────────────────────────────────────────╣");
    // puts("║ 0. 退出                                ║");
    // puts("╚════════════════════════════════════════╝");
    printHeader();
    printCenter("欢迎使用 —— 在线评测系统");
    printDivider();
    printCenter("请选择操作");;
    printDivider();
    printLeft("1. 登录        2. 注册");
    printLeft("3. 修改密码    4. 删除用户");
    printContent("");
    printDivider();
    printCenter("0. 退出");
    printFooter();
    printf("⇒ 请输入选项：");
}
void printMainScreen(const char * username){
    
    // printf("╔════════════════════════════════════════╗\n");
    // printf("║  ACM 竞赛管理与训练系统                ║\n");
    // printf("╠════════════════════════════════════════╣\n");
    // printf("║ 用户：%s\n", username);
    // puts(  "╠────────────────────────────────────────╣");
    // puts(  "║ 1. ACM 竞赛简介                        ║");
    // puts(  "║ 2. ACM 题库                            ║");
    // puts(  "║                                        ║");
    // puts(  "╠────────────────────────────────────────╣");
    // puts(  "║ 0. 返回                                ║");
    // puts(  "╚════════════════════════════════════════╝");
    printHeader();
    printCenter("ACM 竞赛管理与训练系统");
    printDivider();
    char userLine[SCREEN_CHAR_WIDTH];
    snprintf(userLine, sizeof(userLine), "用户: %s", username);
    printLeft(userLine);
    printDivider();
    printLeft("1. ACM 竞赛简介");
    printLeft("2. ACM 题库");
    printContent("");
    printDivider();
    printCenter("0. 返回");
    printFooter();
    printf("⇒ 请输入选项：");
}
void printACMDetailScreen(){
    // puts("╔════════════════════════════════════════╗");
    // puts("║            ACM 竞赛简介                ║");
    // puts("╠════════════════════════════════════════╣");
    // puts("║ 1. 参赛规则                            ║");
    // puts("║ 2. 评分标准                            ║");
    // puts("║ 3. 赛事构成                            ║");
    // puts("║ 4. 赛事介绍                            ║");
    // puts("║ 5. 历届获奖                            ║");
    // puts("╠────────────────────────────────────────╣");
    // puts("║ 0. 返回主菜单                          ║");
    // puts("╚════════════════════════════════════════╝");
    printHeader();
    printCenter("ACM 竞赛简介");
    printDivider();
    printLeft("1. 参赛规则");
    printLeft("2. 评分标准");
    printLeft("3. 赛事构成");
    printLeft("4. 赛事介绍");
    printRight("5. 历届获奖");
    printDivider();
    printCenter("0. 返回主菜单");
    printFooter();
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
void printCenter(const char* content){
    if(get_real_Length(content, NULL) > SCREEN_CHAR_WIDTH - 2){
        printContent(content);
        return;
    }
    if(content == NULL){
        printf("║ %-*s ║\n", SCREEN_CHAR_WIDTH - 4, "");
        return;
    }
    int contentLen = (int)get_real_Length(content, NULL);
    int count=count_chinese(content, NULL);
    int actualLen = contentLen; 
    int totalPadding = SCREEN_CHAR_WIDTH - 2 - actualLen;
    int leftPadding = totalPadding / 2;
    int rightPadding = totalPadding - leftPadding;
    printf("║");
    for(int i = 0; i < leftPadding; i++){
        printf(" ");
    }
    printf("%s", content);
    for(int i = 0; i < rightPadding; i++){
        printf(" ");
    }
    printf("║\n");
}
void printLeft(const char* content){
    if(get_real_Length(content, NULL) > SCREEN_CHAR_WIDTH - 2){
        printContent(content);
        return;
    }
    if(content == NULL){
        printf("║ %-*s ║\n", SCREEN_CHAR_WIDTH - 4, "");
        return;
    }
    int count=count_chinese(content, NULL);
    int actualWidth=SCREEN_CHAR_WIDTH - 4 + count;
    printf("║ %-*s ║\n", actualWidth, content);
}
void printRight(const char* content){
    if(get_real_Length(content, NULL) > SCREEN_CHAR_WIDTH - 2){
        printContent(content);
        return;
    }
    if(content == NULL){
        printf("║ %-*s ║\n", SCREEN_CHAR_WIDTH - 4, "");
        return;
    }
    int count=count_chinese(content, NULL);
    int actualWidth=SCREEN_CHAR_WIDTH - 4 + count;
    printf("║ %*s ║\n", actualWidth, content);
}
void printContent(const char* content){
    // 存在中文
    if(content == NULL){
        printf("║ %-*s ║\n", SCREEN_CHAR_WIDTH - 4, "");
        return;
    }
    if(get_real_Length(content, NULL) > SCREEN_CHAR_WIDTH - 2){
        printf("- %s -", content);
        return;
    }
    int count=count_chinese(content, NULL);
    // 计算实际宽度
    int actualWidth=SCREEN_CHAR_WIDTH - 4 + count;
    printf("║ %-*s ║\n", actualWidth, content);
}
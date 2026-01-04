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
extern void pauseScreen() {
    printf("⎵> 按 Enter 键继续...\n");
    fflush(stdout);
    cleanBuffer();
    getchar();
}
extern void cleanLine(){
    printf("\r");
    for(int i = 0; i < SCREEN_CHAR_WIDTH; i++){
        printf(" ");
    }
    printf("\r");
}
void moveUp(size_t lines){
    if(lines == 0) return;
    printf("\033[%zuA", lines);
}
void moveDown(size_t lines){
    if(lines == 0) return;
    printf("\033[%zuB", lines);
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
    printf("=> 请输入选项：[ ]\b\b");
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
    printf("=> 请输入选项：[ ]\b\b");
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
    printDivider();
    printLeft("5. 历届获奖");
    printDivider();
    printCenter("0. 返回主菜单");
    printFooter();
    printf("=> 请输入选项：[ ]\b\b");
}
void printACMProblemBankScreen(const char * currentUser){
    // puts("========= ACM 题库 =========");
    // printf("当前用户: %s\n", currentUser);
    // puts("|----------------------------|");
    // puts("|      1. 显示所有题目       |");
    // puts("|      2. 搜索题目           |");
    // puts("|      3. 删除题目           |");
    // puts("|      4. 添加题目           |");
    // puts("|      0. 返回               |");
    // puts("|----------------------------|");
    printHeader();
    printCenter("ACM 题库");
    printDivider();
    char userLine[SCREEN_CHAR_WIDTH];
    snprintf(userLine, sizeof(userLine), "当前用户: %s", currentUser);
    printLeft(userLine);
    printDivider();
    printLeft("1. 显示所有题目");
    printLeft("2. 搜索题目");
    printLeft("3. 删除题目");
    printLeft("4. 添加题目");
    printDivider();
    printCenter("0. 返回");
    printFooter();
    printf("=> 请输入选项：[ ]\b\b");
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
    int innerWidth = SCREEN_CHAR_WIDTH - 2; // 去掉左右边框
    int avail = innerWidth; // center 不应用左右 margin
    if(avail <= 0){
        printf("║");
        for(int i=0;i<innerWidth;i++) putchar(' ');
        printf("║\n");
        return;
    }
    if(content == NULL){
        printf("║");
        for(int i=0;i<avail;i++) putchar(' ');
        printf("║\n");
        return;
    }
    int visible = (int)get_real_Length(content, NULL);
    if(visible > avail){
            // 可见内容超过可用宽度：仅绘制左侧边框，允许内容溢出右侧
            printf("║");
            printf("%s\n", content);
        return;
    }
    int totalPadding = avail - visible;
    int leftPadding = totalPadding / 2;
    int rightPadding = totalPadding - leftPadding;
    printf("║");
    for(int i = 0; i < leftPadding; i++) putchar(' ');
    printf("%s", content);
    for(int i = 0; i < rightPadding; i++) putchar(' ');
    printf("║\n");
}
void printLeft(const char* content){
    int innerWidth = SCREEN_CHAR_WIDTH - 2;
    int avail = innerWidth - SCREEN_MARGIN_LEFT - SCREEN_MARGIN_RIGHT;
    if(avail <= 0){
        printf("║"); for(int i=0;i<innerWidth;i++) putchar(' '); printf("║\n"); return;
    }
    if(content == NULL){
        printf("║");
        for(int i=0;i<SCREEN_MARGIN_LEFT;i++) putchar(' ');
        for(int i=0;i<avail;i++) putchar(' ');
        for(int i=0;i<SCREEN_MARGIN_RIGHT;i++) putchar(' ');
        printf("║\n");
        return;
    }
    int visible = (int)get_real_Length(content, NULL);
    if(visible > avail){
            // 可见内容超过可用宽度：仅绘制左侧边框与左边距，允许内容溢出右侧
            printf("║");
            for(int i=0;i<SCREEN_MARGIN_LEFT;i++) putchar(' ');
            printf("%s\n", content);
        return;
    }
    int rightPadding = avail - visible;
    printf("║");
    for(int i=0;i<SCREEN_MARGIN_LEFT;i++) putchar(' ');
    printf("%s", content);
    for(int i=0;i<rightPadding;i++) putchar(' ');
    for(int i=0;i<SCREEN_MARGIN_RIGHT;i++) putchar(' ');
    printf("║\n");
}
void printRight(const char* content){
    int innerWidth = SCREEN_CHAR_WIDTH - 2;
    int avail = innerWidth - SCREEN_MARGIN_LEFT - SCREEN_MARGIN_RIGHT;
    if(avail <= 0){
        printf("║"); for(int i=0;i<innerWidth;i++) putchar(' '); printf("║\n"); return;
    }
    if(content == NULL){
        printf("║");
        for(int i=0;i<SCREEN_MARGIN_LEFT;i++) putchar(' ');
        for(int i=0;i<avail;i++) putchar(' ');
        for(int i=0;i<SCREEN_MARGIN_RIGHT;i++) putchar(' ');
        printf("║\n");
        return;
    }
    int visible = (int)get_real_Length(content, NULL);
    if(visible > avail){
        printContent(content);
        return;
    }
    int leftPadding = avail - visible;
    printf("║");
    for(int i=0;i<SCREEN_MARGIN_LEFT;i++) putchar(' ');
    for(int i=0;i<leftPadding;i++) putchar(' ');
    printf("%s", content);
    for(int i=0;i<SCREEN_MARGIN_RIGHT;i++) putchar(' ');
    printf("║\n");
}
void printContent(const char* content){
    // 存在中文
    if(content == NULL){
        int innerWidth = SCREEN_CHAR_WIDTH - 2;
        printf("║"); for(int i=0;i<innerWidth;i++) putchar(' '); printf("║\n");
        return;
    }
    int innerWidth = SCREEN_CHAR_WIDTH - 2;
    int avail = innerWidth; // content 不应用左右 margin
    if(avail <= 0){ printf("║"); for(int i=0;i<innerWidth;i++) putchar(' '); printf("║\n"); return; }
    int visible = (int)get_real_Length(content, NULL);
    if(visible > avail){
            // 可见内容超过可用宽度：仅绘制左侧边框，允许内容溢出右侧
            printf("║");
            printf("%s\n", content);
        return;
    }
    int rightPadding = avail - visible;
    printf("║");
    printf("%s", content);
    for(int i=0;i<rightPadding;i++) putchar(' ');
    printf("║\n");
}
void printConsole(const char *contentLine, PrintMarginType marginType){
    switch(marginType){
        case MARGIN_LEFT:
            printLeft(contentLine);
            break;
        case MARGIN_CENTER:
            printCenter(contentLine);
            break;
        case MARGIN_RIGHT:
            printRight(contentLine);
            break;
        default:
            printContent(contentLine);
            break;
    }
}
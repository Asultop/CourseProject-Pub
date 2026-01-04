#include "screenManager.h"
#include "chineseSupport.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#endif

// ============== 动态刷新相关 ==============
static ScreenRefreshCallback g_refreshCallback = NULL;
static volatile sig_atomic_t g_needRefresh = 0;
static volatile sig_atomic_t g_refreshPaused = 0;  // 暂停刷新标志

#ifndef _WIN32
// SIGWINCH 信号处理函数
static void sigwinchHandler(int sig) {
    (void)sig;
    g_needRefresh = 1;
    // 仅在未暂停时调用重绘
    if (!g_refreshPaused && g_refreshCallback != NULL) {
        g_refreshCallback();
    }
}
#endif

// 启用动态刷新
void enableDynamicRefresh(ScreenRefreshCallback callback) {
    g_refreshCallback = callback;
    g_refreshPaused = 0;
#ifndef _WIN32
    struct sigaction sa;
    sa.sa_handler = sigwinchHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, NULL);
#endif
}

// 禁用动态刷新
void disableDynamicRefresh() {
    g_refreshCallback = NULL;
    g_refreshPaused = 0;
#ifndef _WIN32
    signal(SIGWINCH, SIG_DFL);
#endif
}

// 暂停动态刷新（用于显示临时内容时）
void pauseDynamicRefresh() {
    g_refreshPaused = 1;
}

// 恢复动态刷新
void resumeDynamicRefresh() {
    g_refreshPaused = 0;
}

// 设置重绘回调
void setRefreshCallback(ScreenRefreshCallback callback) {
    g_refreshCallback = callback;
}

// 手动触发刷新
void triggerRefresh() {
    if (g_refreshCallback != NULL) {
        g_refreshCallback();
    }
}

// 检查是否需要刷新（用于主循环轮询模式）
int checkAndClearRefreshFlag() {
    if (g_needRefresh) {
        g_needRefresh = 0;
        return 1;
    }
    return 0;
}

// ============== 屏幕宽度获取 ==============
// 获取当前屏幕宽度，若 SCREEN_CHAR_WIDTH 为 -1 则动态获取终端宽度
int getScreenWidth() {
    if (SCREEN_CHAR_WIDTH != -1) {
        return SCREEN_CHAR_WIDTH;
    }
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80; // 默认宽度
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 80; // 默认宽度
#endif
}

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
    int width = getScreenWidth();
    for(int i = 0; i < width; i++){
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
    printConsole("欢迎使用 —— 在线评测系统", MARGIN_CENTER);
    printDivider();
    printConsole("请选择操作", MARGIN_CENTER);
    printDivider();
    printConsole("1. 登录        2. 注册", MARGIN_LEFT);
    printConsole("3. 修改密码    4. 删除用户", MARGIN_LEFT);
    printContent("");
    printDivider();
    printConsole("0. 退出", MARGIN_CENTER);
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
    printConsole("ACM 竞赛管理与训练系统", MARGIN_CENTER);
    printDivider();
    int screenWidth = getScreenWidth();
    char userLine[screenWidth > 0 ? screenWidth : 256];
    snprintf(userLine, sizeof(userLine), "用户: %s", username);
    printConsole(userLine, MARGIN_LEFT);
    printDivider();
    printConsole("1. ACM 竞赛简介", MARGIN_LEFT);
    printConsole("2. ACM 题库", MARGIN_LEFT);
    printContent("");
    printDivider();
    printConsole("0. 返回", MARGIN_CENTER);
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
    printConsole("ACM 竞赛简介", MARGIN_CENTER);
    printDivider();
    printConsole("1. 参赛规则", MARGIN_LEFT);
    printConsole("2. 评分标准", MARGIN_LEFT);
    printConsole("3. 赛事构成", MARGIN_LEFT);
    printConsole("4. 赛事介绍", MARGIN_LEFT);
    printDivider();
    printConsole("5. 历届获奖", MARGIN_LEFT);
    printDivider();
    printConsole("0. 返回主菜单", MARGIN_CENTER);
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
    printConsole("ACM 题库", MARGIN_CENTER);
    printDivider();
    int screenWidth = getScreenWidth();
    char userLine[screenWidth > 0 ? screenWidth : 256];
    snprintf(userLine, sizeof(userLine), "当前用户: %s", currentUser);
    printConsole(userLine, MARGIN_LEFT);
    printDivider();
    printConsole("1. 显示所有题目", MARGIN_LEFT);
    printConsole("2. 搜索题目", MARGIN_LEFT);
    printConsole("3. 删除题目", MARGIN_LEFT);
    printConsole("4. 添加题目", MARGIN_LEFT);
    printDivider();
    printConsole("0. 返回", MARGIN_CENTER);
    printFooter();
    printf("=> 请输入选项：[ ]\b\b");
}
void printHeader(){
    int width = getScreenWidth();
    printf("╔");
    for(int i = 0; i < width - 2; i++){
        printf("═");
    }
    printf("╗\n");
}
void printFooter(){
    int width = getScreenWidth();
    printf("╚");
    for(int i = 0; i < width - 2; i++){
        printf("═");
    }
    printf("╝\n");
}
void printDivider(){
    int width = getScreenWidth();
    printf("╠");
    for(int i = 0; i < width - 2; i++){
        printf("─");
    }
    printf("╣\n");
}
void printCenter(const char* content){
    int width = getScreenWidth();
    int innerWidth = width - 2; // 去掉左右边框
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
    int width = getScreenWidth();
    int innerWidth = width - 2;
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
    int width = getScreenWidth();
    int innerWidth = width - 2;
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
    int width = getScreenWidth();
    if(content == NULL){
        int innerWidth = width - 2;
        printf("║"); for(int i=0;i<innerWidth;i++) putchar(' '); printf("║\n");
        return;
    }
    int innerWidth = width - 2;
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
#define SCREEN_MARGIN_LEFT 1
#include "../src-extends/screenManager.h"
#include <stdio.h>
#include <math.h>
#include "../src-extends/Def.h"
#include "../src-extends/chineseSupport.h"
#include "../src-extends/colorPrint.h"
#include <sys/ioctl.h>
#include <unistd.h>
int main(){
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        printf("ws.col:%d\nws.row:%d\n", w.ws_col, w.ws_row);
    }
    printStartAnima();

    printHeader();
    printContent("This is a test content line.");
    printContent("中文测试内容行");
    printFooter();
    puts("");
    printHeader();
    printConsole("Left Aligned Line Test", MARGIN_LEFT);
    printConsole("左对齐测试行", MARGIN_LEFT);
    printConsole("混合对齐 Mix Alignment", MARGIN_LEFT);
    printDivider();
    printConsole("Right Aligned Line Test", MARGIN_RIGHT);
    printConsole("右对齐测试行", MARGIN_RIGHT);
    printConsole("混合对齐 Mix Alignment", MARGIN_RIGHT);
    printDivider();
    printConsole("Centered Line Test",MARGIN_CENTER);
    printConsole("居中行测试", MARGIN_CENTER);
    printConsole("混合居中 Mix Centeralization", MARGIN_CENTER);
    printFooter();
    puts("");
    printHeader();
    printConsole(rainbowizeString("Left Colorful Line Test"), MARGIN_LEFT);
    printConsole(rainbowizeString("左对齐彩色行测试"), MARGIN_LEFT);
    printConsole(rainbowizeString("混合对齐 Mix Alignment"), MARGIN_LEFT);
    printDivider();
    printConsole(rainbowizeString("Right Colorful Line Test"), MARGIN_RIGHT);
    printConsole(rainbowizeString("右对齐彩色行测试"), MARGIN_RIGHT);
    printConsole(rainbowizeString("混合对齐 Mix Alignment"), MARGIN_RIGHT);
    printDivider();
    printConsole(rainbowizeString("Centered Colorful Line Test"), MARGIN_CENTER);
    printConsole(rainbowizeString("居中彩色行测试"), MARGIN_CENTER);
    printConsole(rainbowizeString("混合居中 Mix Centeralization"), MARGIN_CENTER);
    printFooter();

    puts("面板:");
    printSplashScreen();
    puts("");
    printMainScreen("TestUser");
    puts("");
    printACMDetailScreen();
    puts("");
    printACMProblemBankScreen("TestUser");
    puts("");
    return 0;
}
#define SCREEN_MARGIN_LEFT 1
#include "../src-extends/screenManager.h"
#include <stdio.h>
#include <math.h>
#include "../src-extends/Def.h"
#include "../src-extends/chineseSupport.h"
#include "../src-extends/colorPrint.h"
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
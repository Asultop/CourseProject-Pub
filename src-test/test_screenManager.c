#include "../src-extends/screenManager.h"
#include "stdio.h"
int main(){
    puts("SplashScreen:");
    printSplashScreen();
    puts("\nMainScreen:");
    printMainScreen("TestUser");
    puts("\nACMDetailScreen:");
    printACMDetailScreen();
    puts("\nPrintHeader:");
    printHeader();
    puts("\nPrintFooter:");
    printFooter();
    puts("\nPrintDivider:");
    printDivider();
    puts("\n\n");
    printHeader();
    printDivider();
    printFooter();

    puts("\n");
    printContent("This is a test content line.");
    printContent("中文测试内容行");
    puts("\n");
    printHeader();
    printLeft("Left Aligned Line Test");
    printLeft("左对齐测试行");
    printLeft("混合对齐 Mix Alignment");
    printDivider();
    printRight("Right Aligned Line Test");
    printRight("右对齐测试行");
    printRight("混合对齐 Mix Alignment");
    printDivider();
    printCenteredLine("Centered Line Test");
    printCenteredLine("居中行测试");
    printCenteredLine("混合居中 Mix Centeralization");
    printFooter();

    
    return 0;
}
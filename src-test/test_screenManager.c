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
    return 0;
}
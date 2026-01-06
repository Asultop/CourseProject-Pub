#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include "Def.h"
#include <stdio.h>
#include <sys/ioctl.h>

// 屏幕重绘回调函数类型
typedef void (*ScreenRefreshCallback)(void);

// 获取当前屏幕宽度，若 SCREEN_CHAR_WIDTH 为 -1 则动态获取终端宽度
int getScreenWidth();

// 动态刷新相关函数
void enableDynamicRefresh(ScreenRefreshCallback callback);  // 启用动态刷新
void disableDynamicRefresh();                                // 禁用动态刷新
void pauseDynamicRefresh();                                  // 暂停动态刷新
void resumeDynamicRefresh();                                 // 恢复动态刷新
void setRefreshCallback(ScreenRefreshCallback callback);     // 设置重绘回调
void triggerRefresh();                                       // 手动触发刷新

void printStartAnima();
void printSplashScreen();
void printMainScreen(const char * username);
void printACMDetailScreen();
void printACMProblemBankScreen(const char * currentUser);
void cleanLine();
void cleanScreen();
void pauseScreen();
void cleanBuffer();

void moveUp(size_t lines);
void moveDown(size_t lines);

void printHeader();
void printFooter();
void printDivider();
void printCenter(const char* content);
void printContent(const char * contentLine);
void printLeft(const char * contentLine);
void printRight(const char * contentLine);
void printConsole(const char *contentLine, PrintMarginType marginType);
#endif // SCREEN_MANAGER_H
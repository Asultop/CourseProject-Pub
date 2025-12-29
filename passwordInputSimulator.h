#ifndef PASSWORD_INPUT_SIMULATOR_H
#define PASSWORD_INPUT_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// 跨平台宏定义
#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
    #define IS_WINDOWS 1
    #define BS_CHAR '\b'          // Windows退格键
    #define ARROW_PRE 0           // Windows方向键前缀
    #define ARROW_LEFT 75         // Windows左箭头
    #define ARROW_RIGHT 77        // Windows右箭头
#else
    #include <unistd.h>
    #include <termios.h>
    #define IS_WINDOWS 0
    #define BS_CHAR 127           // Linux/macOS退格键
    #define ARROW_PRE 0x1B        // Linux/macOS方向键前缀(ESC)
#endif

#define PWDLEN 20

// 终端配置（Linux/macOS）
static struct termios g_old_tio;
static int g_terminal_inited = 0;

// 初始化终端：关闭回显、非规范模式
static void term_init() ;

// 恢复终端
static void term_restore() ;

// 跨平台读字符（无回显）
static int getch_safe() ;

// 光标移动：相对当前位置移动n步（n>0右移，n<0左移）
static void move_cursor(int n) ;

// 密码输入函数（终极修复显示问题）
void getpwd(char *pwd, int pwdlen) ;


#endif // PASSWORD_INPUT_SIMULATOR_H
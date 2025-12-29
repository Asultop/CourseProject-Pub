#ifndef PROBLEM_BANK_MANAGER_H
#define PROBLEM_BANK_MANAGER_H
#include <stdio.h>
#include <stdbool.h>

#define MAX_PROBLEMS 2048

typedef struct {
    char id[64];
    char title[256];
    char difficulty[64];
    char type[128];
    char folderName[128];
    char problemPath[512];
} ProblemEntry;

// 从目录扫描所有题目条目，返回条目数
int loadAllProblems(const char* problemsDir, ProblemEntry entries[], int maxEntries);

// 交互式界面：显示问题库子菜单并处理用户操作
void interactiveProblemBank(const char* problemsDir);

#endif // PROBLEM_BANK_MANAGER_H

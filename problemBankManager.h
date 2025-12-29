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

// 删除题目（按 ID 或文件夹名），返回 true 表示删除成功
bool deleteProblemByID(const char* problemsDir, const char* id);

// 交互式添加题目：提示 meta 信息并拷贝用户提供的文件到新题目文件夹
void addProblemInteractive(const char* problemsDir);

#endif // PROBLEM_BANK_MANAGER_H

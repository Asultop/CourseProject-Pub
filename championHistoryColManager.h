#ifndef CHAMPION_HISTORY_COL_MANAGER_H
#define CHAMPION_HISTORY_COL_MANAGER_H
#include <stdio.h>
#include <stdbool.h>

#define MAX_CHAMPION_RECORDS 1024

typedef struct {
    char year[16];
    char location[128];
    char university[256];
    char country[128];
    char teamMembers[512];
    char coach[256];
} ChampionRecord;

// 从文件加载记录，返回加载条目数
int loadChampionHistory(const char* filename, ChampionRecord records[], int maxRecords);

// 按条件查询，任一条件为空字符串则视为通配
int queryChampionByCriteria(ChampionRecord records[], int count, ChampionRecord results[], int maxResults,
                           const char* year, const char* location, const char* university,
                           const char* country, const char* teamMembers, const char* coach);

// 交互式查询（从文件加载并提示用户输入条件，展示结果）
void interactiveChampionQuery(const char* filename);

// 打印单条记录，并根据筛选条件对匹配部分做红色高亮（任一条件为空视为通配）
void printChampionRecord(const ChampionRecord* r,
                         const char* yearFilter, const char* locationFilter,
                         const char* universityFilter, const char* countryFilter,
                         const char* teamMembersFilter, const char* coachFilter);

#endif // CHAMPION_HISTORY_COL_MANAGER_H

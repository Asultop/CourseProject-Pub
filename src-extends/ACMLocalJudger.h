#ifndef ACMLOCALJUDGER_H
#define ACMLOCALJUDGER_H

#include "Def.h"

/*
 * 本地判题模拟器。
 * 给定一个源文件路径和对应的 `ProblemEntry`，函数会在题目目录下查找 `in` 子目录，
 * 将其中的每个输入文件视为一个测试点；对于每个测试点，若在同一题目目录的 `out`
 * 子目录中存在同名基名的 `.out` 文件，则认为该测试点为 AC，否则判为 WA。
 *
 * 返回值为 `JudgeSummary`：`count` 表示测试点数量，`infos[]` 包含每个测试点的判题结果与消息。
 */
JudgeSummary acm_local_judge(const char *source_file_path, const ProblemEntry *entry);

#endif // ACMLOCALJUDGER_H

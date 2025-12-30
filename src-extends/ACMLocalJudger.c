#include "ACMLocalJudger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

static bool ends_with(const char *s, const char *suffix) {
    if (!s || !suffix) return false;
    size_t ls = strlen(s), lx = strlen(suffix);
    if (lx > ls) return false;
    return strcmp(s + ls - lx, suffix) == 0;
}

JudgeSummary acm_local_judge(const char *source_file_path, const ProblemEntry *entry) {
    JudgeSummary summary;
    summary.count = 0;
    for (int i = 0; i < MAX_JUDGES_PER_PROBLEM; ++i) {
        summary.infos[i].result = JUDGE_RESULT_WRONG_ANSWER;
        summary.infos[i].message[0] = '\0';
    }

    if (!entry || !source_file_path) return summary;

    // 查找问题
    char probdir[1024];
    strncpy(probdir, entry->problemPath, sizeof(probdir)-1);
    probdir[sizeof(probdir)-1] = '\0';
    char *dname = dirname(probdir); // 获取题目目录
    if (!dname) return summary;

    char indir[1200];
    char outdir[1200];
    snprintf(indir, sizeof(indir), "%s/in", dname);
    snprintf(outdir, sizeof(outdir), "%s/out", dname);

    DIR *d = opendir(indir);
    if (!d) return summary; // 无法打开输入目录

    // 创建临时目录用于编译和运行
    char tmpTemplate[] = "/tmp/acmjudgeXXXXXX";
    char *tmpdir = mkdtemp(tmpTemplate);
    if (!tmpdir) {
        closedir(d);
        return summary;
    }

    // 编译源文件
    char exePath[1400];
    snprintf(exePath, sizeof(exePath), "%s/run_exec", tmpdir);
    char compileLog[1400];
    snprintf(compileLog, sizeof(compileLog), "%s/compile.log", tmpdir);
    char compileCmd[2048];
    snprintf(compileCmd, sizeof(compileCmd), "g++ -std=c++17 -O2 -o '%s' '%s' 2> '%s'", exePath, source_file_path, compileLog);
    int cret = system(compileCmd);

    // 遍历输入文件
    struct dirent *de;
    int idx = 0;
    while ((de = readdir(d)) != NULL && idx < MAX_JUDGES_PER_PROBLEM) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        // 仅处理 .in 文件
        if (!ends_with(de->d_name, ".in")) continue;
        char inpath[1400];
        snprintf(inpath, sizeof(inpath), "%s/%s", indir, de->d_name);
        struct stat st;
        if (stat(inpath, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        // 准备评测条目
        char base[512]; strncpy(base, de->d_name, sizeof(base)-1); base[sizeof(base)-1] = '\0';
        char *dot = strrchr(base, '.'); if (dot) *dot = '\0';
        char expectedOut[1400];
        snprintf(expectedOut, sizeof(expectedOut), "%s/%s.out", outdir, base);

        // 初始化结果条目
        summary.infos[idx].result = JUDGE_RESULT_WRONG_ANSWER;
        summary.infos[idx].message[0] = '\0';

        if (cret != 0) {
            // 编译失败
            summary.infos[idx].result = JUDGE_RESULT_COMPILE_ERROR;
            strncpy(summary.infos[idx].message, "编译错误", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            idx++;
            continue;
        }

        // 运行可执行文件
        char tmpOut[1400];
        snprintf(tmpOut, sizeof(tmpOut), "%s/out_%d.txt", tmpdir, idx);
        char runCmd[2048];
        snprintf(runCmd, sizeof(runCmd), "'%s' < '%s' > '%s' 2> '%s/run_err_%d.log'", exePath, inpath, tmpOut, tmpdir, idx);
        int rret = system(runCmd);
        if (rret != 0) {
            summary.infos[idx].result = JUDGE_RESULT_RUNTIME_ERROR;
            strncpy(summary.infos[idx].message, "运行时错误", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            idx++;
            continue;
        }

        // 比较输出文件
        FILE *f1 = fopen(tmpOut, "rb");
        FILE *f2 = fopen(expectedOut, "rb");
        if (!f2) {
            // 缺少预期输出，视为答案错误
            summary.infos[idx].result = JUDGE_RESULT_WRONG_ANSWER;
            strncpy(summary.infos[idx].message, "答案有误，请检查你的代码", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            if (f1) fclose(f1);
            idx++;
            continue;
        }

        bool same = true;
        if (!f1) {
            same = false;
        } else {
            int c1, c2;
            do {
                c1 = fgetc(f1);
                c2 = fgetc(f2);
                if (c1 != c2) { same = false; break; }
            } while (c1 != EOF && c2 != EOF);
            if (c1 != c2) same = false;
            fclose(f1);
            fclose(f2);
        }

        if (same) {
            summary.infos[idx].result = JUDGE_RESULT_ACCEPTED;
            strncpy(summary.infos[idx].message, "答案正确", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
        } else {
            summary.infos[idx].result = JUDGE_RESULT_WRONG_ANSWER;
            strncpy(summary.infos[idx].message, "答案有误，请检查你的代码", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
        }

        idx++;
    }

    closedir(d);

    // 清理临时文件和目录
    // 仅删除已知文件，避免误删
    char rmCmd[1600];
    #ifdef __linux__
    snprintf(rmCmd, sizeof(rmCmd), "rm -f '%s' '%s'/*.txt '%s'/*.log 2>/dev/null; rmdir '%s' 2>/dev/null", exePath, tmpdir, tmpdir, tmpdir);
    #else
    snprintf(rmCmd, sizeof(rmCmd), "del /Q \"%s\" \"%s\\*.txt\" \"%s\\*.log\" >nul 2>&1 & rmdir \"%s\" >nul 2>&1", exePath, tmpdir, tmpdir, tmpdir);
    #endif
    system(rmCmd);

    summary.count = idx;
    return summary;
}

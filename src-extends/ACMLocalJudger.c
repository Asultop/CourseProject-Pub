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
#include <signal.h>
#include <errno.h>
#include <time.h>

static bool ends_with(const char *s, const char *suffix) {
    if (!s || !suffix) return false;
    size_t ls = strlen(s), lx = strlen(suffix);
    if (lx > ls) return false;
    return strcmp(s + ls - lx, suffix) == 0;
}

// 规范化文件内容为仅包含空格分隔的标记字符串
static size_t normalize_file_tokens(const char *path, char *outbuf, size_t outsz) {
    if (!path || !outbuf || outsz == 0) return 0;
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    size_t wrote = 0;
    char token[512];
    bool first = true;
    while (fscanf(f, "%511s", token) == 1) {
        size_t tlen = strlen(token);
        if (!first) {
            if (wrote + 1 < outsz) outbuf[wrote++] = ' ';
            else break;
        }
        first = false;
        if (wrote + tlen < outsz) {
            memcpy(outbuf + wrote, token, tlen);
            wrote += tlen;
        } else {
            /* truncated */
            size_t can = outsz - wrote - 1;
            if (can > 0) {
                memcpy(outbuf + wrote, token, can);
                wrote += can;
            }
            break;
        }
    }
    outbuf[wrote] = '\0';
    fclose(f);
    return wrote;
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
    char exePath[2048];
    snprintf(exePath, sizeof(exePath), "%s/run_exec", tmpdir);
    char compileLog[2048];
    snprintf(compileLog, sizeof(compileLog), "%s/compile.log", tmpdir);
    char compileCmd[8192];
    snprintf(compileCmd, sizeof(compileCmd), "g++ -std=c++17 -O2 -o '%s' '%s' 2> '%s'", exePath, source_file_path, compileLog);
    int cret = system(compileCmd);

    // 遍历输入文件
    struct dirent *de;
    int idx = 0;
    while ((de = readdir(d)) != NULL && idx < MAX_JUDGES_PER_PROBLEM) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        // 仅处理 .in 文件
        if (!ends_with(de->d_name, ".in")) continue;
        // 构造输入文件路径并检查
        char inpath[2048];
        snprintf(inpath, sizeof(inpath), "%s/%s", indir, de->d_name);
        struct stat st;
        if (stat(inpath, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        // 准备评测条目
        char base[512]; strncpy(base, de->d_name, sizeof(base)-1); base[sizeof(base)-1] = '\0';
        char *dot = strrchr(base, '.'); if (dot) *dot = '\0';

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

        // 运行可执行文件，使用 fork/exec 并支持超时
        char expectedOut[2048];
        snprintf(expectedOut, sizeof(expectedOut), "%s/%s.out", outdir, base);
        char tmpOut[4096];
        snprintf(tmpOut, sizeof(tmpOut), "%s/out_%d.txt", tmpdir, idx);
        char tmpErr[4096];
        snprintf(tmpErr, sizeof(tmpErr), "%s/run_err_%d.log", tmpdir, idx);

        pid_t pid = fork();
        if (pid == -1) {
            summary.infos[idx].result = JUDGE_RESULT_RUNTIME_ERROR;
            strncpy(summary.infos[idx].message, "运行时错误", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            idx++;
            continue;
        }
        if (pid == 0) {
            // child: redirect stdin/stdout/stderr and exec
            FILE *fin = fopen(inpath, "rb");
            if (fin) { dup2(fileno(fin), STDIN_FILENO); fclose(fin); }
            FILE *fout = fopen(tmpOut, "wb");
            if (fout) { dup2(fileno(fout), STDOUT_FILENO); fclose(fout); }
            FILE *ferr = fopen(tmpErr, "wb");
            if (ferr) { dup2(fileno(ferr), STDERR_FILENO); fclose(ferr); }
            // exec
            execl(exePath, exePath, (char*)NULL);
            _exit(127);
        }

        // parent: wait with timeout (milliseconds)
        int status = 0;
        int waited_ms = 0;
        const int poll_ms = 10;
        int finished = 0;
        while (1) {
            pid_t w = waitpid(pid, &status, WNOHANG);
            if (w == -1) {
                // error
                break;
            }
            if (w == pid) { finished = 1; break; }
            if (waited_ms >= MAX_JUDGES_TIMELIMIT_MSEC) {
                // timeout: kill child
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                summary.infos[idx].result = JUDGE_RESULT_TIME_LIMIT_EXCEEDED;
                strncpy(summary.infos[idx].message, "答案有误，请检查你的代码", sizeof(summary.infos[idx].message)-1);
                summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
                finished = 1;
                break;
            }
            struct timespec ts; ts.tv_sec = 0; ts.tv_nsec = poll_ms * 1000000;
            nanosleep(&ts, NULL);
            waited_ms += poll_ms;
        }

        if (!finished) {
            // some error occurred waiting
            summary.infos[idx].result = JUDGE_RESULT_RUNTIME_ERROR;
            strncpy(summary.infos[idx].message, "运行时错误", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            idx++;
            continue;
        }

        // if timed out we already set result; skip to next
        if (summary.infos[idx].result == JUDGE_RESULT_TIME_LIMIT_EXCEEDED) {
            idx++;
            continue;
        }

        // child finished, check exit status
        if (WIFSIGNALED(status)) {
            summary.infos[idx].result = JUDGE_RESULT_RUNTIME_ERROR;
            strncpy(summary.infos[idx].message, "运行时错误", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            idx++;
            continue;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
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

        /* 比较时忽略格式（空白差异），只按 token 比较数据 */
        char norm1[4096]; char norm2[4096];
        normalize_file_tokens(tmpOut, norm1, sizeof(norm1));
        normalize_file_tokens(expectedOut, norm2, sizeof(norm2));
        bool same = (strcmp(norm1, norm2) == 0);
        if (f1) fclose(f1);
        if (f2) fclose(f2);

        if (same) {
            summary.infos[idx].result = JUDGE_RESULT_ACCEPTED;
            strncpy(summary.infos[idx].message, "答案正确", sizeof(summary.infos[idx].message)-1);
            summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
        } else {
            summary.infos[idx].result = JUDGE_RESULT_WRONG_ANSWER;
            /* 读取程序 stdout 内容并附加到 message，避免过长。若规范化输出非空则附加规范化结果以便查看数据。 */
            char outbuf[512]; outbuf[0] = '\0';
            char normout[1024]; normout[0] = '\0';
            FILE *fout = fopen(tmpOut, "rb");
            if (fout) {
                size_t r = fread(outbuf, 1, sizeof(outbuf)-1, fout);
                outbuf[r] = '\0';
                fclose(fout);
                while (r > 0 && (outbuf[r-1] == '\n' || outbuf[r-1] == '\r' || outbuf[r-1] == ' ' || outbuf[r-1] == '\t')) { outbuf[r-1] = '\0'; r--; }
            }
            normalize_file_tokens(tmpOut, normout, sizeof(normout));
            if (normout[0] != '\0') {
                snprintf(summary.infos[idx].message, sizeof(summary.infos[idx].message), "答案有误，请检查你的代码; stdout: %.200s", normout);
            } else if (outbuf[0] != '\0') {
                snprintf(summary.infos[idx].message, sizeof(summary.infos[idx].message), "答案有误，请检查你的代码; stdout: %.200s", outbuf);
            } else {
                strncpy(summary.infos[idx].message, "答案有误，请检查你的代码", sizeof(summary.infos[idx].message)-1);
                summary.infos[idx].message[sizeof(summary.infos[idx].message)-1] = '\0';
            }
        }

        idx++;
    }

    closedir(d);

    // 清理临时文件和目录
    // 仅删除已知文件，避免误删
    char rmCmd[8192];
    #ifdef __linux__
    snprintf(rmCmd, sizeof(rmCmd), "rm -f '%s' '%s'/*.txt '%s'/*.log 2>/dev/null; rmdir '%s' 2>/dev/null", exePath, tmpdir, tmpdir, tmpdir);
    #else
    snprintf(rmCmd, sizeof(rmCmd), "del /Q \"%s\" \"%s\\*.txt\" \"%s\\*.log\" >nul 2>&1 & rmdir \"%s\" >nul 2>&1", exePath, tmpdir, tmpdir, tmpdir);
    #endif
    system(rmCmd);

    summary.count = idx;
    return summary;
}

#include "problemBankManager.h"
#include "Def.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

static void trim_newline(char* s){ if(!s) return; size_t i=strlen(s); if(i==0) return; if(s[i-1]=='\n') s[i-1]='\0'; if(i>1 && s[i-2]=='\r') s[i-2]='\0'; }

// 读取小文件到动态分配字符串（调用者负责free）
static char* readFileToString(const char* path){
    FILE* f = fopen(path, "r");
    if(!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if(len < 0) {
        fclose(f); return NULL; 
    }
    char* buf = (char*)malloc((size_t)len + 1);
    if(!buf){ 
        fclose(f); return NULL; 
    }
    size_t read = fread(buf, 1, (size_t)len, f);
    buf[read] = '\0';
    fclose(f);
    return buf;
}

// 不区分大小写包含匹配
static bool contains_case_insensitive(const char* text, const char* pat){
    if(!pat || pat[0]=='\0') return true;
    if(!text) return false;
    char *lowText = strdup(text);
    char *lowPat = strdup(pat);
    if(!lowText || !lowPat){
        free(lowText); free(lowPat); return false; 
    }
    for(char* p = lowText; *p; ++p) *p = (char)tolower((unsigned char)*p);
    for(char* p = lowPat; *p; ++p) *p = (char)tolower((unsigned char)*p);
    bool res = strstr(lowText, lowPat) != NULL;
    free(lowText); free(lowPat);
    return res;
}

int loadAllProblems(const char* problemsDir, ProblemEntry entries[], int maxEntries){
    if(!problemsDir || !entries) return 0;
    DIR* d = opendir(problemsDir);
    if(!d) return 0;
    struct dirent* de;
    int idx = 0;
    while((de = readdir(d)) != NULL && idx < maxEntries){
        if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        // 构造子目录路径
        char subdir[1024];
        snprintf(subdir, sizeof(subdir), "%s/%s", problemsDir, de->d_name);
        struct stat st;
        if(stat(subdir, &st) != 0) continue;
        if(!S_ISDIR(st.st_mode)) continue;

        // 读取 MetaData 文件
        char metaPath[1200];
        snprintf(metaPath, sizeof(metaPath), "%s/MetaData", subdir);
        FILE* mf = fopen(metaPath, "r");
        if(!mf) continue;
        char line[1024];
        if(fgets(line, sizeof(line), mf) == NULL){ fclose(mf); continue; }
        fclose(mf);
        trim_newline(line);
        // id|题干|难度|类型
        char* tmp = strdup(line);
        char* tok = strtok(tmp, "|");
        char id[64] = ""; char title[256] = ""; char diff[64] = ""; char type[128] = "";
        if(tok) strncpy(id, tok, sizeof(id)-1);
        tok = strtok(NULL, "|"); if(tok) strncpy(title, tok, sizeof(title)-1);
        tok = strtok(NULL, "|"); if(tok) strncpy(diff, tok, sizeof(diff)-1);
        tok = strtok(NULL, "|"); if(tok) strncpy(type, tok, sizeof(type)-1);
        free(tmp);

        ProblemEntry e;
        memset(&e, 0, sizeof(e));
        if(id[0]) strncpy(e.id, id, sizeof(e.id)-1); else strncpy(e.id, de->d_name, sizeof(e.id)-1);
        strncpy(e.title, title, sizeof(e.title)-1);
        strncpy(e.difficulty, diff, sizeof(e.difficulty)-1);
        strncpy(e.type, type, sizeof(e.type)-1);
        strncpy(e.folderName, de->d_name, sizeof(e.folderName)-1);
        snprintf(e.problemPath, sizeof(e.problemPath), "%s/problem.txt", subdir);
        entries[idx++] = e;
    }
    closedir(d);
    return idx;
}

static void printProblemDetails(const ProblemEntry* e){
    if(!e) return;
    puts(  "|----------------------------|");
    printf("|  ID: %s\n", e->id);
    printf("|  标题: %s\n", e->title);
    printf("|  难度: %s\n", e->difficulty);
    printf("|  类型: %s\n", e->type);
    printf("|  题目文件: %s\n", e->problemPath);
    char* stmt = readFileToString(e->problemPath);
    if(stmt){
        printf("\n题干：\n%s\n", stmt);
        free(stmt);
    } else {
        printf("\n题干：无法读取 %s\n", e->problemPath);
    }
}
void cleanScreen();
void pauseScreen(){
    printf("=> 按任意键继续...");
    while(getchar()!='\n');
    getchar(); // 等待用户按键
}
void interactiveProblemBank(const char* problemsDir){
    while(true){
        cleanScreen();
        puts("========= ACM 题库 =========");
        puts("|----------------------------|");
        puts("|      1. 显示所有题目       |");
        puts("|      2. 搜索题目           |");
        puts("|      0. 返回               |");
        puts("|----------------------------|");
        printf("=> 请输入选项：[ ]\b\b");
        int choice = 0; 
        //CleanBuffer
        if(scanf("%d", &choice) != 1){
            int c;
            while((c=getchar())!='\n' && c!=EOF);
            printf("?> 无效输入，请重试。\n");
            pauseScreen();
            continue; 
        }
        if(choice == 0) return;
        if(choice == 1){
            ProblemEntry entries[MAX_PROBLEMS];
            int cnt = loadAllProblems(problemsDir, entries, MAX_PROBLEMS);
            if(cnt == 0){ 
                printf("?> 未找到题目。\n"); 
                pauseScreen();
                continue; 
            }
            printf("-------- 题目列表 (%d) ---------\n", cnt);
            for(int i=0;i<cnt;i++){
                printf("| %s - %s\n", entries[i].id, entries[i].title);
            }
            puts("|------------------------------");
            printf("=> 输入题目 ID 打开详情，或 0 返回：");
            char idBuf[128];
            // 清除换行并读取字符串
            int rc = scanf("%s", idBuf);
            if(rc != 1) {
                int c; 
                while((c=getchar())!='\n' && c!=EOF); 
                continue; 
            }
            if(strcmp(idBuf, "0") == 0) 
                continue;
            // 查找
            int found = -1;
            for(int i=0;i<cnt;i++) 
                if(strcmp(entries[i].id, idBuf) == 0 || strcmp(entries[i].folderName, idBuf) == 0){
                    found = i;
                    break; 
                }
            if(found == -1){ 
                printf("?> 未找到 ID: %s\n", idBuf); 
                pauseScreen();
                continue; 
            }
            printProblemDetails(&entries[found]);
            printf("\n1. 提交结果\n0. 返回\n=> 请选择：");
            int sub; 
            // CleanBuffer
            if(scanf("%d", &sub) != 1) { 
                int c; 
                while((c=getchar())!='\n' && c!=EOF);
                continue; 
            }
            if(sub == 1){
                // To-Do
                printf("提交功能暂未实现。\n");
            }
            pauseScreen();
            continue;
        } else if(choice == 2){
            // 交互式筛选：标题，题干，难度，类型
            char buf[256];
            // CleanBuffer
            int c; 
            while((c=getchar())!='\n' && c!=EOF); 
            printf("输入标题关键字（留空通配）："); 
            fgets(buf, sizeof(buf), stdin); 
            trim_newline(buf); 
            char titleFilter[256]; 
            strncpy(titleFilter, buf, sizeof(titleFilter)-1);
            printf("输入题干关键字（留空通配）："); 
            fgets(buf, sizeof(buf), stdin); 
            trim_newline(buf); 
            char stmtFilter[256]; 
            strncpy(stmtFilter, buf, sizeof(stmtFilter)-1);
            printf("输入难度（留空通配）："); 
            fgets(buf, sizeof(buf), stdin); 
            trim_newline(buf); 
            char diffFilter[64]; 
            strncpy(diffFilter, buf, sizeof(diffFilter)-1);
            printf("输入类型（留空通配）："); 
            fgets(buf, sizeof(buf), stdin); 
            trim_newline(buf); 
            char typeFilter[128]; 
            strncpy(typeFilter, buf, sizeof(typeFilter)-1);

            ProblemEntry entries[MAX_PROBLEMS];
            int cnt = loadAllProblems(problemsDir, entries, MAX_PROBLEMS);
            if(cnt == 0){ 
                printf("?> 未找到题目。\n"); 
                pauseScreen();
                continue; 
            }
            ProblemEntry results[MAX_PROBLEMS]; 
            int rcount = 0;
            for(int i=0;i<cnt;i++){
                // 标题、难度、类型基于 metadata；题干需要读取文件
                bool ok = true;
                if(!contains_case_insensitive(entries[i].title, titleFilter)) ok = false;
                if(!contains_case_insensitive(entries[i].difficulty, diffFilter)) ok = false;
                if(!contains_case_insensitive(entries[i].type, typeFilter)) ok = false;
                if(ok && stmtFilter[0] != '\0'){
                    char* stmt = readFileToString(entries[i].problemPath);
                    if(stmt){
                        if(!contains_case_insensitive(stmt, stmtFilter)) ok = false;
                        free(stmt);
                    } else {
                        ok = false;
                    }
                }
                if(ok){ 
                    results[rcount++] = entries[i]; 
                    if(rcount >= MAX_PROBLEMS) 
                        break; 
                }
            }
            if(rcount == 0){
                printf("?> 未找到匹配题目。\n"); 
                pauseScreen();
                continue; 
            }
            printf("√> 找到 %d 条题目：\n", rcount);
            for(int i=0;i<rcount;i++) 
                printf("%s - %s\n", results[i].id, results[i].title);

            printf("=> 输入题目 ID 打开详情，或 0 返回："); 
            char idBuf[128]; 
            if(scanf("%s", idBuf) != 1) 
                continue; 
            if(strcmp(idBuf, "0")==0) 
                continue;
            int found = -1; 
            for(int i=0;i<rcount;i++) 
                if(strcmp(results[i].id, idBuf) == 0 || strcmp(results[i].folderName, idBuf) == 0){ 
                    found = i; 
                    break; 
                }
            if(found == -1){ 
                printf("?> 未找到 ID: %s\n", idBuf); 
                pauseScreen();
                continue; 
            }
            printProblemDetails(&results[found]);
            printf("\n1. 提交结果\n0. 返回\n=> 请选择："); 
            int sub; 
            if(scanf("%d", &sub) != 1) { 
                int c; 
                while((c=getchar())!='\n' && c!=EOF); 
                continue; 
            }
            if(sub == 1) {
                printf("提交功能暂未实现。\n");
            }
            pauseScreen();
            continue;
        } else {
            printf("?> 无效选项。\n");
            pauseScreen();
            continue;
        }
    }
}

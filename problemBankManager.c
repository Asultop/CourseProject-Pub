#include "problemBankManager.h"
#include "Def.h"
#include "fileHelper.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/select.h>
static void trim_newline(char* s) {
	if(!s) return;
	size_t i=strlen(s);
	if(i==0) return;
	if(s[i-1]=='\n') s[i-1]='\0';
	if(i>1 && s[i-2]=='\r') s[i-2]='\0';
}
// 读取小文件到动态分配字符串（调用者负责free）
static char* readFileToString(const char* path) {
	FILE* f = fopen(path, "r");
	if(!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(len < 0) {
		fclose(f);
		return NULL;
	}
	char* buf = (char*)malloc((size_t)len + 1);
	if(!buf) {
		fclose(f);
		return NULL;
	}
	size_t read = fread(buf, 1, (size_t)len, f);
	buf[read] = '\0';
	fclose(f);
	return buf;
}
// 打印面包屑头部，例如： 欢迎>>系统>>ACM 题库>>子页面
static void print_breadcrumb(const char* sub) {
	cleanScreen();
	if(sub && sub[0]) {
		printf("========= ACM 题库>>%s =========\n", sub);
	} else {
		printf("========= ACM 题库 =========\n");
	}
}
// 不区分大小写包含匹配
static bool contains_case_insensitive(const char* text, const char* pat) {
	if(!pat || pat[0]=='\0') return true;
	if(!text) return false;
	char *lowText = strdup(text);
	char *lowPat = strdup(pat);
	if(!lowText || !lowPat) {
		free(lowText);
		free(lowPat);
		return false;
	}
	for (char* p = lowText; *p; ++p) *p = (char)tolower((unsigned char)*p);
	for (char* p = lowPat; *p; ++p) *p = (char)tolower((unsigned char)*p);
	bool res = strstr(lowText, lowPat) != NULL;
	free(lowText);
	free(lowPat);
	return res;
}
int loadAllProblems(const char* problemsDir, ProblemEntry entries[], int maxEntries) {
	if(!problemsDir || !entries) return 0;
	DIR* d = opendir(problemsDir);
	if(!d) return 0;
	struct dirent* de;
	int idx = 0;
	while((de = readdir(d)) != NULL && idx < maxEntries) {
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
		if(fgets(line, sizeof(line), mf) == NULL) {
			fclose(mf);
			continue;
		}
		fclose(mf);
		trim_newline(line);
		// id|题干|难度|类型
		char* tmp = strdup(line);
		char* tok = strtok(tmp, "|");
		char id[64] = "";
		char title[256] = "";
		char diff[64] = "";
		char type[128] = "";
		if(tok) strncpy(id, tok, sizeof(id)-1);
		tok = strtok(NULL, "|");
		if(tok) strncpy(title, tok, sizeof(title)-1);
		tok = strtok(NULL, "|");
		if(tok) strncpy(diff, tok, sizeof(diff)-1);
		tok = strtok(NULL, "|");
		if(tok) strncpy(type, tok, sizeof(type)-1);
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
// 递归删除目录及其内容
static bool remove_dir_recursive(const char* path) {
	DIR* d = opendir(path);
	if(!d) {
		// 不是目录或无法打开，尝试作为文件删除
		if(remove(path) == 0) return true;
		return false;
	}
	struct dirent* de;
	char child[1024];
	while((de = readdir(d)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		snprintf(child, sizeof(child), "%s/%s", path, de->d_name);
		struct stat st;
		if(stat(child, &st) == 0 && S_ISDIR(st.st_mode)) {
			remove_dir_recursive(child);
		} else {
			remove(child);
		}
	}
	closedir(d);
	if(rmdir(path) == 0) return true;
	return false;
}
// 删除题目实现
bool deleteProblemByID(const char* problemsDir, const char* id) {
	if(!problemsDir || !id) return false;
	ProblemEntry entries[MAX_PROBLEMS];
	int cnt = loadAllProblems(problemsDir, entries, MAX_PROBLEMS);
	if(cnt == 0) return false;
	int found = -1;
	for (int i=0;i<cnt;i++) {
		if(strcmp(entries[i].id, id) == 0 || strcmp(entries[i].folderName, id) == 0) {
			found = i;
			break;
		}
	}
	if(found == -1) return false;
	char path[1024];
	snprintf(path, sizeof(path), "%s/%s", problemsDir, entries[found].folderName);
	return remove_dir_recursive(path);
}
// 简单文件复制
static bool copy_file(const char* src, const char* dst) {
	FILE* fsrc = fopen(src, "rb");
	if(!fsrc) return false;
	FILE* fdst = fopen(dst, "wb");
	if(!fdst) {
		fclose(fsrc);
		return false;
	}
	char buf[4096];
	size_t n;
	while((n = fread(buf,1,sizeof(buf),fsrc))>0) {
		fwrite(buf,1,n,fdst);
	}
	fclose(fsrc);
	fclose(fdst);
	return true;
}
// 交互式添加题目
void addProblemInteractive(const char* problemsDir) {
	char id[128];
	char title[256];
	char diff[64];
	char type[128];
	int c;
	while((c=getchar())!='\n' && c!=EOF);
	printf("=> 输入题目 ID（将作为文件夹名）：");
	fgets(id, sizeof(id), stdin);
	trim_newline(id);
	if(id[0]=='\0') {
		printf("x> ID 不能为空\n");
		return;
	}
	// 检查是否存在
	char newDir[1024];
	snprintf(newDir, sizeof(newDir), "%s/%s", problemsDir, id);
	struct stat st;
	if(stat(newDir, &st) == 0) {
		printf("x> 题目目录已存在：%s \n", newDir);
		return;
	}
	printf("=> 输入标题：");
	fgets(title, sizeof(title), stdin);
	trim_newline(title);
	printf("=> 输入难度：");
	fgets(diff, sizeof(diff), stdin);
	trim_newline(diff);
	printf("=> 输入类型：");
	fgets(type, sizeof(type), stdin);
	trim_newline(type);
	// 创建目录
	if(mkdir(newDir, 0755) != 0) {
		printf("x> 无法创建目录：%s (errno=%d)\n", newDir, errno);
		return;
	}
	// 写 MetaData
	char metaPath[1200];
	snprintf(metaPath, sizeof(metaPath), "%s/MetaData", newDir);
	FILE* mf = fopen(metaPath, "w");
	if(!mf) {
		printf("x> 无法写入 MetaData\n");
		return;
	}
	fprintf(mf, "%s|%s|%s|%s\n", id, title, diff, type);
	fclose(mf);
	printf("=> 请逐行输入题目文件的路径（需要三个文件：analyzing.txt, general_solution.cpp, problem.txt）。\n");
	printf("   粘贴路径时可带引号，程序会自动去掉引号；文件不存在时会继续提示，输入空行取消并删除已创建目录。\n");
	char pathbuf[1024];
	bool gotAnalyzing = false, gotSolution = false, gotProblem = false;
	typedef struct { char name[256]; char src[1024]; } PairFile;
	PairFile inFiles[512]; int inCnt = 0;
	PairFile outFiles[512]; int outCnt = 0;
	while(true) {
		printf("=> 文件路径（空行结束）：");
		if(!fgets(pathbuf, sizeof(pathbuf), stdin)) break;
		trim_newline(pathbuf);
		// 去掉开头和结尾的引号
		if(pathbuf[0] == '"' || pathbuf[0] == '\'') {
			size_t L = strlen(pathbuf);
			if(L>0 && (pathbuf[L-1] == '"' || pathbuf[L-1] == '\'')) {
				// 移除两侧引号
				memmove(pathbuf, pathbuf+1, L-2);
				pathbuf[L-2] = '\0';
			}
		}
		if(pathbuf[0] == '\0') {
			// 用户结束输入 — 先检查必需三项
			if(!(gotAnalyzing && gotSolution && gotProblem)) {
				printf("x> 未提供所有必需文件，取消添加并删除已创建目录...\n");
				remove_dir_recursive(newDir);
				return;
			}
			// 检查 .in -> .out 配对：每个 .in 必须对应存在同名 .out（去掉扩展名比较）
			for(int i=0;i<inCnt;i++) {
				char *iname = inFiles[i].name;
				char *dot = strrchr(iname, '.');
				size_t baselen = dot ? (size_t)(dot - iname) : strlen(iname);
				bool found = false;
				for(int j=0;j<outCnt;j++) {
					char *oname = outFiles[j].name;
					char *odot = strrchr(oname, '.');
					size_t obaselen = odot ? (size_t)(odot - oname) : strlen(oname);
					if(obaselen == baselen && strncmp(iname, oname, baselen) == 0) { found = true; break; }
				}
				if(!found) {
					printf("x> 找不到与 %s 配对的 .out 文件，取消添加并删除已创建目录...\n", inFiles[i].name);
					remove_dir_recursive(newDir);
					return;
				}
			}
			// 创建 in/ out/ 目录（若需要）并拷贝
			char indir[1200];
			char outdir[1200];
			if(inCnt > 0) {
				snprintf(indir, sizeof(indir), "%s/in", newDir);
				mkdir(indir, 0755);
			}
			if(outCnt > 0) {
				snprintf(outdir, sizeof(outdir), "%s/out", newDir);
				mkdir(outdir, 0755);
			}
			for(int i=0;i<inCnt;i++) {
				char dst[1400];
				snprintf(dst, sizeof(dst), "%s/%s", indir, inFiles[i].name);
				if(copy_file(inFiles[i].src, dst)) printf("√ 已拷贝输入文件 %s 到 in/\n", inFiles[i].name); else printf("x> 拷贝失败：%s\n", inFiles[i].src);
			}
			for(int i=0;i<outCnt;i++) {
				char dst[1400];
				snprintf(dst, sizeof(dst), "%s/%s", outdir, outFiles[i].name);
				if(copy_file(outFiles[i].src, dst)) printf("√ 已拷贝输出文件 %s 到 out/\n", outFiles[i].name); else printf("x> 拷贝失败：%s\n", outFiles[i].src);
			}
			printf("√> 必需文件已全部提供，且 .in/.out 配对通过。\n");
			break;
		}
		// 移除可能的起止引号（再次更稳健）
		char *pstart = pathbuf;
		while(*pstart == '"' || *pstart == '\'') pstart++;
		char *pend = pstart + strlen(pstart) - 1;
		while(pend > pstart && (*pend == '"' || *pend == '\'')) {
			*pend = '\0';
			pend--;
		}
		if(!fileExists(pstart)) {
			printf("?> 源文件不存在：%s，请重新输入或输入空行取消。\n", pstart);
			continue;
		}
		// 获取文件名
		char *b = basename(pstart);
		// 小写判断
		char lowb[256];
		strncpy(lowb, b, sizeof(lowb)-1);
		lowb[sizeof(lowb)-1]='\0';
		for (char* q=lowb; *q; ++q) *q = (char)tolower((unsigned char)*q);
		char dst[1200];
		if(strcmp(lowb, "analyzing.txt") == 0) {
			snprintf(dst, sizeof(dst), "%s/analyzing.txt", newDir);
			if(copy_file(pstart, dst)) {
				gotAnalyzing = true;
				printf("√ 已拷贝解析至 analyzing.txt\n");
			} else printf("x> 拷贝失败：%s\n", pstart);
		} else if(strcmp(lowb, "general_solution.cpp") == 0) {
			snprintf(dst, sizeof(dst), "%s/general_solution.cpp", newDir);
			if(copy_file(pstart, dst)) {
				gotSolution = true;
				printf("√ 已拷贝题解至 general_solution.cpp\n");
			} else printf("x> 拷贝失败：%s\n", pstart);
		} else if(strcmp(lowb, "problem.txt") == 0) {
			snprintf(dst, sizeof(dst), "%s/problem.txt", newDir);
			if(copy_file(pstart, dst)) {
				gotProblem = true;
				printf("√ 已拷贝题干至 problem.txt\n");
			} else printf("x> 拷贝失败：%s\n", pstart);
		} else if(strlen(lowb) > 3 && strcmp(lowb + strlen(lowb) - 3, ".in") == 0) {
			// 收集 .in 文件，稍后统一拷贝到 in/，但先按文件名去重以避免重复拷贝
			{
				bool dup = false;
				for(int k=0;k<inCnt;k++) {
					if(strcmp(inFiles[k].name, b) == 0) { dup = true; break; }
				}
				if(dup) {
					printf("?> 已收集过输入文件 %s，忽略重复项\n", b);
				} else if(inCnt < (int)(sizeof(inFiles)/sizeof(inFiles[0]))) {
					strncpy(inFiles[inCnt].name, b, sizeof(inFiles[inCnt].name)-1);
					strncpy(inFiles[inCnt].src, pstart, sizeof(inFiles[inCnt].src)-1);
					inFiles[inCnt].name[sizeof(inFiles[inCnt].name)-1] = '\0';
					inFiles[inCnt].src[sizeof(inFiles[inCnt].src)-1] = '\0';
					inCnt++;
					printf("√ 已收集输入文件 %s (待拷贝)\n", b);
				} else printf("x> .in 文件过多，无法收集：%s\n", b);
			}
		} else if(strlen(lowb) > 4 && strcmp(lowb + strlen(lowb) - 4, ".out") == 0) {
			// 收集 .out 文件，稍后统一拷贝到 out/，先按文件名去重
			{
				bool dup = false;
				for(int k=0;k<outCnt;k++) {
					if(strcmp(outFiles[k].name, b) == 0) { dup = true; break; }
				}
				if(dup) {
					printf("?> 已收集过输出文件 %s，忽略重复项\n", b);
				} else if(outCnt < (int)(sizeof(outFiles)/sizeof(outFiles[0]))) {
					strncpy(outFiles[outCnt].name, b, sizeof(outFiles[outCnt].name)-1);
					strncpy(outFiles[outCnt].src, pstart, sizeof(outFiles[outCnt].src)-1);
					outFiles[outCnt].name[sizeof(outFiles[outCnt].name)-1] = '\0';
					outFiles[outCnt].src[sizeof(outFiles[outCnt].src)-1] = '\0';
					outCnt++;
					printf("√ 已收集输出文件 %s (待拷贝)\n", b);
				} else printf("x> .out 文件过多，无法收集：%s\n", b);
			}
		} else {
			// 复制到原名，但不会计入必须三项
			snprintf(dst, sizeof(dst), "%s/%s", newDir, b);
			if(copy_file(pstart, dst)) {
				printf("√ 已拷贝附加文件 %s\n", b);
			} else printf("x> 拷贝失败：%s\n", pstart);
		}
		// 如果三项都已满足，提示并继续收集以便可能的 .in/.out
		if(gotAnalyzing && gotSolution && gotProblem) {
			printf("√> 已收集必需文件，后续可继续添加测试文件 (.in/.out) 或按空行完成。\n");
		}
	}
	printf("√> 题目 %s 添加完成。\n", id);
}
static void printProblemDetails(const ProblemEntry* e) {
	if(!e) return;
	puts(  "|----------------------------|");
	printf("|  ID: %s\n", e->id);
	printf("|  标题: %s\n", e->title);
	printf("|  难度: %s\n", e->difficulty);
	printf("|  类型: %s\n", e->type);
	printf("|  题目文件: %s\n", e->problemPath);
	puts(  "|----------------------------|");
}
void cleanScreen();
void pauseScreen() {
	printf("=> 按任意键继续...");
	getchar();
    getchar();
	// 等待用户按键
}
void interactiveProblemBank(const char* problemsDir) {
	while(true) {
		cleanScreen();
		puts("========= ACM 题库 =========");
		puts("|----------------------------|");
		puts("|      1. 显示所有题目       |");
		puts("|      2. 搜索题目           |");
		puts("|      3. 删除题目           |");
		puts("|      4. 添加题目           |");
		puts("|      0. 返回               |");
		puts("|----------------------------|");
		printf("=> 请输入选项：[ ]\b\b");
		int choice = 0;
		//CleanBuffer
		if(scanf("%d", &choice) != 1) {
			int c;
			while((c=getchar())!='\n' && c!=EOF);
			printf("?> 无效输入，请重试。\n");
			pauseScreen();
			continue;
		}
		if(choice == 0) return;
		if(choice == 3) {
			// 删除题目
			print_breadcrumb("删除题目");
			printf("=> 请输入要删除的题目 ID 或 文件夹名：");
			char idbuf[128];
			if(scanf("%s", idbuf) != 1) {
				int cc;
				while((cc=getchar())!='\n' && cc!=EOF);
				continue;
			}
            
			// 在删除前尝试加载并显示该题目的 MetaData (若找到)
			ProblemEntry entries[MAX_PROBLEMS];
			int cnt = loadAllProblems(problemsDir, entries, MAX_PROBLEMS);
			int found = -1;
			if(cnt > 0) {
				for (int i=0;i<cnt;i++) {
					if(strcmp(entries[i].id, idbuf) == 0 || strcmp(entries[i].folderName, idbuf) == 0) {
						found = i;
						break;
					}
				}
			}
			if(found != -1) {
				printf("将要删除如下题目：\n");
				printProblemDetails(&entries[found]);
				printf("确认删除以上题目吗？(y/n)：");
			} else {
				// 未找到题目，提示并返回菜单（不再询问确认删除）
				printf("x> 未找到题目：%s\n", idbuf);
				pauseScreen();
				continue;
			}
			char ans[8];
			if(scanf("%s", ans) != 1) continue;
			if(ans[0]=='y' || ans[0]=='Y') {
				if(deleteProblemByID(problemsDir, idbuf)) printf("√> %s 已删除。\n", idbuf); else printf("x> 删除失败，可能不存在或无法删除。\n");
			} else {
				printf("?> 已取消删除。\n");
			}
			pauseScreen();
			continue;
		}
		if(choice == 4) {
			// 添加题目
			print_breadcrumb("添加题目");
			addProblemInteractive(problemsDir);
			pauseScreen();
			continue;
		}
		if(choice == 1) {
			ProblemEntry entries[MAX_PROBLEMS];
			int cnt = loadAllProblems(problemsDir, entries, MAX_PROBLEMS);
			if(cnt == 0) {
				printf("?> 未找到题目。\n");
				pauseScreen();
				continue;
			}
			printf("-------- 题目列表 (%d) ---------\n", cnt);
			printf("难度\t\tID\t\t标题\n");
			for (int i=0;i<cnt;i++) {
				printf("| %s\t\t%s\t\t%s\n", entries[i].difficulty, entries[i].id, entries[i].title);
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
			for (int i=0;i<cnt;i++) 
			                if(strcmp(entries[i].id, idBuf) == 0 || strcmp(entries[i].folderName, idBuf) == 0) {
				found = i;
				break;
			}
			if(found == -1) {
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
			if(sub == 1) {
				// To-Do
				printf("提交功能暂未实现。\n");
			}
			pauseScreen();
			continue;
		} else if(choice == 2) {
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
			if(cnt == 0) {
				printf("?> 未找到题目。\n");
				pauseScreen();
				continue;
			}
			ProblemEntry results[MAX_PROBLEMS];
			int rcount = 0;
			for (int i=0;i<cnt;i++) {
				// 标题、难度、类型基于 metadata；题干需要读取文件
				bool ok = true;
				if(!contains_case_insensitive(entries[i].title, titleFilter)) ok = false;
				if(!contains_case_insensitive(entries[i].difficulty, diffFilter)) ok = false;
				if(!contains_case_insensitive(entries[i].type, typeFilter)) ok = false;
				if(ok && stmtFilter[0] != '\0') {
					char* stmt = readFileToString(entries[i].problemPath);
					if(stmt) {
						if(!contains_case_insensitive(stmt, stmtFilter)) ok = false;
						free(stmt);
					} else {
						ok = false;
					}
				}
				if(ok) {
					results[rcount++] = entries[i];
					if(rcount >= MAX_PROBLEMS) 
					                        break;
				}
			}
			if(rcount == 0) {
				printf("?> 未找到匹配题目。\n");
				pauseScreen();
				continue;
			}
			printf("√> 找到 %d 条题目：\n", rcount);
            printf("|      -------- 题目列表 (%d) ---------       |\n", rcount);
            printf("ID\t\t标题\t\t难度\n");
			for (int i=0;i<rcount;i++) 
			                printf("%s\t\t%s\t\t%s\n", results[i].id, results[i].title, results[i].difficulty);
			printf(".      -------------------------------       .\n");
            printf("=> 输入题目 ID 打开详情，或 0 返回：");
			char idBuf[128];
			if(scanf("%s", idBuf) != 1) 
			                continue;
			if(strcmp(idBuf, "0")==0) 
			                continue;
			int found = -1;
			for (int i=0;i<rcount;i++) 
			                if(strcmp(results[i].id, idBuf) == 0 || strcmp(results[i].folderName, idBuf) == 0) {
				found = i;
				break;
			}
			if(found == -1) {
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
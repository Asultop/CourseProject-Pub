#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
// 修复 Warning 
#include "problemBankManager.h"
#include "Def.h"
#include "fileHelper.h"
#include "markdownPrinter.h"
#include "ACMLocalJudger.h"
#include "screenManager.h"
#include "chineseSupport.h"
#include "codeRender.h"
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

void cleanScreen(void);
void cleanBuffer(void);
void pauseScreen(void);
// 去除字符串末尾的换行符
static void trim_newline(char* s) {
	if(!s) return;
	size_t i=strlen(s);
	if(i==0) return;
	if(s[i-1]=='\n') s[i-1]='\0';
	if(i>1 && s[i-2]=='\r') s[i-2]='\0';
}
// 去除字符串首尾空白（就地修改）
static void trim_space(char* s) {
	if(!s) return;
	// trim leading
	char* start = s;
	while(*start && isspace((unsigned char)*start)) start++;
	if(start != s) memmove(s, start, strlen(start)+1);
	// trim trailing
	size_t len = strlen(s);
	while(len > 0 && isspace((unsigned char)s[len-1])) { s[len-1] = '\0'; len--; }
}

// 比较两个字符串（不区分大小写），用于类型标签匹配
static bool equals_case_insensitive(const char* a, const char* b) {
	if(!a || !b) return false;
	while(*a && *b) {
		if(tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
		a++; b++;
	}
	return *a == '\0' && *b == '\0';
}

// 判断 entries 中的类型字段是否与 typeFilter（逗号分隔）匹配
// 如果 typeFilter 为空则返回 true；否则当 typeFilter 中任一标签等于 entries 的任一标签时返回 true
static bool type_matches_filter(const char* entryType, const char* typeFilter) {
	if(!typeFilter || typeFilter[0] == '\0') return true;
	if(!entryType || entryType[0] == '\0') return false;
	// 复制两个字符串到可修改缓冲
	char efbuf[256];
	char tfbuf[256];
	strncpy(efbuf, entryType, sizeof(efbuf)-1); efbuf[sizeof(efbuf)-1] = '\0';
	strncpy(tfbuf, typeFilter, sizeof(tfbuf)-1); tfbuf[sizeof(tfbuf)-1] = '\0';
	// 切分 entry 类型
	char* epart = efbuf;
	while(epart) {
		char* comma = strchr(epart, ',');
		if(comma) *comma = '\0';
		trim_space(epart);
		// 对于每个 filter token
		char* tpart = tfbuf;
		while(tpart) {
			char* tcomma = strchr(tpart, ',');
			if(tcomma) *tcomma = '\0';
			trim_space(tpart);
			if(equals_case_insensitive(epart, tpart)) return true;
			if(!tcomma) break;
			tpart = tcomma + 1;
		}
		if(!comma) break;
		epart = comma + 1;
	}
	return false;
}

// 将 entryType 中每个逗号分隔的子项按原样输出，若子项命中 typeFilter 中任一 token，则为该子项加高亮
static void highlight_type_to_str(const char* entryType, const char* typeFilter, char* dst, size_t dstsz) {
	if(!dst || dstsz == 0) return;
	dst[0] = '\0';
	if(!entryType) return;
	const char* COLOR = HIGHLIGHT_COLOR;
	const char* RESET = ANSI_FRMT_RESET;
	char buf[256];
	strncpy(buf, entryType, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
	char* part = buf;
	bool first = true;
	size_t used = 0;
	while(part) {
		char* comma = strchr(part, ',');
		if(comma) *comma = '\0';
		trim_space(part);
		if(!first) {
			if(used + 2 < dstsz) { dst[used++] = ','; dst[used++] = ' '; dst[used] = '\0'; }
		}
		// 检查是否匹配任一 filter token
		bool matched = type_matches_filter(part, typeFilter);
		if(matched) {
			size_t rlen = strlen(COLOR);
			size_t plen = strlen(part);
			size_t llen = strlen(RESET);
			if(used + rlen + plen + llen + 1 < dstsz) {
				memcpy(dst + used, COLOR, rlen); used += rlen;
				memcpy(dst + used, part, plen); used += plen;
				memcpy(dst + used, RESET, llen); used += llen;
				dst[used] = '\0';
			}
		} else {
			size_t plen = strlen(part);
			if(used + plen + 1 < dstsz) {
				memcpy(dst + used, part, plen); used += plen; dst[used] = '\0';
			}
		}
		first = false;
		if(!comma) break;
		part = comma + 1;
	}
}
// 读取小文件到动态分配字符串
static char* readFileToString(const char* path) {
	return readFileToStr(path);
}
static void printFileWithLatex(const char* src){
	mdcat_worker(src);
}
// 打印面包屑头部
static void print_breadcrumb(const char* sub) {
	cleanScreen();
	printHeader();
	if(sub && sub[0]) {
		char buf[128];
		snprintf(buf, sizeof(buf), "ACM 题库 - %s", sub);
		printCenter(buf);
	} else {
		printCenter("ACM 题库");
	}
	printFooter();
}
// 不区分大小写包含匹配
static bool contains_case_insensitive(const char* text, const char* pat) {
	if(!pat || pat[0]=='\0') return true;
	if(!text) return false;
	char *lowText = NULL;
	char *lowPat = NULL;
	lowText = (char*)malloc(strlen(text) + 1);
	lowPat = (char*)malloc(strlen(pat) + 1);
	if(!lowText || !lowPat) {
		free(lowText);
		free(lowPat);
		return false;
	}
	strcpy(lowText, text);
	strcpy(lowPat, pat);
	for (char* p = lowText; *p; ++p) *p = (char)tolower((unsigned char)*p);
	for (char* p = lowPat; *p; ++p) *p = (char)tolower((unsigned char)*p);
	bool res = strstr(lowText, lowPat) != NULL;
	free(lowText);
	free(lowPat);
	return res;
}

// 在输出中将匹配的子串以红色高亮（不改变原始大小写）
static void print_highlight(const char* value, const char* filter) {
	const char* COLOR = HIGHLIGHT_COLOR;
	const char* RESET = ANSI_FRMT_RESET;
	if(!value) value = "";
	if(!filter || filter[0] == '\0') {
		// printf("%s", value);
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s", value);
		printLeft(buf);
		return;
	}
	char lowVal[1024];
	char lowFilter[256];
	size_t vlen = strlen(value);
	size_t flen = strlen(filter);
	if(vlen >= sizeof(lowVal)) vlen = sizeof(lowVal)-1;
	if(flen >= sizeof(lowFilter)) flen = sizeof(lowFilter)-1;
	for(size_t i=0;i<vlen;i++) lowVal[i] = (char)tolower((unsigned char)value[i]);
	lowVal[vlen] = '\0';
	for(size_t i=0;i<flen;i++) lowFilter[i] = (char)tolower((unsigned char)filter[i]);
	lowFilter[flen] = '\0';

	const char* cur = value;
	const char* lowCur = lowVal;
	while(true) {
		char* found = strstr(lowCur, lowFilter);
		char buffer[1280];
		if(!found) {
			// printf("%s", cur);
			snprintf(buffer, sizeof(buffer), "%s", cur);
			printLeft(buffer);
			break;
		}
		size_t prefixLen = (size_t)(found - lowCur);
		if(prefixLen > 0) {
			// fwrite(cur, 1, prefixLen, stdout);
			snprintf(buffer, sizeof(buffer), "%.*s", (int)prefixLen, cur);
		}
		// printf("%s", COLOR);
		// fwrite(cur + prefixLen, 1, flen, stdout);
		// printf("%s", RESET);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%s%.*s%s", COLOR, (int)flen, cur + prefixLen, RESET);
		cur += prefixLen + flen;
		lowCur += prefixLen + flen;
		if(*cur == '\0') break;
		printLeft(buffer);
	}
}

// 将高亮文本写入目标字符串（与 print_highlight 行为一致，但不直接写入 stdout）
static void highlight_to_str(const char* value, const char* filter, char* dst, size_t dstsz) {
	const char* COLOR = HIGHLIGHT_COLOR;
	const char* RESET = ANSI_FRMT_RESET;
	if(!dst || dstsz == 0) return;
	dst[0] = '\0';
	if(!value) value = "";
	if(!filter || filter[0] == '\0') {
		strncpy(dst, value, dstsz-1);
		dst[dstsz-1] = '\0';
		return;
	}
	char lowVal[1024];
	char lowFilter[256];
	size_t vlen = strlen(value);
	size_t flen = strlen(filter);
	if(vlen >= sizeof(lowVal)) vlen = sizeof(lowVal)-1;
	if(flen >= sizeof(lowFilter)) flen = sizeof(lowFilter)-1;
	for(size_t i=0;i<vlen;i++) lowVal[i] = (char)tolower((unsigned char)value[i]);
	lowVal[vlen] = '\0';
	for(size_t i=0;i<flen;i++) lowFilter[i] = (char)tolower((unsigned char)filter[i]);
	lowFilter[flen] = '\0';

	const char* cur = value;
	const char* lowCur = lowVal;
	size_t used = 0;
	while(true) {
		char* found = strstr(lowCur, lowFilter);
		if(!found) {
			size_t tocopy = strlen(cur);
			if(used + tocopy + 1 > dstsz) tocopy = (dstsz > used) ? dstsz - used - 1 : 0;
			if(tocopy > 0) memcpy(dst + used, cur, tocopy), used += tocopy;
			break;
		}
		size_t prefixLen = (size_t)(found - lowCur);
		if(prefixLen > 0) {
			size_t tocopy = prefixLen;
			if(used + tocopy + 1 > dstsz) tocopy = (dstsz > used) ? dstsz - used - 1 : 0;
			if(tocopy > 0) memcpy(dst + used, cur, tocopy), used += tocopy;
		}
		// 添加 COLOR
		size_t rlen = strlen(COLOR);
		if(used + rlen + 1 <= dstsz) memcpy(dst + used, COLOR, rlen), used += rlen;
		// 添加匹配的片段
		size_t mlen = flen;
		if(used + mlen + 1 <= dstsz) memcpy(dst + used, cur + prefixLen, mlen), used += mlen;
		// 添加 RESET
		size_t relen = strlen(RESET);
		if(used + relen + 1 <= dstsz) memcpy(dst + used, RESET, relen), used += relen;

		cur += prefixLen + flen;
		lowCur += prefixLen + flen;
		if(*cur == '\0') break;
	}
	dst[used] = '\0';
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
		FILE* mf = openFile(metaPath, "r");
		if(!mf) continue;
		char line[1024];
		if(fgets(line, sizeof(line), mf) == NULL) {
			closeFile(mf);
			continue;
		}
		closeFile(mf);
		trim_newline(line);
		// id|题干|难度|类型
		char* tmp = (char*)malloc(strlen(line) + 1);
		if(tmp) strcpy(tmp, line);
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
		{
			// 构造 problem.txt 路径
			const char *suffix = "/problem.txt";
			size_t max_prefix = sizeof(e.problemPath) - strlen(suffix) - 1;
			snprintf(e.problemPath, sizeof(e.problemPath), "%.*s%s", (int)max_prefix, subdir, suffix);
		}
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
	FILE* fsrc = openFile(src, "rb");
	if(!fsrc) return false;
	FILE* fdst = openFile(dst, "wb");
	if(!fdst) {
		closeFile(fsrc);
		return false;
	}
	char buf[4096];
	size_t n;
	while((n = fread(buf,1,sizeof(buf),fsrc))>0) {
		fwrite(buf,1,n,fdst);
	}
	closeFile(fsrc);
	closeFile(fdst);
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
	// 期望的解文件名：{ID}.c（小写比较）
	char expectedSolLower[256];
	snprintf(expectedSolLower, sizeof(expectedSolLower), "%s.c", id);
	for (char* p = expectedSolLower; *p; ++p) *p = (char)tolower((unsigned char)*p);
	// 创建目录
	if(mkdir(newDir, 0755) != 0) {
		printf("x> 无法创建目录：%s (errno=%d)\n", newDir, errno);
		return;
	}
	// 写 MetaData
	char metaPath[1200];
	snprintf(metaPath, sizeof(metaPath), "%s/MetaData", newDir);
	FILE* mf = openFile(metaPath, "w");
	if(!mf) {
		printf("x> 无法写入 MetaData\n");
		return;
	}
	fprintf(mf, "%s|%s|%s|%s\n", id, title, diff, type);
	closeFile(mf);
	printf("=> 请逐行输入题目文件的路径（需要三个文件：analyzing.txt, %s.c, problem.txt）。\n", id);
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
			// 移除两侧引号
			if(L>0 && (pathbuf[L-1] == '"' || pathbuf[L-1] == '\'')) {
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
				size_t need_len_in = strlen(indir) + 1 + strlen(inFiles[i].name) + 1; /* dir + '/' + name + NUL */
				char *dst = malloc(need_len_in);
				if(!dst) {
					printf("x> 内存不足，无法拷贝输入文件：%s\n", inFiles[i].name);
					continue;
				}
				snprintf(dst, need_len_in, "%s/%s", indir, inFiles[i].name);
				if(copy_file(inFiles[i].src, dst))
					printf("√> 已拷贝输入文件 %s 到 in/\n", inFiles[i].name);
				else
					printf("x> 拷贝失败：%s\n", inFiles[i].src);
				free(dst);
			}
			for(int i=0;i<outCnt;i++) {
				size_t need_len_out = strlen(outdir) + 1 + strlen(outFiles[i].name) + 1; /* dir + '/' + name + NUL */
				char *dst = malloc(need_len_out);
				if(!dst) {
					printf("x> 内存不足，无法拷贝输出文件：%s\n", outFiles[i].name);
					continue;
				}
				snprintf(dst, need_len_out, "%s/%s", outdir, outFiles[i].name);
				if(copy_file(outFiles[i].src, dst))
					printf("√> 已拷贝输出文件 %s 到 out/\n", outFiles[i].name);
				else
					printf("x> 拷贝失败：%s\n", outFiles[i].src);
				free(dst);
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
		} else if(strcmp(lowb, expectedSolLower) == 0) {
			snprintf(dst, sizeof(dst), "%s/%s.c", newDir, id);
			if(copy_file(pstart, dst)) {
				gotSolution = true;
				printf("√ 已拷贝题解至 %s.c\n", id);
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
			/* 复制到原名，但不会计入必须三项 */
			size_t need_len = strlen(newDir) + 1 + strlen(b) + 1;
			char *dst2 = malloc(need_len);
			if(!dst2) {
				printf("x> 内存不足，无法拷贝 %s\n", b);
			} else {
				snprintf(dst2, need_len, "%s/%s", newDir, b);
				if(copy_file(pstart, dst2)) printf("√ 已拷贝附加文件 %s\n", b);
				else printf("x> 拷贝失败：%s\n", pstart);
				free(dst2);
			}
		}
		// 如果三项都已满足，提示并继续收集以便可能的 .in/.out
		if(gotAnalyzing && gotSolution && gotProblem) {
			printf("√> 已收集必需文件，后续可继续添加测试文件 (.in/.out) 或按空行完成。\n");
		}
	}
	printf("√> 题目 %s 添加完成。\n", id);
}
// 打印题目详细信息及内容
static void printProblemDetails(const ProblemEntry* e) {
	if(!e) return;
	// puts(  "|----------------------------|");
	// printf("|  ID: %s\n", e->id);
	// printf("|  标题: %s\n", e->title);
	// printf("|  难度: %s\n", e->difficulty);
	// printf("|  类型: %s\n", e->type);
	// printf("|  题目文件: %s\n", e->problemPath);
	// puts(  "|----------------------------|");
	printHeader();
	printCenter("题目详情");
	printDivider();
	char IDline[128];
	snprintf(IDline, sizeof(IDline), "ID: %s", e->id);
	printLeft(IDline);
	char titleline[512];
	snprintf(titleline, sizeof(titleline), "标题: %s", e->title);
	printLeft(titleline);
	char difline[128];
	snprintf(difline, sizeof(difline), "难度: %s", e->difficulty);
	printLeft(difline);
	char typeline[256];
	snprintf(typeline, sizeof(typeline), "类型: %s", e->type);
	printLeft(typeline);
	printDivider();
	// 读取题目内容文件
    if(fileExists(e->problemPath)) {
        bool contentExist = fileExists(e->problemPath);
        if(contentExist) {
            printCenter("题目开始");
			printDivider();
			// 读取并打印题目内容（支持 LaTeX 渲染）
			printFileWithLatex(e->problemPath);
			printDivider();
			printCenter("题目结束");
			printDivider();
        } else {
            printf("x> 无法读取题目内容文件：%s\n", e->problemPath);
        }
    } else {
        printf("x> 题目内容文件不存在：%s\n", e->problemPath);
    }
}
// 题目详情子菜单：查看题目/解析/题解/运行样例等（循环直到返回）
static void problemDetailMenu(const char* problemsDir, const ProblemEntry* e) {
	if(!e) return;
	int sub = -1;
	do {
		sub = -1;
		printProblemDetails(e);
		// printf("\n1. 提交结果\n2. 查看解析\n3. 查看题解\n4. 运行正确样例\n0. 返回\n=> 请选择：");
		printLeft("1. 提交结果");
		printLeft("2. 查看解析");
		printLeft("3. 查看题解");
		printLeft("4. 运行正确样例");
		printDivider();
		printCenter("0. 返回");
		printFooter();
		printf("=> 请选择：");
		if(scanf("%d", &sub) != 1) {
			// 清理输入缓存
			cleanBuffer();
			printf("x> 无效的选择\n");
			sleep(1);
			continue;	
		}
		if(sub == 1) {
			/* 提交结果：提示用户输入源文件路径（支持带引号），并使用当前题目的 ProblemEntry 进行本地判题 */
			printf("=> 请输入要提交的源文件路径（支持带引号）：");
			{
				char pathbuf[1200];
				int c;
				/* 清除 scanf 后残留的换行 */
				while((c = getchar()) != '\n' && c != EOF);
				if(!fgets(pathbuf, sizeof(pathbuf), stdin)) {
					printf("x> 读取输入失败\n");
					goto continueWithWait;
				}
				trim_newline(pathbuf);
					/* 去掉首尾引号 */
				char *pstart = pathbuf;
				while(*pstart == '"' || *pstart == '\'') pstart++;
				char *pend = pstart + strlen(pstart) - 1;
				while(pend > pstart && (*pend == '"' || *pend == '\'')) { *pend = '\0'; pend--; }
				if(!fileExists(pstart)) {
					printf("x> 源文件不存在：%s\n", pstart);
					// pauseScreen();
					puts("=> 按 Enter 键继续..."); // 特殊处理，cleanBuffer 不适用
					fflush(stdout);
					cleanBuffer();
					continue;
				}
				/* 计算题目目录 */
				char probcopy[1024];
				strncpy(probcopy, e->problemPath, sizeof(probcopy)-1);
				probcopy[sizeof(probcopy)-1] = '\0';
				char *pdir = dirname(probcopy);
				/* 调用本地判题器 */
				JudgeSummary js = acm_local_judge(pstart, e);
				printHeader();
				printCenter("判题结果");
				if (js.count == 0) {
					// printf("=> 未发现 in/ 测试用例 (目录: %s/in)\n", pdir);
					char buf[512];
					snprintf(buf, sizeof(buf), "=> 未发现 in/ 测试用例 (目录: %s/in)", pdir);
					printLeft(buf);
					printFooter();
					goto continueWithWait;
				}
				printDivider();
				for (int i = 0; i < js.count; ++i) {
					JudgeReturnInfo *ri = &js.infos[i];
					switch (ri->result) {
						case JUDGE_RESULT_ACCEPTED:
							// printf(ANSI_BOLD_GREEN "[AC]" ANSI_FRMT_RESET " 测试点 %d: %s\n", i+1, ri->message);
							{
								char buf[512];
								snprintf(buf, sizeof(buf), ANSI_BOLD_GREEN "[AC]" ANSI_FRMT_RESET " 测试点 %d: %s", i+1, ri->message);
								printLeft(buf);
							}
							break;
						case JUDGE_RESULT_WRONG_ANSWER:
							// printf(ANSI_BOLD_COLOR "[WA]" ANSI_FRMT_RESET " 测试点 %d: %s\n", i+1, ri->message);
							{
								char buf[512];
								snprintf(buf, sizeof(buf), ANSI_BOLD_RED "[WA]" ANSI_FRMT_RESET " 测试点 %d: %s", i+1, ri->message);
								printLeft(buf);
							}
							break;
						case JUDGE_RESULT_RUNTIME_ERROR:
							// printf(ANSI_BOLD_MAGENTA "[RE]" ANSI_FRMT_RESET " 测试点 %d: %s\n", i+1, ri->message);
							{
								char buf[512];
								snprintf(buf, sizeof(buf), ANSI_BOLD_MAGENTA "[RE]" ANSI_FRMT_RESET " 测试点 %d: %s", i+1, ri->message);
								printLeft(buf);
							}	
							break;
						case JUDGE_RESULT_COMPILE_ERROR:
							// printf(ANSI_BOLD_YELLOW "[CE]" ANSI_FRMT_RESET " 测试点 %d: %s\n", i+1, ri->message);
							{
								char buf[512];
								snprintf(buf, sizeof(buf), ANSI_BOLD_YELLOW "[CE]" ANSI_FRMT_RESET " 测试点 %d: %s", i+1, ri->message);
								printLeft(buf);
							}
							break;
						case JUDGE_RESULT_TIME_LIMIT_EXCEEDED:
							// printf(ANSI_BOLD_WHITE "[TLE]" ANSI_FRMT_RESET " 测试点 %d: %s\n", i+1, ri->message);
							{
								char buf[512];
								snprintf(buf, sizeof(buf), ANSI_BOLD_WHITE "[TLE]" ANSI_FRMT_RESET " 测试点 %d: %s", i+1, ri->message);
								printLeft(buf);
							}
							break;
						default:
							// printf("[?] 测试点 %d: %s\n", i+1, ri->message);
							{
								char buf[512];
								snprintf(buf, sizeof(buf), "[?] 测试点 %d: %s", i+1, ri->message);
								printLeft(buf);
							}
							break;
					}
					if(i < js.count - 1) printDivider();
				}
				printFooter();
				puts("=> 按 Enter 键继续...");
				fflush(stdout);
				cleanBuffer();
				continue;
			}
		} else if(sub == 2) {
			char path[1200];
			snprintf(path, sizeof(path), "%s/%s/analyzing.txt", problemsDir, e->folderName);
			if(fileExists(path)) {
				bool txt = fileExists(path);
				if(txt) { 
					printHeader();
					printCenter("解析文件");
					printDivider();
					printFileWithLatex(path);
					printDivider();
					printCenter("解析结束"); 
					printFooter();
				}
				else printf("x> 无法读取解析文件：%s\n", path);
			} else {
				printf("x> 解析文件不存在：%s\n", path);
			}
		} else if(sub == 3) {
			char path[1200];
			snprintf(path, sizeof(path), "%s/%s/%s.c", problemsDir, e->folderName, e->id);
			if(!fileExists(path)) snprintf(path, sizeof(path), "%s/%s/general_solution.c", problemsDir, e->folderName);
			if(fileExists(path)) {
				bool srcExist = fileExists(path);
				if(srcExist) { 
					printHeader();
					printCenter("题解文件");
					printDivider();
					codeRender_worker(path);
					puts("");
					printDivider();
					printCenter("题解结束");
					printFooter();
				}
				else printf("x> 无法读取题解文件：%s\n", path);
			} else {
				printf("x> 题解文件不存在：%s 或 %s.c\n", e->folderName, e->id);
			}
		} else if(sub == 4) {
			// 运行正确样例：编译 {id}.c 到临时目录并执行，优先使用 in/ 下第一个 .in 作为输入
			char src[1200];
			snprintf(src, sizeof(src), "%s/%s/%s.c", problemsDir, e->folderName, e->id);
			if(!fileExists(src)) snprintf(src, sizeof(src), "%s/%s/general_solution.c", problemsDir, e->folderName);
			if(!fileExists(src)) {
				printf("x> 找不到可运行的源文件：%s.c 或 general_solution.c\n", e->id);
				goto continueWithWait;
			}
			char tmpTemplate[] = "/tmp/progrunXXXXXX";
			char* tmpdir = mkdtemp(tmpTemplate);
			if(!tmpdir) {
				printf("x> 无法创建临时目录\n");
				goto continueWithWait;
			}
			size_t exePath_len = strlen(tmpdir) + 1 + strlen(e->id) + strlen("_exec") + 1;
			char *exePath = malloc(exePath_len);
			if(!exePath) {
				printf("x> 内存不足，无法准备可执行文件路径\n");
				rmdir(tmpdir);
				goto continueWithWait;
			}
			snprintf(exePath, exePath_len, "%s/%s_exec", tmpdir, e->id);
			size_t compile_need = strlen("g++ -std=c++17 -O2 -o '' '' 2>&1") + strlen(exePath) + strlen(src) + 1;
			char *compileCmd = malloc(compile_need);
			if(!compileCmd) {
				printf("x> 内存不足，无法构造编译命令\n");
				free(exePath);
				rmdir(tmpdir);
				goto continueWithWait;
			}
			snprintf(compileCmd, compile_need, "gcc -std=c11 -O2 -o '%s' '%s' 2>&1", exePath, src);
			printf("=> 编译: %s\n", compileCmd);
			int cret = system(compileCmd);
			free(compileCmd);
			if(cret != 0) {
				printf("x> 编译失败 (返回 %d)\n", cret);
			} else {
				/* 直接执行可执行文件（不自动查找或重定向 .in） */
				char header[128];
				snprintf(header, sizeof(header), " %s 正确样例运行开始 ", e->id);
				printDivider();
				printCenter(header);
				printDivider();
				fflush(stdout);
				size_t run_need = 2 + strlen(exePath) + 1;
				char *runCmd = malloc(run_need);
				if(runCmd) {
					snprintf(runCmd, run_need, "'%s'", exePath);
					int rret = system(runCmd); (void)rret;
					free(runCmd);
				} else {
					/* 退化：直接运行 exePath（不加引号） */
					int rret = system(exePath); (void)rret;
				}
				snprintf(header, sizeof(header), " %s 正确样例运行结束 ", e->id);
				printDivider();
				printCenter(header);
				printFooter();
				remove(exePath);
			}
			free(exePath);
			rmdir(tmpdir);
		}
		else {
			puts("?> 无效选项，请重试。");
		}

		continueWithWait: {
			if(sub != 0 ) {
				pauseScreen();
			}
			if(sub == -1){
				puts("?> 输入格式有误，请重试。");
				pauseScreen();
			}
		}
	} while(sub != 0);
}
void cleanScreen(void);

void pauseScreen(void);
// 交互式题库管理主界面
int compareProblemEntries(const void* a, const void* b) {
	const ProblemEntry* pa = (const ProblemEntry*)a;
	const ProblemEntry* pb = (const ProblemEntry*)b;
	return strcmp(pa->id, pb->id);
}
void interactiveProblemBank(const char* problemsDir, UsrProfile * currentUser) {
	while(true) {
		cleanScreen();
		printACMProblemBankScreen(currentUser->name);
		int choice = 0;
		//CleanBuffer
		if(scanf("%d", &choice) != 1) {
			int c;
			while((c=getchar())!='\n' && c!=EOF);
			printf("?> 无效输入，请重试。\n");
			sleep(1);
			continue;
		}
		if(choice == 0) return;
		if(choice == 3) {
			// 删除题目
			print_breadcrumb("删除题目");
			printf("=> 请输入要删除的题目 ID 或 文件夹名：");
			char idbuf[128];
			if(scanf("%s", idbuf) != 1) {
				cleanBuffer();
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
				
				printHeader();
				printCenter("确认删除题目");
				printDivider();
				printCenter("将要删除如下题目: ");
				{
					char IDline[128];
					snprintf(IDline, sizeof(IDline), "ID: %s", entries[found].id);
					printLeft(IDline);
					char titleline[512];
					snprintf(titleline, sizeof(titleline), "标题: %s", entries[found].title);
					printLeft(titleline);
					char difline[128];
					snprintf(difline, sizeof(difline), "难度: %s", entries[found].difficulty);
					printLeft(difline);
					char typeline[256];
					snprintf(typeline, sizeof(typeline), "类型: %s", entries[found].type);
					printLeft(typeline);
					char probline[512];
					// snprintf(probline, sizeof(probline), "题目文件: %s", entries[found].problemPath);
					{
						char buf[1024];
						size_t w = snprintf(buf, sizeof(buf), "题目文件: ");
						if (w < 0) w = 0;
						if ((size_t)w < sizeof(buf)) {
							size_t rem = sizeof(buf) - (size_t)w - 1;
							strncat(buf, entries[found].problemPath, rem);
							strncpy(probline, buf, sizeof(probline));
							probline[sizeof(probline) - 1] = '\0';
						} else {
							strncpy(probline, "题目文件: [路径过长，无法显示完整]", sizeof(probline));
							probline[sizeof(probline) - 1] = '\0';
						}
					}
					printLeft(probline);
				}
				printFooter();
				// printf("将要删除如下题目：\n");
                // puts(  "|----------------------------|");
                // printf("|  ID: %s\n", entries[found].id);
                // printf("|  标题: %s\n", entries[found].title);
                // printf("|  难度: %s\n", entries[found].difficulty);
                // printf("|  类型: %s\n", entries[found].type);
                // printf("|  题目文件: %s\n", entries[found].problemPath);
                // puts(  "|----------------------------|");
				printf("?> 确认删除以上题目吗？(y/n)：");
			} else {
				// 未找到题目，提示并返回菜单（不再询问确认删除）
				printf("x> 未找到题目：%s\n", idbuf);
				pauseScreen();
				continue;
			}
			char ans[8];
			if(scanf("%s", ans) != 1) continue;
			if(ans[0]=='y' || ans[0]=='Y') {
				if(deleteProblemByID(problemsDir, idbuf)) printf("√> %s 已删除。\n", idbuf); 
				else printf("x> 删除失败，可能不存在或无法删除。\n");
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
			printf("=> 按 Enter 键返回主菜单...");  // 特殊处理， cleanBuffer 在这里不适用
			fflush(stdout);
			cleanBuffer();
			
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
			// printf("-------- 题目列表 (%d) ---------\n", cnt);
			// printf("难度\t\tID\t\t标题\n");
			printHeader();
			printCenter("题目列表");
			printDivider();
			{
				char header[128];
				snprintf(header, sizeof(header), "%-12s %-8s %-25s %s", "难度", "ID", "标题", "类型");
				printLeft(header);
			}
			qsort(entries, (size_t)cnt, sizeof(ProblemEntry), compareProblemEntries);
			for (int i=0;i<cnt;i++) {
				/* 构造足够大的动态缓冲区以包含完整标题，避免截断导致看起来超出边框 */
				size_t need = strlen(entries[i].difficulty) + 1 + strlen(entries[i].id) + 1 + strlen(entries[i].title) + 1 + strlen(entries[i].type) + 64;
				char *line = (char*)malloc(need);
				if(line) {
					snprintf(line, need, "%-12s %-8s %-25s %s", entries[i].difficulty, entries[i].id, entries[i].title, entries[i].type);
					printLeft(line);
					free(line);
				} else {
					/* 内存分配失败则退回到安全的本地缓冲区拼接（不会丢弃太多） */
					char buf[1024];
					int w = snprintf(buf, sizeof(buf), "%-12s %-8s %-25s ", entries[i].difficulty, entries[i].id, "");
					if (w < 0) w = 0;
					if ((size_t)w < sizeof(buf)) {
						size_t rem = sizeof(buf) - (size_t)w - 1;
						strncat(buf, entries[i].title, rem);
					}
					// 添加类型可能被截断
					printLeft(buf);
				}
			}
			printFooter();
			// 交互式循环：在“显示所有题目”列表内处理错误输入并重新提示
			{
				int c;
				while ((c = getchar()) != '\n' && c != EOF) ; // 清理残余输入
				char idBuf[128];
				while (1) {
					printf("=> 输入题目 ID 打开详情，或 0 返回：");
					if(!fgets(idBuf, sizeof(idBuf), stdin)) break;
					trim_newline(idBuf);
					if(idBuf[0] == '\0') {
						printf("?> 无效输入。");
						cleanLine();
						moveUp(1);
						cleanLine();
						continue; // 重新提示，保持在题目列表
					}
					if(strcmp(idBuf, "0") == 0) break; // 返回上一级
					int found = -1;
					for (int j = 0; j < cnt; j++) {
						if(strcmp(entries[j].id, idBuf) == 0 || strcmp(entries[j].folderName, idBuf) == 0) {
							found = j;
							break;
						}
					}
					if(found == -1) {
						printf("?> 未找到 ID: %s", idBuf);
						sleep(1);
						cleanLine();
						moveUp(1);
						cleanLine();
						continue; // 保持在题目列表，重新提示
					}
					problemDetailMenu(problemsDir, &entries[found]);
					break; // 打开详情后返回上一级
				}
			}
			continue;
		} else if(choice == 2) {
			// 交互式筛选：标题，题干，难度，类型
			char buf[256];
			// CleanBuffer
			int c;
			while((c=getchar())!='\n' && c!=EOF);
			printf("=> 输入标题关键字（留空通配）：");
			fgets(buf, sizeof(buf), stdin);
			trim_newline(buf);
			char titleFilter[256];
			strncpy(titleFilter, buf, sizeof(titleFilter)-1);
			printf("=> 输入题干关键字（留空通配）：");
			fgets(buf, sizeof(buf), stdin);
			trim_newline(buf);
			char stmtFilter[256];
			strncpy(stmtFilter, buf, sizeof(stmtFilter)-1);
			printf("=> 输入难度（留空通配）：");
			fgets(buf, sizeof(buf), stdin);
			trim_newline(buf);
			char diffFilter[64];
			strncpy(diffFilter, buf, sizeof(diffFilter)-1);
			printf("=> 输入类型（留空通配）：");
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
				if(!type_matches_filter(entries[i].type, typeFilter)) ok = false;
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
				puts("=> 按 Enter 键继续..."); //  特殊处理， cleanBuffer 在这里不适用
				fflush(stdout); 
				cleanBuffer();
				
				// pauseScreen();

				continue;
			}
			printf("√> 找到 %d 条题目：\n", rcount);
			printHeader();
			{
				char buf[64];
				snprintf(buf, sizeof(buf), "搜索结果 (%d)", rcount);
				printCenter(buf);
			}
			
			printDivider();
			char header[128];
			snprintf(header, sizeof(header), "%-12s %-8s %-25s %s", "难度", "ID", "标题", "类型");
			printLeft(header);
			for (int i=0;i<rcount;i++) {
				// 将高亮标题写入缓冲并构造整行，保证标题列使用固定可见宽度以对齐难度列
				const char* title = results[i].title ? results[i].title : "";
				const char* diff = results[i].difficulty ? results[i].difficulty : "";
				const char* type = results[i].type ? results[i].type : "";
				const int title_col_width = 25; // 与表头宽度保持一致
				const int type_col_width = 12;
				size_t need = strlen(diff) * 4 + strlen(results[i].id) + strlen(title) * 4 + strlen(type) * 4 + 512;
				char *line = (char*)malloc(need);
				if(!line) {
					// 回退到简单打印（无对齐）
					// 难度
					char dbuf_fallback[256];
					highlight_to_str(diff, diffFilter, dbuf_fallback, sizeof(dbuf_fallback));
					printf("%-12s ", dbuf_fallback);
					// ID
					printf("%-8s ", results[i].id);
					// 标题
					print_highlight(title, titleFilter);
					int vlen = (int)get_real_Length(title, NULL);
					if(vlen < title_col_width) {
						for(int k=0;k < title_col_width - vlen; ++k) putchar(' ');
					}
					putchar(' ');
					// 类型
					char tbuf_fallback[256];
					highlight_type_to_str(type, typeFilter, tbuf_fallback, sizeof(tbuf_fallback));
					printf("%s\n", tbuf_fallback);
				} else {
					// 先写难度（高亮）
					size_t rem = 0;
					char *dbuf = malloc(256);
					if(dbuf) {
						highlight_to_str(diff, diffFilter, dbuf, 256);
						size_t dlen = strlen(dbuf);
						if(rem + dlen + 1 < need) memcpy(line + rem, dbuf, dlen), rem += dlen, line[rem] = '\0';
						free(dbuf);
					}
					// 补充空格到难度列宽
					int visible_diff_len = (int)get_real_Length(diff, NULL);
					int dpad = 0;
					if(visible_diff_len < 12) dpad = 12 - visible_diff_len;
					if(rem + dpad + 1 < need) { memset(line + rem, ' ', dpad + 1); rem += dpad + 1; line[rem] = '\0'; }
					// 添加 ID
					int off = snprintf(line + rem, need - rem, "%-8s ", results[i].id);
					if(off < 0) off = 0;
					rem += (size_t)off;
					// 高亮 title 到临时缓冲
					size_t tbufsz = strlen(title) * 4 + 64;
					if(tbufsz < 256) tbufsz = 256;
					char *tbuf = malloc(tbufsz);
					if(tbuf) {
						highlight_to_str(title, titleFilter, tbuf, tbufsz);
						size_t copy = strlen(tbuf);
						if(rem + copy + 1 < need) memcpy(line + rem, tbuf, copy), rem += copy, line[rem] = '\0';
						free(tbuf);
					}
					// 根据原始（可见）标题长度填充空格以对齐列
					int visible_len = (int)get_real_Length(title, NULL);
					int pad = 0;
					if(visible_len < title_col_width) pad = title_col_width - visible_len;
					if(rem + pad + 1 < need) { memset(line + rem, ' ', pad + 1); rem += pad + 1; line[rem] = '\0'; }
					// 添加类型（专用高亮）
					size_t typebufsz = strlen(type) * 4 + 128;
					if(typebufsz < 128) typebufsz = 128;
					char *typebuf = malloc(typebufsz);
					if(typebuf) {
						highlight_type_to_str(type, typeFilter, typebuf, typebufsz);
						size_t tcopy = strlen(typebuf);
						if(rem + tcopy + 1 < need) memcpy(line + rem, typebuf, tcopy), rem += tcopy, line[rem] = '\0';
						free(typebuf);
					}
					printLeft(line);
					free(line);
				}
			}
			printFooter();
			// 交互式循环：输入错误或未找到 ID 时返回到此搜索结果列表
			{
				// int c;
				// while ((c = getchar()) != '\n' && c != EOF) ; // 清理残余输入
				// 我c了， 这个 CLeanBuffer 怎么这么坏啊
				char idBuf[128];
				while (1) {
					printf("=> 输入题目 ID 打开详情，或 0 返回：");
					if(!fgets(idBuf, sizeof(idBuf), stdin)) break;
					trim_newline(idBuf);
					if(idBuf[0] == '\0') {
						printf("?> 无效输入。\n");
						pauseScreen();
						continue; // 重新提示，保持在搜索结果
					}
					if(strcmp(idBuf, "0") == 0) break; // 返回到上级（搜索列表外）
					int found = -1;
					for (int j = 0; j < rcount; j++) {
						if(strcmp(results[j].id, idBuf) == 0 || strcmp(results[j].folderName, idBuf) == 0) {
							found = j;
							break;
						}
					}
					if(found == -1) {
						printf("?> 未找到 ID: %s", idBuf);
						sleep(1);
						cleanLine();
						// printf("\033[1A"); // 光标上移一行
						moveUp(1);
						cleanLine();
						// pauseScreen();
						continue; // 保持在搜索结果，重新提示
					}
					problemDetailMenu(problemsDir, &results[found]);
					break; // 打开详情后返回到上一级
				}
			}
			continue;
		} else {
			printf("?> 无效选项。\n");
			pauseScreen();
			continue;
		}
	}
}
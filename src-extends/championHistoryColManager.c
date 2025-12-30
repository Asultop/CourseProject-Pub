#include "championHistoryColManager.h"
#include "screenManager.h"
#include "markdownPrinter.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim_newline(char* s){
    if(!s) return;
    size_t i = strlen(s);
    if(i==0) return;
    if(s[i-1]=='\n') s[i-1]='\0';
    if(i>1 && s[i-2]=='\r') s[i-2]='\0';
}

// 小写化拷贝到目标
static void tolower_copy(const char* src, char* dst, size_t maxlen){
    if(!src || !dst) return;
    size_t i;
    for(i=0;i<maxlen-1 && src[i];i++) dst[i]= (char)tolower((unsigned char)src[i]);
    dst[i]='\0';
}

int loadChampionHistory(const char* filename, ChampionRecord records[], int maxRecords){
    if(!filename || !records) return 0;
    FILE* f = fopen(filename, "r");
    if(!f) return 0;
    char line[1024];
    int idx = 0;
    while(fgets(line, sizeof(line), f) != NULL && idx < maxRecords){
        trim_newline(line);
        if(line[0]=='\0') continue;
        // 使用 '|' 分割：年份|地点|大学|国家|队员|教练
        char* tmp = strdup(line);
        char* tok;
        int field = 0;
        ChampionRecord rec;
        memset(&rec, 0, sizeof(rec));
        tok = strtok(tmp, "|");
        while(tok != NULL){
            trim_newline(tok);
            switch(field){
                case 0: strncpy(rec.year, tok, sizeof(rec.year)-1); break;
                case 1: strncpy(rec.location, tok, sizeof(rec.location)-1); break;
                case 2: strncpy(rec.university, tok, sizeof(rec.university)-1); break;
                case 3: strncpy(rec.country, tok, sizeof(rec.country)-1); break;
                case 4: strncpy(rec.teamMembers, tok, sizeof(rec.teamMembers)-1); break;
                case 5: strncpy(rec.coach, tok, sizeof(rec.coach)-1); break;
                default: break;
            }
            field++;
            tok = strtok(NULL, "|");
        }
        free(tmp);
        // 只要年份或大学等存在就算一条
        if(rec.year[0] != '\0' || rec.university[0] != '\0'){
            records[idx++] = rec;
        }
    }
    fclose(f);
    return idx;
}

static bool match_field(const char* fieldValue, const char* filter){
    if(!filter || filter[0]=='\0') return true; // 通配
    if(!fieldValue) return false;
    // 不区分大小写的包含匹配
    char lv[1024];
    char fv[1024];
    tolower_copy(filter, lv, sizeof(lv));
    tolower_copy(fieldValue, fv, sizeof(fv));
    if(strstr(fv, lv) != NULL) return true;
    return false;
}

int queryChampionByCriteria(ChampionRecord records[], int count, ChampionRecord results[], int maxResults,
                           const char* year, const char* location, const char* university,
                           const char* country, const char* teamMembers, const char* coach){
    if(!records || count<=0 || !results) return 0;
    int out = 0;
    for(int i=0;i<count && out < maxResults;i++){
        ChampionRecord* r = &records[i];
        if(!match_field(r->year, year)) continue;
        if(!match_field(r->location, location)) continue;
        if(!match_field(r->university, university)) continue;
        if(!match_field(r->country, country)) continue;
        if(!match_field(r->teamMembers, teamMembers)) continue;
        if(!match_field(r->coach, coach)) continue;
        results[out++] = *r;
    }
    return out;
}

// 打印字段并对匹配子串高亮（红色）
static void print_field_highlight(const char* label, const char* value, const char* filter){
    // const char* COLOR = "\x1b[31m"; //红色 ANSI 转义码
    const char* COLOR = HIGHLIGHT_COLOR;
    const char* RESET = ANSI_FRMT_RESET;
    if(!label) label = "";
    if(!value) value = "";
    if(!filter || filter[0] == '\0'){
        char buf[512];
        snprintf(buf, sizeof(buf), "%s: %s", label, value);
        printLeft(buf);
        // printf("%s: %s\n", label, value);
        return;
    }
    // 不区分大小写查找所有匹配片段并用红色包裹
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

    // printf("%s: ", label);
    char buffer[1280];
    snprintf(buffer, sizeof(buffer), "%s: ", label);
    const char* cur = value;
    const char* lowCur = lowVal;
    while(true){
        char* found = strstr(lowCur, lowFilter);
        if(!found){
            // 添加剩余部分到 buffer，循环结束后统一打印
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%s", cur);
            break;
        }
        // 计算偏移
        size_t prefixLen = (size_t)(found - lowCur);
        // 添加前缀到 buffer
        if(prefixLen > 0){
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%.*s", (int)prefixLen, cur);
        }
        // 添加高亮部分（使用原始大小写）到 buffer
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%s%.*s%s", COLOR, (int)flen, cur + prefixLen, RESET);
        // 移动指针继续查找
        cur += prefixLen + flen;
        lowCur += prefixLen + flen;
        if(*cur == '\0'){
            break;
        }
        // 继续循环以拼接后续部分
    }
    // 循环结束后一次性打印构造好的行，避免重复输出
    printLeft(buffer);
}

void printChampionRecord(const ChampionRecord* r,
                         const char* yearFilter, const char* locationFilter,
                         const char* universityFilter, const char* countryFilter,
                         const char* teamMembersFilter, const char* coachFilter){
    if(!r) return;
    
    print_field_highlight("年份", r->year, yearFilter);
    print_field_highlight("决赛地点", r->location, locationFilter);
    print_field_highlight("冠军大学", r->university, universityFilter);
    print_field_highlight("国家", r->country, countryFilter);
    print_field_highlight("队员", r->teamMembers, teamMembersFilter);
    print_field_highlight("教练", r->coach, coachFilter);
}

void interactiveChampionQuery(const char* filename){
    ChampionRecord records[MAX_CHAMPION_RECORDS];
    int count = loadChampionHistory(filename, records, MAX_CHAMPION_RECORDS);
    if(count == 0){
        printf("x> 无法读取或文件中无数据：%s\n", filename);
        return;
    }
    char buf[256];
    char year[64] = "";
    char location[128] = "";
    char university[256] = "";
    char country[128] = "";
    char teamMembers[256] = "";
    char coach[128] = "";

    printf("=> 输入查询条件，留空表示通配（回车跳过）\n");
    fgets(buf, sizeof(buf), stdin); // 清除残留换行 BUG WORK
    printf("=> 年份：");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    strncpy(year, buf, sizeof(year)-1);
    printf("=> 决赛地点：");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    strncpy(location, buf, sizeof(location)-1);
    printf("=> 冠军大学：");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    strncpy(university, buf, sizeof(university)-1);
    printf("=> 国家：");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    strncpy(country, buf, sizeof(country)-1);
    printf("=> 队员（可输入名字片段）：");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    strncpy(teamMembers, buf, sizeof(teamMembers)-1);
    printf("=> 教练：");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    strncpy(coach, buf, sizeof(coach)-1);

    ChampionRecord results[256];
    int found = queryChampionByCriteria(records, count, results, 256, year, location, university, country, teamMembers, coach);
    if(found == 0){
        printf("?> 未找到匹配的获奖记录。\n");
        return;
    }
    printf("√> 找到 %d 条记录：\n", found);
    printHeader();
    printCenter("获奖记录详情");
    printDivider();
    for(int i=0;i<found;i++){
        printChampionRecord(&results[i], year, location, university, country, teamMembers, coach);
        if(i < found - 1){
            printDivider();
        }
    }
    printFooter();
    
}

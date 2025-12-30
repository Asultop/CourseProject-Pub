
#ifndef DEF_H
#define DEF_H
//颜色&格式定义
#define ANSI_FRMT_RESET     "\e[0m"
#define ANSI_FRMT_BOLD      "\e[1m"
#define ANSI_FRMT_ITALICS   "\e[3m"
#define ANSI_FRMT_UNDERLINE "\e[4m"
#define ANSI_COLOR_RED      "\e[31m"
#define ANSI_COLOR_GREEN    "\e[32m"
#define ANSI_COLOR_YELLOW   "\e[33m"
#define ANSI_COLOR_BLUE     "\e[34m"
#define ANSI_COLOR_MAGENTA  "\e[35m"
#define ANSI_COLOR_CYAN     "\e[36m"
#define ANSI_BOLD_RED       "\e[31;1;1m"
#define ANSI_BOLD_GREEN     "\e[32;1;1m"
#define ANSI_BOLD_YELLOW    "\e[33;1;1m"
#define ANSI_BOLD_BLUE      "\e[34;1;1m"
#define ANSI_BOLD_MAGENTA   "\e[35;1;1m"
#define ANSI_BOLD_CYAN      "\e[36;1;1m"
#define ANSI_BOLD_WHITE     "\e[37;1;1m"


// 屏幕尺寸定义
#define SCREEN_CHAR_WIDTH 50
// 左右Margin
#define SCREEN_MARGIN_LEFT 2
#define SCREEN_MARGIN_RIGHT 2
// 高亮颜色定义

#define HIGHLIGHT_COLOR ANSI_COLOR_YELLOW

// 数据量定义
#define MAX_NAME_LEN 100
#define MAX_PASSWORD_LEN 100
#define MAX_USER_COUNT 1000
#define MAX_MESSAGE_LEN 256
#define MAX_PROBLEMS 2048
#define MAX_JUDGES_PER_PROBLEM 100
#define MAX_JUDGES_TIMELIMIT_MSEC 512 // 0.512 s

#define RETRY_DELAY_SECONDS 30
#define SLOW_TRY_LIMIT 5
#define MAX_TRY_COUNT 15
#define CAPTCHA_RETRY_LIMIT 3
#define MAX_CHAMPION_RECORDS 1024

// 目录定义
#define DATABASE_DIR "./DataBase"
#define PLATFORM_DIR DATABASE_DIR "/Platform"
#define PROBLEM_DIR PLATFORM_DIR "/Problems"
#define USERDATA_DIR DATABASE_DIR "/usrData"

// 文件定义
#define ACMT_DIR PLATFORM_DIR "/ACMT"
#define RULEFILE ACMT_DIR "/参赛规则.txt"
#define RATEDFILE ACMT_DIR "/评分标准.txt"
#define COMPFILE ACMT_DIR "/赛事构成.txt"
#define INTRFILE ACMT_DIR "/赛事介绍.txt"
#define AWARFILE ACMT_DIR "/历届获奖.txt"
// 用户信息结构体定义
typedef struct {
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
}UsrProfile;
// 历届冠军记录结构体定义
typedef struct {
    char year[16];
    char location[128];
    char university[256];
    char country[128];
    char teamMembers[512];
    char coach[256];
} ChampionRecord;
// 问题集定义
typedef struct {
    char id[64];
    char title[256];
    char difficulty[64];
    char type[128];
    char folderName[128];
    char problemPath[512];
} ProblemEntry;
// 判题结果返回
typedef enum{
    JUDGE_RESULT_ACCEPTED=1,
    JUDGE_RESULT_WRONG_ANSWER=2,
    JUDGE_RESULT_TIME_LIMIT_EXCEEDED=3,
    JUDGE_RESULT_RUNTIME_ERROR=4,
    JUDGE_RESULT_COMPILE_ERROR=5
} JudgeResult;
// 编码类型枚举
typedef enum {
    ENCODING_UTF8,
    ENCODING_GBK,
    ENCODING_UNKNOWN
} EncodingType;

// 返回信息定义
typedef enum {
    ERR=-1,
    SUCCESS=0
} UsrActionReturnInfo;
typedef struct {
    UsrActionReturnInfo info;
    char message[MAX_MESSAGE_LEN];
    UsrProfile* user;
} UsrActionReturnType;

typedef struct{
    JudgeResult result;
    char message[256];
} JudgeReturnInfo;
typedef struct{
    int count;
    JudgeReturnInfo infos[MAX_JUDGES_PER_PROBLEM];
} JudgeSummary;

#endif // DEF_H
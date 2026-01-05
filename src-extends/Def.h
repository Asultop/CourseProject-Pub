
#ifndef DEF_H
#define DEF_H
// 启用 POSIX 特性宏，确保 strdup/mkdtemp/fileno/nanosleep 等声明可用
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif


// 修复 Warning
// ========== 基础ANSI格式宏（原有+扩展） ==========
#define ANSI_FRMT_RESET         "\e[0m"          // 重置所有格式
#define ANSI_FRMT_BOLD          "\e[1m"          // 加粗
#define ANSI_FRMT_ITALICS       "\e[3m"          // 斜体
#define ANSI_FRMT_UNDERLINE     "\e[4m"          // 下划线
#define ANSI_FRMT_BLINK         "\e[5m"          // 闪烁（部分终端支持）
#define ANSI_FRMT_REVERSE       "\e[7m"          // 反色
#define ANSI_FRMT_STRIKETHROUGH "\e[9m"          // 删除线

// ========== 基础8色（前景色） ==========
#define ANSI_COLOR_BLACK        "\e[30m"         // 黑色
#define ANSI_COLOR_RED          "\e[31m"         // 红色
#define ANSI_COLOR_GREEN        "\e[32m"         // 绿色
#define ANSI_COLOR_YELLOW       "\e[33m"         // 黄色
#define ANSI_COLOR_BLUE         "\e[34m"         // 蓝色
#define ANSI_COLOR_MAGENTA      "\e[35m"         // 品红
#define ANSI_COLOR_CYAN         "\e[36m"         // 青色
#define ANSI_COLOR_WHITE        "\e[37m"         // 白色

// ========== 基础8色（背景色） ==========
#define ANSI_BG_BLACK           "\e[40m"         // 背景黑
#define ANSI_BG_RED             "\e[41m"         // 背景红
#define ANSI_BG_GREEN           "\e[42m"         // 背景绿
#define ANSI_BG_YELLOW          "\e[43m"         // 背景黄
#define ANSI_BG_BLUE            "\e[44m"         // 背景蓝
#define ANSI_BG_MAGENTA         "\e[45m"         // 背景品红
#define ANSI_BG_CYAN            "\e[46m"         // 背景青
#define ANSI_BG_WHITE           "\e[47m"         // 背景白

// ========== 高亮8色（前景色，加粗+高亮） ==========
#define ANSI_BOLD_BLACK         "\e[30;1m"       // 粗黑
#define ANSI_BOLD_RED           "\e[31;1m"       // 粗红（修正原冗余1；1）
#define ANSI_BOLD_GREEN         "\e[32;1m"       // 粗绿
#define ANSI_BOLD_YELLOW        "\e[33;1m"       // 粗黄
#define ANSI_BOLD_BLUE          "\e[34;1m"       // 粗蓝
#define ANSI_BOLD_MAGENTA       "\e[35;1m"       // 粗品红
#define ANSI_BOLD_CYAN          "\e[36;1m"       // 粗青
#define ANSI_BOLD_WHITE         "\e[37;1m"       // 粗白

// ========== 256色扩展（前景色，兼容大部分终端） ==========
#define ANSI_COLOR_256_GRAY     "\e[38;5;240m"   // 灰色（注释友好）
#define ANSI_COLOR_256_ORANGE    "\e[38;5;208m"   // 橙色（预处理指令）
#define ANSI_COLOR_256_PURPLE   "\e[38;5;129m"   // 紫色（运算符）
#define ANSI_COLOR_256_LIGHT_BLUE "\e[38;5;117m" // 浅蓝（关键字）
#define ANSI_COLOR_256_LIGHT_GREEN "\e[38;5;118m" // 浅绿（类型）
#define ANSI_COLOR_256_LIGHT_YELLOW "\e[38;5;226m" // 亮黄（数字）
#define ANSI_COLOR_256_PINK     "\e[38;5;206m"   // 粉色（字符串）
#define ANSI_COLOR_256_TEAL     "\e[38;5;38m"    // 蓝绿色（变量）

// ========== 真彩色扩展（RGB，现代终端支持） ==========
#define rgb( r, g, b) "\e[38;2;" #r ";" #g ";" #b "m"
#define ANSI_BG_RGB( r, g, b)    "\e[48;2;" #r ";" #g ";" #b "m"



//类似 VSCode 2017 Dark 主题配色

#define COLOR_KEYWORD           rgb(86, 156, 214) ANSI_FRMT_BOLD
#define COLOR_TYPE              rgb(86, 156, 214) ANSI_FRMT_BOLD
#define COLOR_VARIABLE          rgb(212, 212, 212)
#define COLOR_NUMBER            rgb(181, 206, 168)
#define COLOR_STRING            rgb(206, 145, 120)
#define COLOR_COMMENT           rgb(106, 153, 85)
#define COLOR_PREPROCESSOR      rgb(197, 134, 192) ANSI_FRMT_BOLD
#define COLOR_OPERATOR          rgb(212, 212, 212)
#define COLOR_DEFAULT           rgb(204, 204, 204) ANSI_FRMT_RESET


// 屏幕尺寸定义 (无符号类型 -1: 跟随终端宽度)
#define SCREEN_CHAR_WIDTH -1
// 开启 ColorPrint
#define RTXON 1
#define RANDOM_RTX_OFFSET // 注释则不开启
// 左右Margin
#define SCREEN_MARGIN_LEFT 1
#define SCREEN_MARGIN_RIGHT 1
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


// 跨平台

#ifdef _WIN32
    #define PATH_SEP '\\'
#else
    #define PATH_SEP '/'
#endif

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
// 中文支持 格式
typedef enum{
    MARGIN_LEFT,
    MARGIN_CENTER,
    MARGIN_RIGHT,
    MARGIN_NONE,
    MARGIN_ELSE
} PrintMarginType;
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
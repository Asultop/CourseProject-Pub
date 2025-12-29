
#ifndef DEF_H
#define DEF_H

// 数据量定义
#define MAX_NAME_LEN 100
#define MAX_PASSWORD_LEN 100
#define MAX_USER_COUNT 1000
#define MAX_MESSAGE_LEN 256
#define RETRY_DELAY_SECONDS 5
#define SLOW_TRY_LIMIT 5
#define MAX_TRY_COUNT 15
#define CAPTCHA_RETRY_LIMIT 3
// 目录定义
#define DATABASE_DIR "./database"
#define PLATFORM_DIR DATABASE_DIR "/Platform"
#define PROBLEM_DIR PLATFORM_DIR "/Problems"
#define SOLUTION_DIR PLATFORM_DIR "/Solutions"
#define USERDATA_DIR DATABASE_DIR "/usrData"

// 文件定义
#define RULEFILE PLATFORM_DIR "/参赛规则.txt"
#define RATEDFILE PLATFORM_DIR "/评分标准.txt"
#define COMPFILE PLATFORM_DIR "/赛事构成.txt"
#define INTRFILE PLATFORM_DIR "/赛事介绍.txt"
#define AWARFILE PLATFORM_DIR "/历届获奖.txt"
// 用户信息结构体定义
typedef struct {
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
}UsrProfile;

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
#endif // DEF_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "src-extends/championHistoryColManager.h"
#include "src-extends/passwordInputSimulator.h"
#include "src-extends/problemBankManager.h"
#include "src-extends/markdownPrinter.h"
#include "src-extends/releaseRuntime.h"
#include "src-extends/screenManager.h"
#include "src-extends/usrManager.h"
#include "src-extends/fileHelper.h"
#include "src-extends/md5.h"
#include "src-extends/Def.h"

#ifdef _WIN32
    #define sleep(seconds) Sleep((seconds) * 1000)
#endif

void cleanBuffer(void);
void cleanScreen(void);
void pauseScreen(void);


void displayFileContent(const char* filepath){
    // FILE* file = fopen(filepath, "r");
    // if(file == NULL){
    //     printf("x> 无法打开文件：%s\n", filepath);
    //     sleep(1);
    //     return;
    // }
    // char line[MAX_MESSAGE_LEN];
    // printf("====== 文件内容：%s ======\n", filepath);
    // while(fgets(line, sizeof(line), file) != NULL){
    //     printf("%s", line);
    // }
    // printf("\n====== 文件结束 ======\n");
    // fclose(file);
    // printf("=> 按任意键继续...");
    // getchar(); // 捕获换行符
    // getchar(); // 等待用户按键
    mdcat_worker(filepath);
    pauseScreen();
}
void getInACMIntroduction(){
    while(true){
        cleanScreen();
        printACMDetailScreen();
        int choice;
        if(scanf("%d", &choice) != 1){
            cleanBuffer();
            printf("?> 无效的选择，请重新输入。\n");
            sleep(1);
            continue;
        }
        switch (choice){
            case 1:
                displayFileContent(RULEFILE);
                break;
            case 2:
                displayFileContent(RATEDFILE);
                break;
            case 3:
                displayFileContent(COMPFILE);
                break;
            case 4:
                displayFileContent(INTRFILE);
                break;
            case 5:
                interactiveChampionQuery(AWARFILE);
                printf("√> 按下任意键继续...");
                getchar();
                break;
                
            case 0:
                return;
            default:
                printf("?> 无效的选择，请重新输入。\n");
                break;
        }
    }
}

UsrProfile globalUserGroup[MAX_USER_COUNT]={0};
UsrProfile currentUser={0};
bool envCheck(void);
void initDataBase(void);
// ========== 环境初始化模块 ==========
bool tryRepairEnvironment(void){
    puts("?> 是否要修复环境？(y/n)：");
    char choice;
    if(scanf(" %c", &choice) != 1 || (choice != 'y' && choice != 'Y')){
        printf("x> 环境未修复，程序退出！\n");
        sleep(1);
        return false;
    }
    printf("=> 正在修复环境...\n");
    releaseRuntimeResources(DATABASE_DIR);
    if(!envCheck()){
        printf("x> 环境修复失败，程序退出！\n");
        sleep(1);
        return false;
    }
    printf("√> 环境修复成功！\n");
    sleep(1);
    return true;
}

bool initEnvironment(void){
    if(!envCheck()){
        if(!tryRepairEnvironment()){
            return false;
        }
    }
    initDataBase();
    puts("");
    
    // 检查是否需要创建默认管理员账号
    if(!hasUsrInDB(globalUserGroup)){
        UsrProfile adminUser;
        createUser(globalUserGroup, &adminUser, "admin", "admin123");
        saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
        printf("√> 已创建默认管理员账号：admin，密码：admin123\n");
        sleep(3);
        puts("⎵> 按 Enter 键继续...");
        getchar();
    }
    return true;
}

// ========== 登录/注册界面模块 ==========
typedef enum {
    SPLASH_CONTINUE,    // 继续在登录界面
    SPLASH_LOGIN_OK,    // 登录成功
    SPLASH_EXIT         // 退出程序
} SplashResult;

SplashResult handleSplashChoice(int choice){
    switch (choice){
        case 1: // 登录
            if(!login(globalUserGroup, &currentUser)){
                printf("x> 登录失败\n");
                sleep(1);
                return SPLASH_CONTINUE;
            }
            printf("√> 欢迎，%s！\n", currentUser.name);
            sleep(1);
            return SPLASH_LOGIN_OK;
            
        case 2: // 注册
            if(!registerUser(globalUserGroup, &currentUser)){
                printf("x> 注册失败\n");
                sleep(1);
                return SPLASH_CONTINUE;
            }
            puts("√> 注册成功！请登录以继续。");
            sleep(1);
            return SPLASH_CONTINUE;
            
        case 3: // 修改密码
            if(!modifyAccount(globalUserGroup)){
                printf("x> 修改密码失败\n");
                sleep(1);
                return SPLASH_CONTINUE;
            }
            printf("√> 请重新登录以使用新密码。\n");
            sleep(1);
            return SPLASH_CONTINUE;
            
        case 4: // 删除用户
            if(!deleteUserFlow(globalUserGroup)){
                printf("x> 删除用户失败\n");
                sleep(1);
                return SPLASH_CONTINUE;
            }
            printf("√> 用户删除成功！请重新登录。\n");
            sleep(1);
            return SPLASH_CONTINUE;
            
        case 0: // 退出程序
            printf("√> 感谢使用，再见！\n");
            sleep(1);
            return SPLASH_EXIT;
            
        default:
            printf("?> 无效的选择，请重新输入。\n");
            sleep(1);
            return SPLASH_CONTINUE;
    }
}

bool runSplashScreen(void){
    while(true){
        cleanScreen();
        printSplashScreen();
        
        int choice;
        if(scanf("%d", &choice) != 1){
            cleanBuffer();
            printf("?> 无效的选择，请重新输入。\n");
            sleep(1);
            continue;
        }
        
        SplashResult result = handleSplashChoice(choice);
        if(result == SPLASH_LOGIN_OK){
            return true;  // 登录成功
        }else if(result == SPLASH_EXIT){
            return false; // 用户选择退出
        }
        // SPLASH_CONTINUE: 继续循环
    }
}

// ========== 主界面模块 ==========
typedef enum {
    MAIN_CONTINUE,  // 继续在主界面
    MAIN_LOGOUT     // 注销/返回登录界面
} MainMenuResult;

MainMenuResult handleMainMenuChoice(int choice){
    switch (choice){
        case 1: // ACM 竞赛简介
            getInACMIntroduction();
            return MAIN_CONTINUE;
            
        case 2: // ACM 题库
            interactiveProblemBank(PROBLEM_DIR, &currentUser);
            return MAIN_CONTINUE;
            
        case 0: // 注销/返回
            return MAIN_LOGOUT;
            
        default:
            printf("?> 无效的选择，请重新输入。\n");
            sleep(1);
            return MAIN_CONTINUE;
    }
}

void runMainMenu(void){
    while(true){
        cleanScreen();
        printMainScreen(currentUser.name);
        
        int choice;
        if(scanf("%d", &choice) != 1){
            cleanBuffer();
            printf("?> 无效的选择，请重新输入。\n");
            sleep(1);
            continue;
        }
        
        if(handleMainMenuChoice(choice) == MAIN_LOGOUT){
            break;
        }
    }
}

// ========== 验证码模块 ==========
extern char* getRandomCaptcha(){
    // 时间随机种子
    srand(time(NULL));
    // 字符表
    // static char c[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"; //全字表
    static char c[]="0123456789"; //数字字表
    char captcha[5];
    // 生成4位随机验证码
    for(int i = 0; i < 4; i++){
        srand(time(NULL) + i * 100);
        captcha[i] = c[rand() % (sizeof(c) - 1)];
    }
    captcha[4] = '\0';
    char* result = (char*)malloc(5 * sizeof(char));
    strcpy(result, captcha);
    return result;
}
extern bool checkCaptcha(int retryCount){
    if(retryCount <= 0){
        printf("x> 验证码尝试次数过多，操作已取消！\n");
        sleep(1);
        return false;
    }
    char* generatedCaptcha = getRandomCaptcha();
    char userInput[5];
    printf("=> 验证码：%4s\n", generatedCaptcha);
    printf("=> 请输入验证码：");
    
    scanf("%s", userInput);
    if(strcmp(generatedCaptcha, userInput) == 0){
        free(generatedCaptcha);
        return true;
    }else{
        free(generatedCaptcha);
        return checkCaptcha(retryCount - 1);
    }
}
bool envCheck(){
    // 检查必要的文件夹是否存在
    if(!dirExists(ACMT_DIR)){
        fprintf(stderr, "x> 必要的文件夹 %s 不存在！\n", ACMT_DIR);
        sleep(1);
        return false;    
    }
    if(!dirExists(USERDATA_DIR)){
        fprintf(stderr, "x> 必要的文件夹 %s 不存在！\n", USERDATA_DIR);
        sleep(1);
        return false;
    }
    if(!dirExists(PROBLEM_DIR)){
        fprintf(stderr, "x> 必要的文件夹 %s 不存在！\n", PROBLEM_DIR);
        sleep(1);
        return false;
    }
    return true;
}
void initDataBase(){
    // 初始化用户数据文件
    if(fileExists(USERDATA_DIR "/userData.txt") == false){
        if(createFile(USERDATA_DIR "/userData.txt") == false){
            fprintf(stderr, "x> 创建用户数据文件失败！\n");
            sleep(1);
            exit(EXIT_FAILURE);
        }
    }
    if(touchFile(USERDATA_DIR "/userData.txt") == false){
        fprintf(stderr, "x> 初始化用户数据文件失败！\n");
        sleep(1);
        exit(EXIT_FAILURE);
    }
    UsrActionReturnInfo getUsrResult = getAllUsrByReadDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(getUsrResult == ERR){
        fprintf(stderr, "x> 初始化用户数据失败！\n");
        sleep(1);
        exit(EXIT_FAILURE);
    }
}
// ========== 程序入口 ==========
int main(int argc, char *argv[]){
    (void)argc; // 标记参数未使用
    (void)argv;
    
    // 初始化环境
    if(!initEnvironment()){
        exit(EXIT_FAILURE);
    }
    
    // 主程序循环：登录界面 -> 主界面 -> 注销后返回登录界面
    while(true){
        // 登录/注册界面
        if(!runSplashScreen()){
            break; // 用户选择退出
        }
        
        // 主界面（登录成功后进入）
        runMainMenu();
        
        // 注销后清屏，继续循环回到登录界面
        cleanScreen();
    }
    
    return 0;
}

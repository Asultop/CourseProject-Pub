#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "src-extends/Def.h"
#include "src-extends/usrManager.h"
#include "src-extends/fileHelper.h"
#include "src-extends/passwordInputSimulator.h"
#include "src-extends/md5.h"
#include "src-extends/championHistoryColManager.h"
#include "src-extends/problemBankManager.h"
#include "src-extends/markdownPrinter.h"
#include "src-extends/screenManager.h"

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
int main(int argc,char *argv[]){
    // 初始化
    initDataBase();
    puts("");
    // 检查是否存在 userData.txt 文件
    if(!hasUsrInDB(globalUserGroup)){
        UsrProfile adminUser;
        createUser(globalUserGroup, &adminUser, "admin", "admin123");
        saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
        printf("√> 已创建默认管理员账号：admin，密码：admin123\n");
        sleep(3); // 特调 5 秒以便用户查看
        puts("⎵> 按 Enter 键继续...");
        getchar();
        // pauseScreen();
    }

    while(true){ // Splash 登录/注册界面
        cleanScreen();
        printSplashScreen();
        int choice;
        if(scanf("%d", &choice) != 1){
            cleanBuffer();
            printf("?> 无效的选择，请重新输入。\n");
            sleep(1);
            continue;
        }
        switch (choice){
            case 1:
                if(!login(globalUserGroup,&currentUser)){
                    printf("x> 登录失败\n");
                    sleep(1);
                    // exit(EXIT_FAILURE);
                    continue;
                }
                printf("√> 欢迎，%s！\n", currentUser.name);
                sleep(1);
                goto loginSuccess;
                break;
            case 2:
                if(!registerUser(globalUserGroup,&currentUser)){
                    printf("x> 注册失败\n");
                    sleep(1);
                    // exit(EXIT_FAILURE);
                    continue;
                }
                puts("√> 注册成功！请登录以继续。");
                sleep(1);
                break;
            case 3:
                //修改密码
                if(!modifyAccount(globalUserGroup)){
                    printf("x> 修改密码失败\n");
                    sleep(1);
                    continue;
                }
                printf("√> 请重新登录以使用新密码。\n");
                sleep(1);
                // exit(0);
                continue;
                break;
            case 4:
                //删除用户
                if(!deleteUserFlow(globalUserGroup)){
                    printf("x> 删除用户失败\n");
                    sleep(1);
                    // exit(EXIT_FAILURE);
                    continue;
                }
                printf("√> 用户删除成功！请重新登录。\n");
                sleep(1);
                // exit(0);
                continue;
                break;
            case 0:
                // 退出程序
                printf("√> 感谢使用，再见！\n");
                sleep(1);
                exit(0);
            default:
                printf("?> 无效的选择，请重新输入。\n");
                sleep(1);
                // goto Splash; // LINE DIFF_25 从定义处
                continue;
                break;
        }
        
    }
    loginSuccess:{}
    // 进入主界面
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
        if(choice == 0) break;
        switch (choice){
            case 1:
                // ACM 竞赛简介
                getInACMIntroduction();
                break;
            case 2:
                // ACM 题库
                interactiveProblemBank(PROBLEM_DIR, &currentUser);
                break;
            case 0:
                // 退出程序
                break;
            default:
                printf("?> 无效的选择，请重新输入。\n");
                sleep(1);
                break;
        }
    }
    cleanScreen();
    main(argc,argv);
    return 0;
}
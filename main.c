#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Def.h"
#include "usrManager.h"
#include "fileHelper.h"
#include "passwordInputSimulator.h"
#include "md5.h"
#include "championHistoryColManager.h"
#ifdef _WIN32
    #define sleep(seconds) Sleep((seconds) * 1000)
#endif
void cleanBuffer(){
    // int c;
    // while ((c = getchar()) != '\n' && c != EOF);
}
void cleanScreen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
void printSplashScreen(){
    puts(  "==== 欢迎使用在线评测系统 ===="  );
    puts(  "|----------------------------|");
    puts(  "|         请选择操作         |" );
    puts(  "|----------------------------|");
    puts(  "|                            |");
    puts(  "|   1.登录      2.注册       |" );
    puts(  "|   3.修改密码  4.删除用户   |" );
    puts(  "|                            |");
    puts(  "|----------------------------|");
    puts(  "|          0.退出            |");
    puts(  "|----------------------------|");
    printf("=> 请输入选项：[ ]\b\b"         );
}
void printMainScreen(const char * username){
    cleanScreen();
    printf("==== ACM 竞赛管理与训练系统 ====\n");
    printf("用户：%s\n", username);
    puts(  "|----------------------------|");
    puts(  "|         请选择操作         |" );
    puts(  "|----------------------------|");
    puts(  "|                            |");
    puts(  "|    1.ACM 竞赛简介          |" );
    puts(  "|    2.ACM 题库              |" );
    puts(  "|                            |");
    puts(  "|----------------------------|");
    puts(  "|    0.退出                  |");
    puts(  "|----------------------------|");
    printf("=> 请输入选项：[ ]\b\b"         );
}
void displayFileContent(const char* filepath){
    FILE* file = fopen(filepath, "r");
    if(file == NULL){
        printf("x> 无法打开文件：%s\n", filepath);
        return;
    }
    char line[MAX_MESSAGE_LEN];
    printf("====== 文件内容：%s ======\n", filepath);
    while(fgets(line, sizeof(line), file) != NULL){
        printf("%s", line);
    }
    printf("\n====== 文件结束 ======\n");
    fclose(file);
    printf("=> 按任意键继续...");
    getchar(); // 捕获换行符
    getchar(); // 等待用户按键
}
void getInACMIntroduction(){
    while(true){
        cleanScreen();
        puts("====== ACM 竞赛简介 ======");
        puts("|----------------------------|");
        puts("|         1.参赛规则         |");
        puts("|         2.评分标准         |");
        puts("|         3.赛事构成         |");
        puts("|         4.赛事介绍         |");
        puts("|         5.历届获奖         |");
        puts("|----------------------------|");
        puts("|         0.返回主菜单       |");
        puts("|----------------------------|");
        printf("=> 请输入选项：[ ]\b\b");
        int choice;
        cleanBuffer();
        scanf("%d", &choice);
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
                cleanBuffer();
                interactiveChampionQuery(AWARFILE);
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

char* getRandomCaptcha(){
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
bool checkCaptcha(int retryCount){
    if(retryCount <= 0){
        printf("x> 验证码尝试次数过多，操作已取消！\n");
        return false;
    }
    char* generatedCaptcha = getRandomCaptcha();
    char userInput[5];
    printf("=> 验证码：%4s\n", generatedCaptcha);
    printf("=> 请输入验证码：");
    cleanBuffer();
    scanf("%s", userInput);
    if(strcmp(generatedCaptcha, userInput) == 0){
        free(generatedCaptcha);
        return true;
    }else{
        free(generatedCaptcha);
        return checkCaptcha(retryCount - 1);
    }
}
bool login(UsrProfile * prof){
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
    cleanBuffer();
    scanf("%s", name);

    UsrActionReturnType result = queryUserByName(globalUserGroup, name);
    if(result.info == ERR){
        printf("x> 用户不存在！\n");
        return false;
    }
    int attempts = 0;
    while(true){
        if(attempts > 0 && attempts % SLOW_TRY_LIMIT == 0){
            printf("x> 尝试次数过多 请等待 %d 秒 再尝试\n", RETRY_DELAY_SECONDS);
            for(int i = RETRY_DELAY_SECONDS; i > 0; i--){
                printf("\r=> 等待时间：%d 秒 ", i);
                fflush(stdout);
                sleep(1);
            }
            puts("");
        }
        if(attempts > MAX_TRY_COUNT){
            printf("\n x> 尝试次数过多，登录失败！\n");
            return false;
        }
        printf("=> 请输入密码 (输入IDK退出)：");
        getpwd(password, MAX_PASSWORD_LEN);
        if(strcmp(password, "IDK") == 0){
            printf("x> 已取消登录！\n");
            return false;
        }
        if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
            printf("x> 验证码错误，请重新登录！\n");
            return false;
        }
        if(loginUser(name, password)){
            strcpy(prof->name, name);
            strcpy(prof->password, password);
            printf("√> 登录成功！欢迎，%s\n", name);
            return true;
        }else{
            printf("x> 用户名或密码错误！\n");
        }
        attempts++;
    }
    
}
bool registerUser(UsrProfile * prof){
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
    cleanBuffer();
    scanf("%s", name);
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == SUCCESS){
        printf("x> 用户名已存在，请重新注册！\n");
        return false;
    }
    printf("=> 请输入密码：");
    getpwd(password, MAX_PASSWORD_LEN);
    if(strcmp(password, "IDK") == 0){
        printf("x> 无法使用IDK作为密码，已取消注册！\n");
        return false;
    }
    printf("=> 请再次输入密码：");
    char passwordConfirm[MAX_PASSWORD_LEN];
    getpwd(passwordConfirm, MAX_PASSWORD_LEN);
    if(strcmp(password, passwordConfirm) != 0){
        printf("x> 两次输入的密码不一致，请重新注册！\n");
        return false;
    }
    
    if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
        printf("x> 验证码错误，请重新注册！\n");
        return false;
    }
    UsrProfile newUser;
    UsrActionReturnType createResult = createUser(globalUserGroup, &newUser, name, password);
    if(createResult.info == ERR){
        printf("x> %s\n", createResult.message);
        return false;
    }
    UsrActionReturnInfo saveResult = saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(saveResult == ERR){
        printf("x> 保存用户数据失败！\n");
        return false;
    }
    printf("√> 注册成功！欢迎，%s\n", name);
    strcpy(prof->name, name);
    strcpy(prof->password, password);
    return true;
}
bool modifyAccount(){
    char name[MAX_NAME_LEN];
    char oldPassword[MAX_PASSWORD_LEN];
    char newPassword[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
    cleanBuffer();
    scanf("%s", name);
    if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
        printf("x> 验证码错误，已取消修改！\n");
        return false;
    }
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == ERR){
        printf("x> 用户不存在！\n");
        return false;
    }
    printf("=> 请输入旧密码：");
    getpwd(oldPassword, MAX_PASSWORD_LEN);
    char md5OldPassword[33];
    MD5_String(oldPassword, md5OldPassword);
    if(strcmp(md5OldPassword, queryResult.user->password) != 0){
        printf("x> 旧密码错误！\n");
        return false;
    }
    printf("=> 请输入新密码：");
    getpwd(newPassword, MAX_PASSWORD_LEN);
    if(strcmp(newPassword, "IDK") == 0){
        printf("x> 无法使用IDK作为密码，已取消修改！\n");
        return false;
    }
    char newPasswordConfirm[MAX_PASSWORD_LEN];
    printf("=> 请再次输入新密码：");
    getpwd(newPasswordConfirm, MAX_PASSWORD_LEN);

    
    if(strcmp(newPassword, newPasswordConfirm) != 0){
        printf("x> 两次输入的新密码不一致！\n");
        return false;
    }
    char md5NewPassword[33];
    MD5_String(newPassword, md5NewPassword);
    strcpy(queryResult.user->password, md5NewPassword);
    UsrActionReturnInfo saveResult = saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(saveResult == ERR){
        printf("x> 保存用户数据失败！\n");
        return false;
    }
    printf("√> 密码修改成功！\n");
    return true;
}
bool deleteUserFlow(UsrProfile globalUserGroup[]){
    char name[MAX_NAME_LEN];
    printf("=> 请输入要删除的用户名：");
    cleanBuffer();
    scanf("%s", name);
    if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
        printf("x> 验证码错误，已取消删除！\n");
        return false;
    }
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == ERR){
        printf("x> 用户不存在！\n");
        return false;
    }
    UsrActionReturnType deleteResult = deleteUserByName(globalUserGroup, name);
    if(deleteResult.info == ERR){
        printf("x> 删除用户失败！\n");
        return false;
    }
    UsrActionReturnInfo saveResult = saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(saveResult == ERR){
        printf("x> 保存用户数据失败！\n");
        return false;
    }
    return true;
}
void initDataBase(){
    // 初始化用户数据文件
    if(fileExists(USERDATA_DIR "/userData.txt") == false){
        if(createFile(USERDATA_DIR "/userData.txt") == false){
            fprintf(stderr, "x> 创建用户数据文件失败！\n");
            exit(EXIT_FAILURE);
        }
    }
    if(touchFile(USERDATA_DIR "/userData.txt") == false){
        fprintf(stderr, "x> 初始化用户数据文件失败！\n");
        exit(EXIT_FAILURE);
    }
    UsrActionReturnInfo getUsrResult = getAllUsrByReadDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(getUsrResult == ERR){
        fprintf(stderr, "x> 初始化用户数据失败！\n");
        exit(EXIT_FAILURE);
    }
}
int main(int argc,char *argv[]){
    // 初始化
    initDataBase();
    // 检查是否存在 userData.txt 文件
    if(!hasUsrInDB(globalUserGroup)){
        UsrProfile adminUser;
        createUser(globalUserGroup, &adminUser, "admin", "admin123");
        saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
        printf("√> 已创建默认管理员账号：admin，密码：admin123\n");
    }

    LINE_DIFF_25_splash: { // Splash 登录/注册界面
        printSplashScreen();
        int choice;
        cleanBuffer();
        scanf("%d", &choice);
        switch (choice){
            case 1:
                if(!login(&currentUser)){
                    printf("x> 登录失败\n");
                    exit(EXIT_FAILURE);
                }
                printf("√> 欢迎，%s！\n", currentUser.name);
                
                break;
            case 2:
                if(!registerUser(&currentUser)){
                    printf("x> 注册失败\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 3:
                //修改密码
                if(!modifyAccount()){
                    printf("x> 修改密码失败\n");
                    exit(EXIT_FAILURE);
                }
                printf("√> 请重新登录以使用新密码。\n");
                exit(0);
                break;
            case 4:
                //删除用户
                if(!deleteUserFlow(globalUserGroup)){
                    printf("x> 删除用户失败\n");
                    exit(EXIT_FAILURE);
                }
                printf("√> 用户删除成功！请重新登录。\n");
                exit(0);
                break;
            case 0:
                // 退出程序
                printf("√> 感谢使用，再见！\n");
                exit(0);
            default:
                printf("?> 无效的选择，请重新输入。\n");
                goto LINE_DIFF_25_splash; // LINE DIFF_25 从定义处
                break;
        }
    }
    sleep(1);
    // 进入主界面
    while(true){
        printMainScreen(currentUser.name);
        int choice;
        cleanBuffer();
        scanf("%d", &choice);
        switch (choice){
            case 1:
                // ACM 竞赛简介
                getInACMIntroduction();
                break;
            case 2:
                // ACM 题库
                // manageProblemBank();
                break;
            case 0:
                // 退出程序
                printf("√> 感谢使用，再见！\n");
                exit(0);
            default:
                printf("?> 无效的选择，请重新输入。\n");
                break;
        }
    }
    return 0;
}
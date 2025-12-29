#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Def.h"
#include "usrManager.h"
#include "fileHelper.h"
#include "passwordInputSimulator.h"
#ifdef _WIN32
    #define sleep(seconds) Sleep((seconds) * 1000)
#endif
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
bool checkCaptcha(){
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
        return checkCaptcha();
    }
}
bool login(UsrProfile * prof){
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
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
        if(!checkCaptcha()){
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
    
    if(!checkCaptcha()){
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
    scanf("%s", name);
    //TODO-RECENT
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

            case 4:
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
    while(true){
        printMainScreen(currentUser.name);
        int choice;
        scanf("%d", &choice);

    }
    return 0;
}
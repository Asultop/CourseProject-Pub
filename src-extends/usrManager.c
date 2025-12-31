#include "usrManager.h"
#include "passwordInputSimulator.h"
#include "Def.h"
#include "md5.h"
#include <stdio.h>
#include "fileHelper.h"
#include <string.h>
#include <stdlib.h>
bool checkCaptcha(int retryCount);

UsrActionReturnType createUser(UsrProfile globalUserGroup[],UsrProfile* user, const char* name, const char* password){
    if(strlen(name) >= MAX_NAME_LEN || strlen(password) >= MAX_PASSWORD_LEN){
        return (UsrActionReturnType){ERR,"参数超过要求", NULL};
    }
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup,name);
    if(queryResult.info == SUCCESS){
        return (UsrActionReturnType){ERR, "用户已存在", NULL};
    }
    strcpy(user->name, name);
    char hashedPassword[33];
    MD5_String(password, hashedPassword);
    strcpy(user->password, hashedPassword);

    return addToGlobalUserGroup(globalUserGroup, user);
    
}

UsrActionReturnType queryUserByName(UsrProfile globalUserGroup[], const char* name){
    for(int i = 0; i < MAX_USER_COUNT; i++){
        if(globalUserGroup[i].name[0] == '\0'){
            continue;
        }
        if(strcmp(globalUserGroup[i].name, name) == 0){
            return (UsrActionReturnType){SUCCESS, "用户查询成功", &globalUserGroup[i]};
        }
    }
    return (UsrActionReturnType){ERR, "用户不存在", NULL};
}

UsrActionReturnType deleteUserByName(UsrProfile globalUserGroup[], const char* name){
    for(int i = 0; i < MAX_USER_COUNT; i++){
        if(globalUserGroup[i].name[0] == '\0'){
            continue;
        }
        if(strcmp(globalUserGroup[i].name, name) == 0){
            globalUserGroup[i].name[0] = '\0';
            globalUserGroup[i].password[0] = '\0';
            return (UsrActionReturnType){SUCCESS, "用户删除成功", NULL};
        }
    }
    return (UsrActionReturnType){ERR, "用户不存在，无法删除", NULL};
}

UsrActionReturnType addToGlobalUserGroup(UsrProfile globalUserGroup[], UsrProfile* user){
    for(int i = 0; i < MAX_USER_COUNT; i++){
        if(globalUserGroup[i].name[0] == '\0'){
            globalUserGroup[i] = *user;
            return (UsrActionReturnType){SUCCESS, "用户添加成功", &globalUserGroup[i]};
        }
        if(strcmp(globalUserGroup[i].name, user->name) == 0){
            return (UsrActionReturnType){ERR, "用户已存在，无法添加", NULL};
        }
    }
    return (UsrActionReturnType){ERR, "用户组已满，无法添加新用户", NULL};
}

UsrActionReturnInfo getAllUsrByReadDataFile(UsrProfile globalUserGroup[],const char* filename){
    FILE* file = openFile(filename, "r");
    if(file == NULL){
        return ERR;
    }
    char line[MAX_NAME_LEN + MAX_PASSWORD_LEN + 10];
    int index = 0;
    while(fgets(line, sizeof(line), file) != NULL && index < MAX_USER_COUNT){
        char name[MAX_NAME_LEN];
        char password[MAX_PASSWORD_LEN];
        if(sscanf(line, "%s %s", name, password) == 2){
            strcpy(globalUserGroup[index].name, name);
            strcpy(globalUserGroup[index].password, password);
            index++;
        }
    }
    closeFile(file);
    return SUCCESS;
}
UsrActionReturnInfo saveAllUsrToDataFile(UsrProfile globalUserGroup[],const char* filename){
    FILE* file = openFile(filename, "w");
    if(file == NULL){
        return ERR;
    }
    for(int i = 0; i < MAX_USER_COUNT; i++){
        if(globalUserGroup[i].name[0] != '\0'){
            fprintf(file, "%s %s\n", globalUserGroup[i].name, globalUserGroup[i].password);
        }
    }
    closeFile(file);
    return SUCCESS;
}
bool loginUser(UsrProfile globalUserGroup[], const char* name, const char* password){

    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == ERR){
        return false;
    }
    char hashedPassword[33];
    MD5_String(password, hashedPassword);
    if(strcmp(queryResult.user->password, hashedPassword) == 0){
        return true;
    }
    return false;
}
bool hasUsrInDB(UsrProfile globalUserGroup[]){
    for(int i = 0; i < MAX_USER_COUNT; i++){
        if(globalUserGroup[i].name[0] != '\0'){
            return true;
        }
    }
    return false;
}
bool login(UsrProfile globalUserGroup[],UsrProfile * prof){
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
    
    scanf("%s", name);

    UsrActionReturnType result = queryUserByName(globalUserGroup, name);
    if(result.info == ERR){
        printf("x> 用户不存在！\n");
        sleep(1);
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
            sleep(1);
            return false;
        }
        printf("=> 请输入密码 (输入IDK退出)：");
        getpwd(password, MAX_PASSWORD_LEN);
        if(strcmp(password, "IDK") == 0){
            printf("x> 已取消登录！\n");
            sleep(1);
            return false;
        }
        if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
            printf("x> 验证码错误，请重新登录！\n");
            sleep(1);
            return false;
        }
        if(loginUser(globalUserGroup,name, password)){
            strcpy(prof->name, name);
            strcpy(prof->password, password);
            printf("√> 登录成功！欢迎，%s\n", name);
            return true;
        }else{
            printf("x> 用户名或密码错误！\n");
            sleep(1);
        }
        attempts++;
    }
    
}
bool registerUser(UsrProfile globalUserGroup[],UsrProfile * prof){
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
    
    scanf("%s", name);
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == SUCCESS){
        printf("x> 用户名已存在，请重新注册！\n");
        sleep(1);
        return false;
    }
    printf("=> 请输入密码：");
    getpwd(password, MAX_PASSWORD_LEN);
    if(strcmp(password, "IDK") == 0){
        printf("x> 无法使用IDK作为密码，已取消注册！\n");
        sleep(1);
        return false;
    }
    printf("=> 请再次输入密码：");
    char passwordConfirm[MAX_PASSWORD_LEN];
    getpwd(passwordConfirm, MAX_PASSWORD_LEN);
    if(strcmp(password, passwordConfirm) != 0){
        printf("x> 两次输入的密码不一致，请重新注册！\n");
        sleep(1);
        return false;
    }
    
    if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
        printf("x> 验证码错误，请重新注册！\n");
        sleep(1);
        return false;
    }
    UsrProfile newUser;
    UsrActionReturnType createResult = createUser(globalUserGroup, &newUser, name, password);
    if(createResult.info == ERR){
        printf("x> %s\n", createResult.message);
        sleep(1);
        return false;
    }
    UsrActionReturnInfo saveResult = saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(saveResult == ERR){
        printf("x> 保存用户数据失败！\n");
        sleep(1);
        return false;
    }
    printf("√> 注册成功！欢迎，%s\n", name);
    strcpy(prof->name, name);
    strcpy(prof->password, password);
    return true;
}
bool modifyAccount(UsrProfile globalUserGroup[]){
    char name[MAX_NAME_LEN];
    char oldPassword[MAX_PASSWORD_LEN];
    char newPassword[MAX_PASSWORD_LEN];
    printf("=> 请输入用户名：");
    scanf("%s", name);
    if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
        printf("x> 验证码错误，已取消修改！\n");
        sleep(1);
        return false;
    }
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == ERR){
        printf("x> 用户不存在！\n");
        sleep(1);
        return false;
    }
    printf("=> 请输入旧密码：");
    getpwd(oldPassword, MAX_PASSWORD_LEN);
    char md5OldPassword[33];
    MD5_String(oldPassword, md5OldPassword);
    if(strcmp(md5OldPassword, queryResult.user->password) != 0){
        printf("x> 旧密码错误！\n");
        sleep(1);
        return false;
    }
    printf("=> 请输入新密码：");
    getpwd(newPassword, MAX_PASSWORD_LEN);
    if(strcmp(newPassword, "IDK") == 0){
        printf("x> 无法使用IDK作为密码，已取消修改！\n");
        sleep(1);
        return false;
    }
    char newPasswordConfirm[MAX_PASSWORD_LEN];
    printf("=> 请再次输入新密码：");
    getpwd(newPasswordConfirm, MAX_PASSWORD_LEN);

    
    if(strcmp(newPassword, newPasswordConfirm) != 0){
        printf("x> 两次输入的新密码不一致！\n");
        sleep(1);
        return false;
    }
    char md5NewPassword[33];
    MD5_String(newPassword, md5NewPassword);
    strcpy(queryResult.user->password, md5NewPassword);
    UsrActionReturnInfo saveResult = saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(saveResult == ERR){
        printf("x> 保存用户数据失败！\n");
        sleep(1);
        return false;
    }
    printf("√> 密码修改成功！\n");
    sleep(1);
    return true;
}
bool deleteUserFlow(UsrProfile globalUserGroup[]){
    char name[MAX_NAME_LEN];
    printf("=> 请输入要删除的用户名：");
    scanf("%s", name);
    if(!checkCaptcha(CAPTCHA_RETRY_LIMIT)){
        printf("x> 验证码错误，已取消删除！\n");
        sleep(1);
        return false;
    }
    UsrActionReturnType queryResult = queryUserByName(globalUserGroup, name);
    if(queryResult.info == ERR){
        printf("x> 用户不存在！\n");
        sleep(1);
        return false;
    }
    UsrActionReturnType deleteResult = deleteUserByName(globalUserGroup, name);
    if(deleteResult.info == ERR){
        printf("x> 删除用户失败！\n");
        sleep(1);
        return false;
    }
    UsrActionReturnInfo saveResult = saveAllUsrToDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(saveResult == ERR){
        printf("x> 保存用户数据失败！\n");
        sleep(1);
        return false;
    }
    return true;
}
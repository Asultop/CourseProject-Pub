#include "usrManager.h"
#include "Def.h"
#include "md5.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    FILE* file = fopen(filename, "r");
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
    fclose(file);
    return SUCCESS;
}
UsrActionReturnInfo saveAllUsrToDataFile(UsrProfile globalUserGroup[],const char* filename){
    FILE* file = fopen(filename, "w");
    if(file == NULL){
        return ERR;
    }
    for(int i = 0; i < MAX_USER_COUNT; i++){
        if(globalUserGroup[i].name[0] != '\0'){
            fprintf(file, "%s %s\n", globalUserGroup[i].name, globalUserGroup[i].password);
        }
    }
    fclose(file);
    return SUCCESS;
}
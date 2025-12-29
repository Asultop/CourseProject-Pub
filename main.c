#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Def.h"
#include "usrManager.h"
#include "fileHelper.h"

void printSplashScreen(){
    puts("==== 欢迎使用在线评测系统 ====");
    puts("|----------------------------|");
    puts("|         请选择操作         |");
    puts("|----------------------------|");
    puts("|                            |");
    puts("|    1.登录     2.注册       |");
    puts("|                            |");
    puts("|----------------------------|");
    puts("|    0.退出                  |");
    puts("|----------------------------|");
}


UsrProfile globalUserGroup[MAX_USER_COUNT]={0};
void initDataBase(){
    // 初始化用户数据文件
    if(fileExists(USERDATA_DIR "/userData.txt") == false){
        if(createFile(USERDATA_DIR "/userData.txt") == false){
            fprintf(stderr, "创建用户数据文件失败！\n");
            exit(EXIT_FAILURE);
        }
    }
    if(touchFile(USERDATA_DIR "/userData.txt") == false){
        fprintf(stderr, "初始化用户数据文件失败！\n");
        exit(EXIT_FAILURE);
    }
    UsrActionReturnInfo getUsrResult = getAllUsrByReadDataFile(globalUserGroup, USERDATA_DIR "/userData.txt");
    if(getUsrResult == ERR){
        fprintf(stderr, "初始化用户数据失败！\n");
        exit(EXIT_FAILURE);
    }



}
int main(int argc,char *argv[]){
    printSplashScreen();
    return 0;
}
#ifndef USRMANAGER_H
#define USRMANAGER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Def.h"


UsrActionReturnType createUser(UsrProfile globalUserGroup[],UsrProfile* user, const char* name, const char* password);
UsrActionReturnType queryUserByName(UsrProfile globalUserGroup[], const char* name);
UsrActionReturnType deleteUserByName(UsrProfile globalUserGroup[], const char* name);
UsrActionReturnType addToGlobalUserGroup(UsrProfile globalUserGroup[], UsrProfile* user);

UsrActionReturnInfo getAllUsrByReadDataFile(UsrProfile globalUserGroup[],const char* filename);
UsrActionReturnInfo saveAllUsrToDataFile(UsrProfile globalUserGroup[],const char* filename);

bool loginUser(UsrProfile globalUserGroup[], const char* name, const char* password);
bool hasUsrInDB(UsrProfile globalUserGroup[]);

// Interactive - 交互式函数 
bool login(UsrProfile globalUserGroup[],UsrProfile* prof);
bool registerUser(UsrProfile globalUserGroup[],UsrProfile* prof);
bool modifyAccount(UsrProfile globalUserGroup[]);
bool deleteUserFlow(UsrProfile globalUserGroup[]);
#endif // USRMANAGER_H
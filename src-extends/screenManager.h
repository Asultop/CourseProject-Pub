#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include "Def.h"
#include <stdio.h>

void printSplashScreen();
void printMainScreen(const char * username);
void printACMDetailScreen();
void printACMProblemBankScreen(const char * currentUser);

void printHeader();
void printFooter();
void printDivider();
void printCenter(const char* content);
void printContent(const char * contentLine);
void printLeft(const char * contentLine);
void printRight(const char * contentLine);
#endif // SCREEN_MANAGER_H
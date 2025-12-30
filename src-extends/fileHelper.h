#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
bool fileExists(const char* filename);
bool touchFile(const char* filename);
bool createFile(const char* filename);

#endif // FILEHELPER_H
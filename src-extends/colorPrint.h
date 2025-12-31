#ifndef COLORPRINT_H
#define COLORPRINT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "chineseSupport.h"
char * rainbowStringGenerator(size_t length); // 带 ANSI 真彩色
char * rainbowizeString(const char * inputStr);


#endif // COLORPRINT_H
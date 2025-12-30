#ifndef CODE_RENDER_H
#define CODE_RENDER_H

#include <stdio.h>
#include <stddef.h>  // C11标准头文件，提供size_t
#include "Def.h"
#include "chineseSupport.h"

// 以带语法高亮的形式渲染并打印代码文件，返回0表示成功，非0表示失败
int codeRender_worker(const char* file);

#endif // CODE_RENDER_H
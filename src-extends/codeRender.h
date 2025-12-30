#ifndef CODE_RENDER_H
#define CODE_RENDER_H

#include <stdio.h>
#include <stddef.h>  // C11标准头文件，提供size_t
#include "Def.h"
#include "chineseSupport.h"

// ========== C/C++高亮颜色映射（可自定义） ==========
#define COLOR_KEYWORD       ANSI_BOLD_BLUE      // 关键字：粗蓝
#define COLOR_COMMENT       ANSI_COLOR_GREEN    // 注释：绿色
#define COLOR_STRING        ANSI_BOLD_RED       // 字符串：粗红
#define COLOR_TYPE          ANSI_BOLD_CYAN      // 类型关键字：粗青
#define COLOR_NUMBER        ANSI_BOLD_YELLOW    // 数字：粗黄
#define COLOR_PREPROCESSOR  ANSI_BOLD_MAGENTA   // 预处理指令：粗紫
#define COLOR_OPERATOR      ANSI_COLOR_YELLOW   // 运算符：黄色
#define COLOR_DEFAULT       ANSI_FRMT_RESET     // 默认：重置
#define COLOR_VARIABLE     ANSI_COLOR_MAGENTA     // 变量：青色

// C11特性：静态断言（编译期验证ANSI宏长度）

// static_assert(sizeof(ANSI_FRMT_RESET) <= 16, "ANSI转义序列长度超出预期");
// static_assert(sizeof(COLOR_KEYWORD) <= 32, "高亮颜色宏长度超出预期");

#ifdef __cplusplus
extern "C" {
#endif

// 以带语法高亮的形式渲染并打印代码文件，返回0表示成功，非0表示失败
int codeRender_worker(const char* file);

#ifdef __cplusplus
}
#endif

#endif // CODE_RENDER_H
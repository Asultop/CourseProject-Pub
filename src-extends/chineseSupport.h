#ifndef CHINESE_SUPPORT_H
#define CHINESE_SUPPORT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Def.h"

/************************ 工具函数：UTF-8合法性检测 ************************/
int is_utf8_valid(const char *str);
/************************ 工具函数：检测字符串编码 ************************/
EncodingType detect_encoding(const char *str);
/************************ 工具函数：UTF-8转Unicode ************************/
unsigned int utf8_to_unicode(const unsigned char *utf8);
/************************ 统计UTF-8中文字符（仅汉字） ************************/
int count_utf8_chinese(const char *str);
/************************ 统计GBK中文字符（仅汉字） ************************/
int count_gbk_chinese(const char *str);
/************************ 通用中文字符统计函数（自动识别编码） ************************/
int count_chinese(const char *str, EncodingType *detected_encoding);
/************************ GBK中文字符判断 ************************/
int is_gbk_chinese_char(const unsigned char *c);
/************************ UTF-8中文/全角字符判断 ************************/
int is_utf8_double_width(const unsigned char *c, int i, int len);
/************************ 获取字符串实际显示长度（含中文） ************************/
unsigned long get_real_Length(const char * str, EncodingType *encoding);
/************************ 根据边距类型获取填充空格字符串 ************************/
char * getSpaceContent(const char *content, size_t totalWidth, PrintMarginType marginType);

// 将原始行按“可显示字符”拆分为数组，每个元素为一个显示单元（ASCII单字符或多字节中文/全角符号或转义序列）。
// 返回新分配的 `char**`，数组以最后元素为 "EOL" 标记结束。调用方需对返回值以及其中每个字符串使用 free() 释放。
char ** processRawChar(const char * rawLine);

// 释放由 processRawChar 返回的数组
void freeProcessedChars(char ** arr);

#endif // CHINESE_SUPPORT_H
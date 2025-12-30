#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

// 将基目录与相对路径拼接并规范化（处理 "." 和 ".."），结果写入 out（大小 outsz）。
// out 在 outsz>0 时会以 '\0' 结尾。
void joinBasedirAndRel(const char *basedir, const char *rel, char *out, size_t outsz);

// 打开/关闭/读取/复制/删除文件的工具封装（小驼峰命名）
FILE* openFile(const char* path, const char* mode);
void closeFile(FILE* f);
// 读取整个文件为 malloc 分配的字符串，调用者负责 free
char* readFileToStr(const char* path);
// 复制文件，成功返回 true
bool copyFile(const char* src, const char* dst);
// 删除文件，成功返回 true
bool removeFile(const char* path);
bool fileExists(const char* filename);
bool touchFile(const char* filename);
bool createFile(const char* filename);

#endif // FILEHELPER_H
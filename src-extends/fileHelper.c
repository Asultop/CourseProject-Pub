#include "fileHelper.h"
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
bool fileExists(const char* filename){
    FILE* file = fopen(filename, "r");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}
bool dirExists(const char* dirname){
    #ifdef _WIN32
        DWORD ftyp = GetFileAttributesA(dirname);
        if (ftyp == INVALID_FILE_ATTRIBUTES)
            return false;  // 目录不存在
        if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
            return true;   // 是目录
        return false;      // 不是目录
    #else
        struct stat st;
        if (stat(dirname, &st) == 0 && S_ISDIR(st.st_mode)){
            return true;
        }
        return false;
    #endif
}
bool touchFile(const char* filename){
    FILE* file = fopen(filename, "a");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}
bool createFile(const char* filename){
    {
        if (!filename) return false;
        char dir[PATH_MAX];
        strncpy(dir, filename, sizeof(dir)-1);
        dir[sizeof(dir)-1] = '\0';
        char *p = dir + strlen(dir);
        while (p > dir && *p != '/' && *p != '\\') --p;
        if (p == dir && *p != '/' && *p != '\\') {
            
        } else {
            if (*p) *p = '\0';
            size_t len = strlen(dir);
            size_t start = 1;
            
            if (len > 1 && dir[1] == ':') start = 3; // 省略驱动器 (Windows)
            for (size_t i = start; i <= len; ++i) {
                if (dir[i] == '/' || dir[i] == '\\' || dir[i] == '\0') {
                    char save = dir[i];
                    dir[i] = '\0';
                    if (!dirExists(dir)) {
                        #ifdef _WIN32
                            if (!CreateDirectoryA(dir, NULL)) {
                                #ifdef _MSC_VER
                                    _mkdir(dir);
                                #else
                                    mkdir(dir, 0755);
                                #endif
                            }
                        #else
                            mkdir(dir, 0755);
                        #endif
                    }
                    dir[i] = save;
                }
            }
        }
    }

    FILE* file = fopen(filename, "w");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}

// 将基目录与相对路径拼接并规范化（处理 "." 和 ".."），结果写入 out（大小 outsz）。
void joinBasedirAndRel(const char *basedir, const char *rel, char *out, size_t outsz) {
    if (!out || outsz == 0) return;
    out[0] = '\0';
    if (!rel || rel[0] == '\0') {
        if (basedir) strncpy(out, basedir, outsz-1);
        out[outsz-1] = '\0';
        return;
    }
    // 如果 rel 是绝对路径，则规范化 rel
    if (rel[0] == '/') {
        char buf[4096]; strncpy(buf, rel, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
        char *components[256]; size_t compc = 0;
        char *tok = strtok(buf, "/");
        while (tok) {
            if (strcmp(tok, ".") == 0) { tok = strtok(NULL, "/"); continue; }
            if (strcmp(tok, "..") == 0) { if (compc > 0) compc--; tok = strtok(NULL, "/"); continue; }
            components[compc++] = tok;
            tok = strtok(NULL, "/");
        }
        size_t pos = 0;
        if (pos + 1 < outsz) out[pos++] = '/';
        for (size_t i = 0; i < compc; ++i) {
            size_t len = strlen(components[i]);
            if (pos + len + 1 < outsz) { memcpy(out + pos, components[i], len); pos += len; out[pos++] = '/'; }
            else break;
        }
        if (pos > 0 && pos < outsz) out[pos-1] = '\0'; else if (pos < outsz) out[pos] = '\0';
        return;
    }

    // 拼接 basedir 和 rel
    char joined[4096];
    if (basedir && basedir[0] != '\0') snprintf(joined, sizeof(joined), "%s/%s", basedir, rel);
    else snprintf(joined, sizeof(joined), "%s", rel);

    char buf[4096]; strncpy(buf, joined, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
    char *components[256]; size_t compc = 0;
    char *tok = strtok(buf, "/");
    while (tok) {
        if (strcmp(tok, ".") == 0) { tok = strtok(NULL, "/"); continue; }
        if (strcmp(tok, "..") == 0) { if (compc > 0) compc--; tok = strtok(NULL, "/"); continue; }
        components[compc++] = tok;
        tok = strtok(NULL, "/");
    }
    size_t pos = 0;
    if (joined[0] == '/') { if (pos + 1 < outsz) out[pos++] = '/'; }
    for (size_t i = 0; i < compc; ++i) {
        size_t len = strlen(components[i]);
        if (pos + len + 1 < outsz) { memcpy(out + pos, components[i], len); pos += len; out[pos++] = '/'; }
        else break;
    }
    if (pos == 0) { if (pos + 1 < outsz) { out[pos++] = '.'; out[pos] = '\0'; } else out[outsz-1] = '\0'; }
    else { if (pos < outsz) out[pos-1] = '\0'; else out[outsz-1] = '\0'; }
}

// 打开文件（封装 fopen），失败返回 NULL
FILE* openFile(const char* path, const char* mode) {
    if (!path || !mode) return NULL;
    return fopen(path, mode);
}

// 关闭文件（封装 fclose）
void closeFile(FILE* f) {
    if (f) fclose(f);
}

// 读取整个文件为 malloc 分配的字符串，调用者负责 free
char* readFileToStr(const char* path) {
    if (!path) return NULL;
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)len, f);
    buf[r] = '\0';
    fclose(f);
    return buf;
}

// 复制文件（简单实现）
bool copyFile(const char* src, const char* dst) {
    if (!src || !dst) return false;
    FILE *fs = fopen(src, "rb");
    if (!fs) return false;
    FILE *fd = fopen(dst, "wb");
    if (!fd) { fclose(fs); return false; }
    char buf[4096]; size_t n;
    while ((n = fread(buf,1,sizeof(buf),fs)) > 0) fwrite(buf,1,n,fd);
    fclose(fs); fclose(fd);
    return true;
}

// 删除文件封装
bool removeFile(const char* path) {
    if (!path) return false;
    return remove(path) == 0;
}
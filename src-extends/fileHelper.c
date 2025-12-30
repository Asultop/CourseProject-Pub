#include "fileHelper.h"
#include <string.h>
#include <limits.h>

bool fileExists(const char* filename){
    FILE* file = fopen(filename, "r");
    if(file){
        fclose(file);
        return true;
    }
    return false;
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
    FILE* file = fopen(filename, "w");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}

// Join base directory and relative path, normalize '.' and '..'
void joinBasedirAndRel(const char *basedir, const char *rel, char *out, size_t outsz) {
    if (!out || outsz == 0) return;
    out[0] = '\0';
    if (!rel || rel[0] == '\0') {
        if (basedir) strncpy(out, basedir, outsz-1);
        out[outsz-1] = '\0';
        return;
    }
    // If rel is absolute, normalize rel
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

    // join basedir + rel
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
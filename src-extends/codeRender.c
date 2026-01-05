#define _XOPEN_SOURCE 700  // 启用POSIX扩展，兼容
#include "codeRender.h"
#include "chineseSupport.h"
#include "screenManager.h"
#include "fileHelper.h"
#include <libgen.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdalign.h>      // 对齐头文件
#include <stdnoreturn.h>   //  noreturn头文件
#include <errno.h>         // 错误码

// ========== C/C++关键字列表（包含新增关键字） ==========
static const char* const cpp_keywords[] = {
    // C基础关键字
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    // C++扩展关键字
    "bool", "catch", "class", "const_cast", "delete", "dynamic_cast", "explicit",
    "false", "friend", "inline", "mutable", "namespace", "new", "operator", "private",
    "protected", "public", "reinterpret_cast", "static_cast", "template", "this",
    "throw", "true", "try", "typeid", "typename", "using", "virtual", "wchar_t",
    // 新增关键字
    "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
    "_Imaginary", "_Noreturn", "alignas", "alignof", "atomic_bool",
    "atomic_char", "atomic_int", "constexpr", "decltype", "nullptr",
    "static_assert", "thread_local",
    NULL
};

// ========== 内部工具函数声明（规范） ==========
static int is_cpp_keyword(const char* restrict token);
static int is_cpp_type(const char* restrict token);
static int is_number(const char* restrict token);
static int is_preprocessor(const char* restrict line, int pos);
static int is_operator(char c);
static int is_variable(const char* restrict token);  // 新增：变量判断
static void render_cpp_line(const char* restrict line, int* restrict in_multi_comment);
static char * find_top_level_semicolon(char *start);
// 已知类型集合（从 include 文件中收集）
static char ** known_types = NULL;
static size_t known_types_count = 0;
static size_t known_types_cap = 0;

static void clear_known_types(void) {
    if (!known_types) return;
    for (size_t i = 0; i < known_types_count; ++i) free(known_types[i]);
    free(known_types);
    known_types = NULL; known_types_count = 0; known_types_cap = 0;
}

static int add_known_type(const char *name) {
    if (!name || name[0] == '\0') return 0;
    for (size_t i = 0; i < known_types_count; ++i) if (strcmp(known_types[i], name) == 0) return 0;
    if (known_types_count + 1 >= known_types_cap) {
        size_t nc = (known_types_cap == 0) ? 64 : known_types_cap * 2;
        char **tmp = (char**)realloc(known_types, nc * sizeof(char*));
        if (!tmp) return 0;
        known_types = tmp; known_types_cap = nc;
    }
    known_types[known_types_count] = (char*)malloc(strlen(name) + 1);
    if (!known_types[known_types_count]) return 0;
    strcpy(known_types[known_types_count], name);
    known_types_count++;
    return 1;
}

static int is_known_type(const char *name) {
    if (!name) return 0;
    for (size_t i = 0; i < known_types_count; ++i) if (strcmp(known_types[i], name) == 0) return 1;
    return 0;
}


static void collect_types_from_file(const char *path) {
    if (!path) return;
    char *content = readFileToStr(path);
    if (!content) return;

    // 1) 找 typedef ... ; 并取分号前最后一个标识符
    char *p = content;
    while ((p = strstr(p, "typedef")) != NULL) {
        char *semi = find_top_level_semicolon(p);
        if (!semi) break;
        char *q = semi - 1;
        while (q > p && isspace((unsigned char)*q)) q--;
        char *end = q;
        while (end > p && (isalnum((unsigned char)*(end-1)) || *(end-1) == '_' || *(end-1) == '*')) end--;
        while (*end == '*' || isspace((unsigned char)*end)) end++;
        if (end > q) { p = semi + 1; continue; }
        size_t len = (size_t)(q - end + 1);
        if (len > 0 && len < 256) {
            char id[256]; size_t j=0; char *r2=end;
            while (r2<=q && j+1<sizeof(id)) { if (*r2 != '*') id[j++]=*r2; r2++; }
            id[j]='\0';
            while (j>0 && isspace((unsigned char)id[j-1])) id[--j]='\0';
            char *last = NULL; char *tok = strtok(id, " \t\n\r");
            while (tok) { last = tok; tok = strtok(NULL, " \t\n\r"); }
            if (last && last[0]) add_known_type(last);
        }
        p = semi + 1;
    }

    // 2) 找 struct/class/enum NAME
    p = content;
    while ((p = strstr(p, "struct")) != NULL) {
        // 确保前面是空白或行首
        if (p == content || !isalnum((unsigned char)*(p-1))) {
            char *q = p + 6; if (*q == '\0') break;
            if (isspace((unsigned char)*q)) {
                while (*q && isspace((unsigned char)*q)) q++;
                char id[256]; size_t oi=0;
                while (*q && (isalnum((unsigned char)*q) || *q == '_') && oi+1<sizeof(id)) id[oi++]=*q++;
                id[oi]='\0'; if (oi>0) add_known_type(id);
            }
        }
        p += 6;
    }
    p = content;
    while ((p = strstr(p, "class")) != NULL) {
        char *q = p + 5; if (*q == '\0') break;
        if (isspace((unsigned char)*q)) { while (*q && isspace((unsigned char)*q)) q++; char id[256]; size_t oi=0; while (*q && (isalnum((unsigned char)*q) || *q == '_') && oi+1<sizeof(id)) id[oi++]=*q++; id[oi]='\0'; if (oi>0) add_known_type(id); }
        p += 5;
    }
    p = content;
    while ((p = strstr(p, "enum")) != NULL) {
        char *q = p + 4; if (*q == '\0') break;
        if (isspace((unsigned char)*q)) { while (*q && isspace((unsigned char)*q)) q++; char id[256]; size_t oi=0; while (*q && (isalnum((unsigned char)*q) || *q == '_') && oi+1<sizeof(id)) id[oi++]=*q++; id[oi]='\0'; if (oi>0) add_known_type(id); }
        p += 4;
    }

    // 3) 找 using NAME = ... ;
    p = content;
    while ((p = strstr(p, "using")) != NULL) {
        char *q = p + 5; if (*q == '\0') break;
        if (isspace((unsigned char)*q)) { while (*q && isspace((unsigned char)*q)) q++; char id[256]; size_t oi=0; while (*q && (isalnum((unsigned char)*q) || *q == '_') && oi+1<sizeof(id)) id[oi++]=*q++; id[oi]='\0'; if (oi>0) add_known_type(id); }
        p += 5;
    }

    free(content);
}

// 递归解析 #include "..." 并收集包含文件中的类型定义，避免循环包含
static void collect_types_from_includes_recursive(const char *file, char ***visited_files, size_t *visited_count, size_t *visited_cap) {
    if (!file) return;
    // 检查是否已访问
    for (size_t i = 0; i < *visited_count; ++i) {
        if (strcmp((*visited_files)[i], file) == 0) return;
    }
    // 记录为已访问
    if (*visited_count + 1 >= *visited_cap) {
        size_t nc = (*visited_cap == 0) ? 64 : (*visited_cap) * 2;
        char **tmp = (char**)realloc(*visited_files, nc * sizeof(char*));
        if (!tmp) return;
        *visited_files = tmp; *visited_cap = nc;
    }
    (*visited_files)[*visited_count] = strdup(file);
    (*visited_count)++;

        // 读取整个文件内容并在内存中查找 #include "..." 模式
        char *content = readFileToStr(file);
        if (!content) return;
        char *dup = strdup(file);
        char *basedir = dirname(dup);
        char *p = content;
        while ((p = strstr(p, "#include")) != NULL) {
            char *q = strchr(p, '"');
            if (!q) { p += 8; continue; }
            char *r = strchr(q+1, '"');
            if (!r) { p = q + 1; continue; }
            size_t plen = (size_t)(r - q - 1);
            char rel[1024];
            if (plen >= sizeof(rel)) plen = sizeof(rel) - 1;
            memcpy(rel, q+1, plen);
            rel[plen] = '\0';
            char normalized[4096];
            joinBasedirAndRel(basedir, rel, normalized, sizeof(normalized));
            char incpath[4096]; strncpy(incpath, normalized, sizeof(incpath)-1); incpath[sizeof(incpath)-1] = '\0';
            collect_types_from_file(incpath);
            collect_types_from_includes_recursive(incpath, visited_files, visited_count, visited_cap);
            p = r + 1;
        }
        free(content);
        free(dup);
}

static void collect_types_from_includes(const char *file) {
    char **visited = NULL; size_t vcount = 0, vcap = 0;
    collect_types_from_includes_recursive(file, &visited, &vcount, &vcap);
    if (visited) {
        for (size_t i = 0; i < vcount; ++i) free(visited[i]);
        free(visited);
    }
}



// 已迁移到 fileHelper 的 readFileToStr
static char * find_top_level_semicolon(char *start) {
    char *i = start;
    int brace = 0;
    while (*i) {
        if (*i == '"' || *i == '\'') {
            char quote = *i; i++;
            while (*i) {
                if (*i == '\\') { i += 2; continue; }
                if (*i == quote) { i++; break; }
                i++;
            }
            continue;
        }
        if (*i == '/' && *(i+1) == '/') { i += 2; while (*i && *i != '\n') i++; continue; }
        if (*i == '/' && *(i+1) == '*') { i += 2; while (*i && !(*i == '*' && *(i+1) == '/')) i++; if (*i) i += 2; continue; }
        if (*i == '{') { brace++; i++; continue; }
        if (*i == '}') { if (brace > 0) brace--; i++; continue; }
        if (*i == ';' && brace == 0) return i;
        i++;
    }
    return NULL;
}
static _Noreturn void print_error(const char* restrict msg);

// ========== 内部工具函数实现 ==========
static _Noreturn void print_error(const char* restrict msg) {
    fprintf(stderr, "[CodeRender错误] %s\n", msg);
    fflush(stderr);
    exit(EXIT_FAILURE);  // 标准退出码
}

static int is_cpp_keyword(const char* restrict token) {
    if (token == NULL || *token == '\0') return 0;

    // sizeof...计算数组长度，避免魔法数字
    const size_t kw_count = (sizeof(cpp_keywords) / sizeof(cpp_keywords[0])) - 1;
    for (size_t i = 0; i < kw_count; ++i) {
        if (strcmp(token, cpp_keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_cpp_type(const char* restrict token) {
    static const char* const types[] = {
        "char", "short", "int", "long", "float", "double", "bool", "void",
        "struct", "union", "enum", "class", "typedef", "wchar_t", "_Bool", NULL
    };

    const size_t type_count = (sizeof(types) / sizeof(types[0])) - 1;
    for (size_t i = 0; i < type_count; ++i) {
        if (strcmp(token, types[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_number(const char* restrict token) {
    if (*token == '\0') return 0;

    int has_digit = 0;
    int has_dot = 0;
    const int is_hex = (*token == '0' && (*(token + 1) == 'x' || *(token + 1) == 'X'));
    
    // 跳过十六进制前缀
    size_t start = is_hex ? 2 : 0;
    if (start >= strlen(token)) return 0;

    // 跳过数字后缀（ll/L/U等）
    size_t end = strlen(token);
    while (end > start) {
        const char c = token[end - 1];
        if (c == 'l' || c == 'L' || c == 'u' || c == 'U') {
            --end;
        } else {
            break;
        }
    }

    // 检查数字有效性
    for (size_t i = start; i < end; ++i) {
        const unsigned char uc = (unsigned char)token[i];  // 避免符号扩展
        if (isdigit(uc) || (is_hex && isxdigit(uc))) {
            has_digit = 1;
            continue;
        }
        if (uc == '.' && !has_dot && !is_hex) {
            has_dot = 1;
            continue;
        }
        return 0;
    }
    return has_digit;
}

// 新增：判断是否为变量（非关键字/非类型/非数字的合法标识符）
static int is_variable(const char* restrict token) {
    if (token == NULL || *token == '\0') return 0;
    
    // 排除关键字、类型、数字
    if (is_cpp_keyword(token) || is_cpp_type(token) || is_number(token)) {
        return 0;
    }
    
    // 变量名规则：首字符为字母/下划线，后续为字母/数字/下划线
    if (!isalpha((unsigned char)token[0]) && token[0] != '_') {
        return 0;
    }
    
    for (size_t i = 1; token[i] != '\0'; ++i) {
        if (!isalnum((unsigned char)token[i]) && token[i] != '_') {
            return 0;
        }
    }
    return 1;
}

static int is_preprocessor(const char* restrict line, int pos) {
    if (line[pos] != '#') return 0;
    for (int i = 0; i < pos; ++i) {
        if (!isspace((unsigned char)line[i])) return 0;
    }
    return 1;
}

static int is_operator(char c) {
    static const char ops[] = "+-*/%=<>!&|^~;:,.()[]{}";
    return strchr(ops, c) != NULL;
}

static void render_cpp_line(const char* restrict line, int* restrict in_multi_comment) {
    // 使用 processRawChar 将行拆分为显示单元，保证中文/全角/ANSI序列作为单个单元
    char ** units = processRawChar(line);
    if(!units) return;

    int in_single_comment = 0;
    int in_string = 0;
    char quote = '\0';
    char tokenbuf[512]; tokenbuf[0] = '\0';

    // 输出缓冲区，累积所有输出，最后一次性打印
    char buf[8192];
    size_t off = 0;
    #define BUF_APPEND(...) do { \
        int _n = snprintf(buf + off, sizeof(buf) - off, __VA_ARGS__); \
        if (_n > 0 && off + (size_t)_n < sizeof(buf)) off += (size_t)_n; \
    } while(0)

    // 遍历直到遇到 "EOL" 标记（不包含 EOL 本身）
    for (size_t i = 0; units[i] && strcmp(units[i], "EOL") != 0; ++i) {
        const char *cur = units[i];
        const char *next = units[i+1];
        if (!cur) break;

        // 快捷判断单字节ASCII字符
        int is_ascii = (cur[0] != '\0' && (unsigned char)cur[0] < 0x80 && cur[1] == '\0');
        char c = is_ascii ? cur[0] : '\0';

        // 多行注释状态
        if (*in_multi_comment) {
            BUF_APPEND("%s%s%s", COLOR_COMMENT, cur, COLOR_DEFAULT);
            // 检查结束 */（下一个单元为 '/'）
            if (is_ascii && c == '*' && next && next[0] == '/' && next[1] == '\0') {
                BUF_APPEND("%s", units[++i]);
                *in_multi_comment = 0;
                BUF_APPEND(COLOR_DEFAULT);
            }
            continue;
        } else if (in_single_comment) {
            BUF_APPEND("%s%s%s", COLOR_COMMENT, cur, COLOR_DEFAULT);
            continue;
        } else if (in_string) {
            BUF_APPEND("%s%s%s", COLOR_STRING, cur, COLOR_DEFAULT);
            // 处理转义：如果为 '\\' 且下一个存在，则打印下一个并跳过
            if (is_ascii && c == '\\' && next) {
                BUF_APPEND("%s%s%s", COLOR_STRING, next, COLOR_DEFAULT);
                i++;
            }
            if (is_ascii && c == quote) {
                in_string = 0;
                BUF_APPEND(COLOR_DEFAULT);
            }
            continue;
        }

        // 注释/预处理/字符串起始检测 --- 以单元为基础
        if (is_ascii) {
            // 多行注释起始 /*
            if (c == '/' && next && next[0] == '*' && next[1] == '\0') {
                //  刷新 tokenbuf
                if (tokenbuf[0]) { BUF_APPEND("%s", tokenbuf); tokenbuf[0] = '\0'; }
                BUF_APPEND("%s/*", COLOR_COMMENT);
                *in_multi_comment = 1;
                i++; // 跳过 '*'
                continue;
            }
            // 单行注释起始 //
            else if (c == '/' && next && next[0] == '/' && next[1] == '\0') {
                if (tokenbuf[0]) { BUF_APPEND("%s", tokenbuf); tokenbuf[0] = '\0'; }
                BUF_APPEND("%s//", COLOR_COMMENT);
                in_single_comment = 1;
                i++;
                continue;
            }
            // 预处理指令 (#) — 判断行首非空白
            else if (c == '#') {
                // 检查前面是否只有空白
                int only_ws = 1;
                // 扫描前面的单元
                for (size_t k = 0; k < i; ++k) { if (!(units[k][0] == ' ' && units[k][1] == '\0') && !(units[k][0] == '\t' && units[k][1] == '\0')) { only_ws = 0; break; } }
                if (only_ws) {
                    if (tokenbuf[0]) { BUF_APPEND("%s", tokenbuf); tokenbuf[0] = '\0'; }
                    BUF_APPEND("%s", COLOR_PREPROCESSOR);
                    // print remaining units until EOL
                    for (size_t k = i; units[k] && strcmp(units[k], "EOL") != 0; ++k) {
                        BUF_APPEND("%s", units[k]);
                    }
                    BUF_APPEND(COLOR_DEFAULT);
                    break;
                }
            }
            // 字符串起始
            else if (c == '"' || c == '\'') {
                if (tokenbuf[0]) { BUF_APPEND("%s", tokenbuf); tokenbuf[0] = '\0'; }
                BUF_APPEND("%s%c", COLOR_STRING, c);
                in_string = 1; quote = c;
                continue;
            }
        }

        // 单词分割逻辑：ASCII 字母数字或 '_' 认为是标识符的一部分
        if (is_ascii && (isalnum((unsigned char)c) || c == '_')) {
            size_t tb = strlen(tokenbuf);
            if (tb + 2 < sizeof(tokenbuf)) {
                tokenbuf[tb] = c; tokenbuf[tb+1] = '\0';
            }
            continue;
        } else {
            // flush tokenbuf
            if (tokenbuf[0]) {
                // 判断类型并高亮
                if (is_cpp_type(tokenbuf) || is_known_type(tokenbuf)) {
                    BUF_APPEND("%s%s%s", COLOR_TYPE, tokenbuf, COLOR_DEFAULT);
                } else if (is_cpp_keyword(tokenbuf)) {
                    BUF_APPEND("%s%s%s", COLOR_KEYWORD, tokenbuf, COLOR_DEFAULT);
                } else if (is_number(tokenbuf)) {
                    BUF_APPEND("%s%s%s", COLOR_NUMBER, tokenbuf, COLOR_DEFAULT);
                } else if (is_variable(tokenbuf)) {
                    BUF_APPEND("%s%s%s", COLOR_VARIABLE, tokenbuf, COLOR_DEFAULT);
                } else {
                    BUF_APPEND("%s", tokenbuf);
                }
                tokenbuf[0] = '\0';
            }
            // 输出当前单元
            if (is_ascii) {
                if (is_operator(c)) {
                    BUF_APPEND("%s%c%s", COLOR_OPERATOR, c, COLOR_DEFAULT);
                } else {
                    BUF_APPEND("%c", c);
                }
            } else {
                // 多字节或转义序列直接输出（不做运算符判断）
                BUF_APPEND("%s", cur);
            }
        }
    }

    #undef BUF_APPEND


    // 一次性输出整个缓冲区
    if (off > 0) {
        printf("║ %s", buf);
        // printConsole(buf, MARGIN_LEFT);
    }

    freeProcessedChars(units);
}

// ========== 核心函数实现（标准） ==========
int codeRender_worker(const char* restrict file) {
    // 严格空指针检查
    if (file == NULL) {
        fprintf(stderr, "[CodeRender错误] 文件路径为NULL\n");
        return -1;
    }

    // 安全打开文件（兼容MSVC fopen_s和GCC fopen）
    FILE* fp = NULL;
#if defined(_MSC_VER)  // Windows MSVC编译器（ Annex K安全函数）
    const errno_t err = fopen_s(&fp, file, "r");
    if (err != 0) {
        fprintf(stderr, "[CodeRender错误] 无法打开文件 %s（错误码：%d）\n", file, err);
        return -1;
    }
#else  // Linux/macOS GCC/Clang
        fp = openFile(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "[CodeRender错误] 无法打开文件 %s：%s\n", file, strerror(errno));
        return -1;
    }
#endif

    // 静态断言验证缓冲区大小（取消注释启用）
    // static_assert(4096 >= 1024, "行缓冲区过小，无法处理长行");
    char line[4096];
    int in_multi_comment = 0;  // 跨行长注释状态

    // 收集当前文件及其 include 的类型定义
    clear_known_types();
    collect_types_from_file(file); // 收集文件本身（支持多行 typedef/struct/using）
    collect_types_from_includes(file);
    // DEBUG: 输出已收集的类型，便于诊断（开发时用）
    (void)known_types_count;

    // 逐行读取并渲染
    while (fgets(line, (int)sizeof(line), fp) != NULL) {
        render_cpp_line(line, &in_multi_comment);
    }

    // 检查文件读取错误
    if (ferror(fp)) {
        fprintf(stderr, "[CodeRender错误] 读取文件 %s 时发生IO错误\n", file);
        fclose(fp);
        return -1;
    }

    // 处理未闭合的多行注释
    if (in_multi_comment) {
        printf("%s%s", COLOR_COMMENT, "/* 未闭合的多行注释 */" COLOR_DEFAULT);
    }

    // 安全关闭文件，避免野指针
    if (fp != NULL) {
            closeFile(fp);
        fp = NULL;
    }

    fflush(stdout);  // 强制刷新输出缓冲区
    return 0;
}

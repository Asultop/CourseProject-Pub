#define _XOPEN_SOURCE 700  // 启用POSIX扩展，兼容C11
#include "codeRender.h"
#include "chineseSupport.h"
#include <libgen.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdalign.h>      // C11对齐头文件
#include <stdnoreturn.h>   // C11 noreturn头文件
#include <errno.h>         // C11错误码

// ========== C/C++关键字列表（包含C11新增关键字） ==========
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
    // C11新增关键字
    "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
    "_Imaginary", "_Noreturn", "alignas", "alignof", "atomic_bool",
    "atomic_char", "atomic_int", "constexpr", "decltype", "nullptr",
    "static_assert", "thread_local",
    NULL
};

// ========== 内部工具函数声明（C11规范） ==========
static int is_cpp_keyword(const char* restrict token);
static int is_cpp_type(const char* restrict token);
static int is_number(const char* restrict token);
static int is_preprocessor(const char* restrict line, int pos);
static int is_operator(char c);
static int is_variable(const char* restrict token);  // 新增：变量判断
static void render_cpp_line(const char* restrict line, int* restrict in_multi_comment);
static void join_basedir_and_rel(const char *basedir, const char *rel, char *out, size_t outsz);
static char * read_file_to_str_alloc(const char *path);
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
    char *content = read_file_to_str_alloc(path);
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

    FILE *f = fopen(file, "r");
    if (!f) return;
    char line[4096];
    // 获取基目录
    char *dup = strdup(file);
    char *basedir = dirname(dup);
    while (fgets(line, sizeof(line), f)) {
        char *inc = strstr(line, "#include");
        if (!inc) continue;
        char *q = strchr(line, '"');
        if (!q) continue;
        char *r = strchr(q+1, '"');
        if (!r) continue;
        size_t plen = (size_t)(r - q - 1);
        char incpath[PATH_MAX];
        // 构造相对路径
        // q+1 到 r-1 是相对路径
        char rel[1024];
        if (plen >= (int)sizeof(rel)) plen = (int)sizeof(rel) - 1;
        memcpy(rel, q+1, (size_t)plen);
        rel[plen] = '\0';
        // 规范化路径
        char normalized[PATH_MAX];
        // join_basedir_and_rel 在下面定义
        join_basedir_and_rel(basedir, rel, normalized, sizeof(normalized));
        strncpy(incpath, normalized, sizeof(incpath)-1);
        incpath[sizeof(incpath)-1] = '\0';
        (void)0;
        collect_types_from_file(incpath);
        (void)0;
        collect_types_from_includes_recursive(incpath, visited_files, visited_count, visited_cap);
    }
    free(dup);
    fclose(f);
}

static void collect_types_from_includes(const char *file) {
    char **visited = NULL; size_t vcount = 0, vcap = 0;
    collect_types_from_includes_recursive(file, &visited, &vcount, &vcap);
    if (visited) {
        for (size_t i = 0; i < vcount; ++i) free(visited[i]);
        free(visited);
    }
}

static void join_basedir_and_rel(const char *basedir, const char *rel, char *out, size_t outsz) {
    if (!out || outsz == 0) return;
    out[0] = '\0';
    if (!rel || rel[0] == '\0') {
        if (basedir) strncpy(out, basedir, outsz-1);
        out[outsz-1] = '\0';
        return;
    }
    (void)basedir; (void)rel;
    // If rel is absolute, normalize rel alone
    if (rel[0] == '/') {
        // 规范化绝对路径 rel
        char tmp[PATH_MAX]; strncpy(tmp, rel, sizeof(tmp)-1); tmp[sizeof(tmp)-1] = '\0';
        // 继续使用 tmp 作为输入进行规范化
        const char *src = tmp;
        // 分割并规范化路径组件
        char *stack[PATH_MAX]; size_t top = 0;
        char buf[PATH_MAX]; strncpy(buf, src, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
        char *tok = strtok(buf, "/");
        while (tok) {
            if (strcmp(tok, ".") == 0) { tok = strtok(NULL, "/"); continue; }
            if (strcmp(tok, "..") == 0) { if (top > 0) top--; tok = strtok(NULL, "/"); continue; }
            stack[top++] = tok;
            tok = strtok(NULL, "/");
        }
        // 重构路径
        size_t pos = 0;
        if (pos + 1 < outsz) out[pos++] = '/';
        for (size_t i = 0; i < top; ++i) {
            size_t len = strlen(stack[i]);
            if (pos + len + 1 < outsz) {
                memcpy(out + pos, stack[i], len); pos += len; out[pos++] = '/';
            } else break;
        }
        if (pos > 0 && pos < outsz) out[pos-1] = '\0'; else if (pos < outsz) out[pos] = '\0';
        return;
    }

    // 基于 basedir 和 rel 连接
    char joined[PATH_MAX];
    if (basedir && basedir[0] != '\0') snprintf(joined, sizeof(joined), "%s/%s", basedir, rel);
    else snprintf(joined, sizeof(joined), "%s", rel);
    (void)joined;

    // 规范化 joined 路径
    char buf[PATH_MAX]; strncpy(buf, joined, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
    // 使用 strtok 分割组件，保证每个组件以 '\0' 结尾
    char *components[PATH_MAX]; size_t compc = 0;
    char *tok = strtok(buf, "/");
    while (tok) {
        if (strcmp(tok, ".") == 0) { tok = strtok(NULL, "/"); continue; }
        if (strcmp(tok, "..") == 0) { if (compc > 0) compc--; tok = strtok(NULL, "/"); continue; }
        components[compc++] = tok;
        tok = strtok(NULL, "/");
    }
    // 重构路径
    size_t pos = 0;
    (void)compc;
    // 检查是否为绝对路径
    if (joined[0] == '/') {
        if (pos + 1 < outsz) out[pos++] = '/';
    }
    for (size_t i = 0; i < compc; ++i) {
        size_t len = strlen(components[i]);
        if (pos + len + 1 < outsz) {
            memcpy(out + pos, components[i], len); pos += len; out[pos++] = '/';
        } else break;
    }
    if (pos == 0) {
        // empty -> current dir
        if (pos + 1 < outsz) { out[pos++] = '.'; out[pos] = '\0'; }
        else out[outsz-1] = '\0';
    } else {
        if (pos < outsz) out[pos-1] = '\0'; else out[outsz-1] = '\0';
    }
    (void)out;
}

// Read entire file into a malloc'd buffer, null-terminated. Caller must free.
static char * read_file_to_str_alloc(const char *path) {
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

// Find semicolon at top-level (not inside braces), skipping strings and comments.
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
    exit(EXIT_FAILURE);  // C11标准退出码
}

static int is_cpp_keyword(const char* restrict token) {
    if (token == NULL || *token == '\0') return 0;

    // C11：sizeof...计算数组长度，避免魔法数字
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
        const unsigned char uc = (unsigned char)token[i];  // C11：避免符号扩展
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
            printf("%s%s%s", COLOR_COMMENT, cur, COLOR_DEFAULT);
            // 检查结束 */（下一个单元为 '/'）
            if (is_ascii && c == '*' && next && next[0] == '/' && next[1] == '\0') {
                printf("%s", units[++i]);
                *in_multi_comment = 0;
                printf(COLOR_DEFAULT);
            }
            continue;
        }

        if (in_single_comment) {
            printf("%s%s%s", COLOR_COMMENT, cur, COLOR_DEFAULT);
            continue;
        }

        if (in_string) {
            printf("%s%s%s", COLOR_STRING, cur, COLOR_DEFAULT);
            // 处理转义：如果为 '\\' 且下一个存在，则打印下一个并跳过
            if (is_ascii && c == '\\' && next) {
                printf("%s%s%s", COLOR_STRING, next, COLOR_DEFAULT);
                i++;
            }
            if (is_ascii && c == quote) {
                in_string = 0;
                printf(COLOR_DEFAULT);
            }
            continue;
        }

        // 注释/预处理/字符串起始检测 --- 以单元为基础
        if (is_ascii) {
            // 多行注释起始 /*
            if (c == '/' && next && next[0] == '*' && next[1] == '\0') {
                //  刷新 tokenbuf
                if (tokenbuf[0]) { printf("%s", tokenbuf); tokenbuf[0] = '\0'; }
                printf("%s/*", COLOR_COMMENT);
                *in_multi_comment = 1;
                i++; // 跳过 '*'
                continue;
            }
            // 单行注释起始 //
            if (c == '/' && next && next[0] == '/' && next[1] == '\0') {
                if (tokenbuf[0]) { printf("%s", tokenbuf); tokenbuf[0] = '\0'; }
                printf("%s//", COLOR_COMMENT);
                in_single_comment = 1;
                i++;
                continue;
            }
            // 预处理指令 (#) — 判断行首非空白
            if (c == '#') {
                // 检查前面是否只有空白
                int only_ws = 1;
                // 扫描前面的单元
                for (size_t k = 0; k < i; ++k) { if (!(units[k][0] == ' ' && units[k][1] == '\0') && !(units[k][0] == '\t' && units[k][1] == '\0')) { only_ws = 0; break; } }
                if (only_ws) {
                    if (tokenbuf[0]) { printf("%s", tokenbuf); tokenbuf[0] = '\0'; }
                    printf("%s", COLOR_PREPROCESSOR);
                    // print remaining units until EOL
                    for (size_t k = i; units[k] && strcmp(units[k], "EOL") != 0; ++k) {
                        printf("%s", units[k]);
                    }
                    printf(COLOR_DEFAULT);
                    break;
                }
            }
            // 字符串起始
            if (c == '"' || c == '\'') {
                if (tokenbuf[0]) { printf("%s", tokenbuf); tokenbuf[0] = '\0'; }
                printf("%s%c", COLOR_STRING, c);
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
                if (is_cpp_type(tokenbuf) || is_known_type(tokenbuf)) printf("%s%s%s", COLOR_TYPE, tokenbuf, COLOR_DEFAULT);
                else if (is_cpp_keyword(tokenbuf)) printf("%s%s%s", COLOR_KEYWORD, tokenbuf, COLOR_DEFAULT);
                else if (is_number(tokenbuf)) printf("%s%s%s", COLOR_NUMBER, tokenbuf, COLOR_DEFAULT);
                else if (is_variable(tokenbuf)) printf("%s%s%s", COLOR_VARIABLE, tokenbuf, COLOR_DEFAULT);
                else printf("%s", tokenbuf);
                tokenbuf[0] = '\0';
            }
            // 输出当前单元
            if (is_ascii) {
                if (is_operator(c)) printf("%s%c%s", COLOR_OPERATOR, c, COLOR_DEFAULT);
                else printf("%c", c);
            } else {
                // 多字节或转义序列直接输出（不做运算符判断）
                printf("%s", cur);
            }
        }
    }

    freeProcessedChars(units);
}

// ========== 核心函数实现（C11标准） ==========
int codeRender_worker(const char* restrict file) {
    // C11：严格空指针检查
    if (file == NULL) {
        fprintf(stderr, "[CodeRender错误] 文件路径为NULL\n");
        return -1;
    }

    // C11：安全打开文件（兼容MSVC fopen_s和GCC fopen）
    FILE* fp = NULL;
#if defined(_MSC_VER)  // Windows MSVC编译器（C11 Annex K安全函数）
    const errno_t err = fopen_s(&fp, file, "r");
    if (err != 0) {
        fprintf(stderr, "[CodeRender错误] 无法打开文件 %s（错误码：%d）\n", file, err);
        return -1;
    }
#else  // Linux/macOS GCC/Clang
    fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "[CodeRender错误] 无法打开文件 %s：%s\n", file, strerror(errno));
        return -1;
    }
#endif

    // C11：静态断言验证缓冲区大小（取消注释启用）
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

    // C11：检查文件读取错误
    if (ferror(fp)) {
        fprintf(stderr, "[CodeRender错误] 读取文件 %s 时发生IO错误\n", file);
        fclose(fp);
        return -1;
    }

    // 处理未闭合的多行注释
    if (in_multi_comment) {
        printf("%s%s", COLOR_COMMENT, "/* 未闭合的多行注释 */" COLOR_DEFAULT);
    }

    // C11：安全关闭文件，避免野指针
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    fflush(stdout);  // C11：强制刷新输出缓冲区
    return 0;
}

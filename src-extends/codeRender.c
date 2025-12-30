#define _XOPEN_SOURCE 700  // 启用POSIX扩展，兼容C11
#include "codeRender.h"
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
    int in_single_comment = 0;  // 单行注释状态
    int in_string = 0;          // 字符串状态
    char quote = '\0';          // 字符串引号类型（' 或 "）
    int token_start = -1;       // 单词起始位置
    char token[256] = {0};      // 临时单词缓存
    const size_t len = strlen(line);

    for (size_t i = 0; i <= len; ++i) {
        const char c = (i < len) ? line[i] : '\0';

        // ========== 多行注释处理 ==========
        if (*in_multi_comment) {
            printf("%s%c", COLOR_COMMENT, c);
            if (c == '*' && i + 1 < len && line[i + 1] == '/') {
                printf("%s%c", COLOR_COMMENT, line[++i]);
                *in_multi_comment = 0;
                printf(COLOR_DEFAULT);
            }
            continue;
        }

        // ========== 单行注释处理 ==========
        if (in_single_comment) {
            printf("%s%c", COLOR_COMMENT, c);
            continue;
        }

        // ========== 字符串处理 ==========
        if (in_string) {
            printf("%s%c", COLOR_STRING, c);
            // 处理转义字符
            if (c == '\\' && i + 1 < len) {
                printf("%s%c", COLOR_STRING, line[++i]);
                continue;
            }
            // 字符串结束
            if (c == quote) {
                in_string = 0;
                printf(COLOR_DEFAULT);
            }
            continue;
        }

        // ========== 注释/预处理/字符串起始检测 ==========
        if (i < len) {
            // 多行注释起始 /*
            if (c == '/' && line[i + 1] == '*') {
                // 输出缓存的token
                if (token_start != -1) {
                    const size_t token_len = i - token_start;
                    if (token_len < sizeof(token) - 1) {
                        strncpy(token, line + token_start, token_len);
                        token[token_len] = '\0';
                    }
                    token_start = -1;
                    if (is_cpp_type(token)) printf("%s%s%s", COLOR_TYPE, token, COLOR_DEFAULT);
                    else if (is_cpp_keyword(token)) printf("%s%s%s", COLOR_KEYWORD, token, COLOR_DEFAULT);
                    else if (is_number(token)) printf("%s%s%s", COLOR_NUMBER, token, COLOR_DEFAULT);
                    else if (is_variable(token)) printf("%s%s%s", COLOR_VARIABLE, token, COLOR_DEFAULT); // 变量高亮
                    else printf("%s", token);
                }
                printf("%s/*", COLOR_COMMENT);
                *in_multi_comment = 1;
                ++i;
                continue;
            }
            // 单行注释起始 //
            else if (c == '/' && line[i + 1] == '/') {
                if (token_start != -1) {
                    const size_t token_len = i - token_start;
                    if (token_len < sizeof(token) - 1) {
                        strncpy(token, line + token_start, token_len);
                        token[token_len] = '\0';
                    }
                    token_start = -1;
                    if (is_cpp_type(token)) printf("%s%s%s", COLOR_TYPE, token, COLOR_DEFAULT);
                    else if (is_cpp_keyword(token)) printf("%s%s%s", COLOR_KEYWORD, token, COLOR_DEFAULT);
                    else if (is_number(token)) printf("%s%s%s", COLOR_NUMBER, token, COLOR_DEFAULT);
                    else if (is_variable(token)) printf("%s%s%s", COLOR_VARIABLE, token, COLOR_DEFAULT); // 变量高亮
                    else printf("%s", token);
                }
                printf("%s//", COLOR_COMMENT);
                in_single_comment = 1;
                ++i;
                continue;
            }
            // 预处理指令 #
            else if (is_preprocessor(line, (int)i)) {
                if (token_start != -1) {
                    const size_t token_len = i - token_start;
                    if (token_len < sizeof(token) - 1) {
                        strncpy(token, line + token_start, token_len);
                        token[token_len] = '\0';
                    }
                    token_start = -1;
                    printf("%s", token);
                }
                // 渲染预处理指令（直到行尾）
                printf("%s", COLOR_PREPROCESSOR);
                while (i < len && line[i] != '\n') {
                    printf("%c", line[i++]);
                }
                printf(COLOR_DEFAULT);
                --i;
                continue;
            }
            // 字符串起始 " 或 '
            else if (c == '"' || c == '\'') {
                if (token_start != -1) {
                    const size_t token_len = i - token_start;
                    if (token_len < sizeof(token) - 1) {
                        strncpy(token, line + token_start, token_len);
                        token[token_len] = '\0';
                    }
                    token_start = -1;
                    if (is_cpp_type(token)) printf("%s%s%s", COLOR_TYPE, token, COLOR_DEFAULT);
                    else if (is_cpp_keyword(token)) printf("%s%s%s", COLOR_KEYWORD, token, COLOR_DEFAULT);
                    else if (is_number(token)) printf("%s%s%s", COLOR_NUMBER, token, COLOR_DEFAULT);
                    else if (is_variable(token)) printf("%s%s%s", COLOR_VARIABLE, token, COLOR_DEFAULT); // 变量高亮
                    else printf("%s", token);
                }
                printf("%s%c", COLOR_STRING, c);
                in_string = 1;
                quote = c;
                continue;
            }
        }

        // ========== 单词分割与高亮（核心：新增变量高亮） ==========
        if (isalnum((unsigned char)c) || c == '_') {
            if (token_start == -1) token_start = (int)i;
        } else {
            // 输出缓存的token
            if (token_start != -1) {
                const size_t token_len = i - token_start;
                if (token_len < sizeof(token) - 1) {
                    strncpy(token, line + token_start, token_len);
                    token[token_len] = '\0';
                    // 根据类型高亮（优先级：类型 > 关键字 > 数字 > 变量 > 默认）
                    if (is_cpp_type(token)) {
                        printf("%s%s%s", COLOR_TYPE, token, COLOR_DEFAULT);
                    } else if (is_cpp_keyword(token)) {
                        printf("%s%s%s", COLOR_KEYWORD, token, COLOR_DEFAULT);
                    } else if (is_number(token)) {
                        printf("%s%s%s", COLOR_NUMBER, token, COLOR_DEFAULT);
                    } else if (is_variable(token)) {  // 新增：变量高亮
                        printf("%s%s%s", COLOR_VARIABLE, token, COLOR_DEFAULT);
                    } else {
                        printf("%s", token);
                    }
                }
                token_start = -1;
            }
            // 输出当前字符（运算符高亮）
            if (i < len) {
                if (is_operator(c)) {
                    printf("%s%c%s", COLOR_OPERATOR, c, COLOR_DEFAULT);
                } else {
                    printf("%c", c);
                }
            }
        }
    }
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

// ========== 测试主函数（可选，编译时定义CODE_RENDER_TEST启用） ==========
#ifdef CODE_RENDER_TEST
int main(int argc, char* argv[]) {
    // C11：参数合法性检查
    if (argc != 2) {
        printf("C11 CodeRender 使用方式：%s <C/C++文件路径>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // C11：restrict限定指针
    const char* restrict file_path = argv[1];
    const int ret = codeRender_worker(file_path);

    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
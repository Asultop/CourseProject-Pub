
#include "chineseSupport.h"

/************************ 工具函数：UTF-8合法性检测 ************************/
int is_utf8_valid(const char *str) {
    int i = 0;
    int len = strlen(str);
    while (i < len) {
        unsigned char c = (unsigned char)str[i];
        if (c < 0x80) {  // 单字节ASCII
            i++;
        } else if (c < 0xC0) {  // 非法：单独的10xxxxxx字节
            return 0;
        } else if (c < 0xE0) {  // 双字节UTF-8
            if (i + 1 >= len || (unsigned char)str[i+1] < 0x80 || (unsigned char)str[i+1] > 0xBF) {
                return 0;
            }
            i += 2;
        } else if (c < 0xF0) {  // 三字节UTF-8
            if (i + 2 >= len || (unsigned char)str[i+1] < 0x80 || (unsigned char)str[i+1] > 0xBF ||
                (unsigned char)str[i+2] < 0x80 || (unsigned char)str[i+2] > 0xBF) {
                return 0;
            }
            i += 3;
        } else if (c < 0xF8) {  // 四字节UTF-8
            if (i + 3 >= len || (unsigned char)str[i+1] < 0x80 || (unsigned char)str[i+1] > 0xBF ||
                (unsigned char)str[i+2] < 0x80 || (unsigned char)str[i+2] > 0xBF ||
                (unsigned char)str[i+3] < 0x80 || (unsigned char)str[i+3] > 0xBF) {
                return 0;
            }
            i += 4;
        } else {  // 非法：0xF8及以上
            return 0;
        }
    }
    return 1;
}

/************************ 工具函数：检测字符串编码 ************************/
EncodingType detect_encoding(const char *str) {
    if (str == NULL || *str == '\0') {
        return ENCODING_UNKNOWN;
    }
    // 优先验证UTF-8合法性
    if (is_utf8_valid(str)) {
        return ENCODING_UTF8;
    }
    // 非UTF-8则判定为GBK（实际场景可增加GBK合法性检测）
    return ENCODING_GBK;
}

/************************ 工具函数：UTF-8转Unicode ************************/
unsigned int utf8_to_unicode(const unsigned char *utf8) {
    unsigned int unicode = 0;
    if ((utf8[0] & 0xF0) == 0xE0) {  // 三字节UTF-8
        unicode = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
    } else if ((utf8[0] & 0xF8) == 0xF0) {  // 四字节UTF-8（极少用）
        unicode = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) |
                  ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
    }
    return unicode;
}

/************************ 工具函数：判断是否为中文汉字（排除标点） ************************/
// UTF-8字符判断
int is_utf8_chinese_char(const unsigned char *c) {
    unsigned int unicode = utf8_to_unicode(c);
    return (unicode >= 0x4E00 && unicode <= 0x9FA5);  // 汉字Unicode范围
}

// GBK字符判断（GBK转Unicode简化版，仅判断汉字区间）
int is_gbk_chinese_char(const unsigned char *c) {
    // GBK汉字区：第一字节0xB0~0xF7，第二字节0xA1~0xFE
    return (c[0] >= 0xB0 && c[0] <= 0xF7) && (c[1] >= 0xA1 && c[1] <= 0xFE);
}

/************************ 统计UTF-8中文字符（仅汉字） ************************/
int count_utf8_chinese(const char *str) {
    int count = 0;
    int i = 0;
    int len = strlen(str);
    while (i < len) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0xE0 && c <= 0xEF) {  // 三字节UTF-8（中文候选）
            if (i + 2 < len && is_utf8_chinese_char((unsigned char *)&str[i])) {
                count++;
            }
            i += 3;
        } else if (c >= 0xC0 && c <= 0xDF) {  // 双字节UTF-8（非中文）
            i += 2;
        } else if (c >= 0xF0 && c <= 0xF7) {  // 四字节UTF-8（非中文）
            i += 4;
        } else {  // 单字节ASCII
            i++;
        }
    }
    return count;
}

/************************ 统计GBK中文字符（仅汉字） ************************/
int count_gbk_chinese(const char *str) {
    int count = 0;
    int i = 0;
    int len = strlen(str);
    while (i < len) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0x80) {  // 双字节GBK字符
            if (i + 1 < len && is_gbk_chinese_char((unsigned char *)&str[i])) {
                count++;
            }
            i += 2;
        } else {  // 单字节ASCII
            i++;
        }
    }
    return count;
}

/************************ 通用中文字符统计函数（自动识别编码） ************************/
int count_chinese(const char *str, EncodingType *detected_encoding) {
    if (str == NULL || *str == '\0') {
        if (detected_encoding != NULL) {
            *detected_encoding = ENCODING_UNKNOWN;
        }
        return 0;
    }

    // 检测编码
    EncodingType encoding = detect_encoding(str);
    if (detected_encoding != NULL) {
        *detected_encoding = encoding;
    }

    // 根据编码统计
    int count = 0;
    switch (encoding) {
        case ENCODING_UTF8:
            count = count_utf8_chinese(str);
            break;
        case ENCODING_GBK:
            count = count_gbk_chinese(str);
            break;
        default:
            count = 0;
            break;
    }
    return count;
}
// 判断UTF-8字符是否为中文/全角字符（计2）
// 支持: 中文汉字、全角标点、全角字母数字、日文平假名/片假名、韩文、
//       中文标点符号等双宽度字符
// 注意：上标/下标(⁰¹²³⁴⁵⁶⁷⁸⁹)、希腊字母、箭头、数学符号等通常是单宽度
int is_utf8_double_width(const unsigned char *c, int i, int len) {
    // 三字节UTF-8：中文/全角字符
    if ((c[i] >= 0xE0 && c[i] <= 0xEF) && (i + 2 < len)) {
        unsigned int unicode = utf8_to_unicode(&c[i]);
        // 中文汉字：0x4E00~0x9FFF（CJK统一汉字）
        if (unicode >= 0x4E00 && unicode <= 0x9FFF) return 1;
        // CJK扩展A：0x3400~0x4DBF
        if (unicode >= 0x3400 && unicode <= 0x4DBF) return 1;
        // 全角ASCII变体：0xFF01~0xFF5E（！到～的全角版本）
        if (unicode >= 0xFF01 && unicode <= 0xFF5E) return 1;
        // 全角空格：0x3000
        if (unicode == 0x3000) return 1;
        // 中文标点符号：0x3001~0x303F（CJK符号和标点，排除0x3000已处理）
        if (unicode >= 0x3001 && unicode <= 0x303F) return 1;
        // 日文平假名：0x3040~0x309F
        if (unicode >= 0x3040 && unicode <= 0x309F) return 1;
        // 日文片假名：0x30A0~0x30FF
        if (unicode >= 0x30A0 && unicode <= 0x30FF) return 1;
        // 韩文字母：0x1100~0x11FF（谚文字母）- 注意：这是双字节UTF-8范围
        // 韩文音节：0xAC00~0xD7AF
        if (unicode >= 0xAC00 && unicode <= 0xD7AF) return 1;
        // 中文标点扩展：0xFE30~0xFE4F（CJK兼容形式）
        if (unicode >= 0xFE30 && unicode <= 0xFE4F) return 1;
        // 中文竖排标点：0xFE10~0xFE1F
        if (unicode >= 0xFE10 && unicode <= 0xFE1F) return 1;
        // 全角形式：0xFF00~0xFFEF（仅全角部分，半角片假名0xFF61~0xFF9F是单宽）
        if (unicode >= 0xFF00 && unicode <= 0xFF60) return 1;
        if (unicode >= 0xFFA0 && unicode <= 0xFFEF) return 1;
        // 制表符/边框：0x2500~0x257F（Box Drawing）- 通常双宽
        if (unicode >= 0x2500 && unicode <= 0x257F) return 1;
        // 方块元素：0x2580~0x259F - 通常双宽
        if (unicode >= 0x2580 && unicode <= 0x259F) return 1;
        
        // === 以下是单宽度字符，返回0 ===
        // 上标/下标：0x2070~0x209F（⁰¹²³⁴⁵⁶⁷⁸⁹等）- 单宽
        // 箭头：0x2190~0x21FF - 单宽
        // 数学运算符：0x2200~0x22FF - 单宽
        // 杂项技术符号：0x2300~0x23FF - 单宽
        // 几何图形：0x25A0~0x25FF - 单宽
        // 杂项符号：0x2600~0x26FF - 大部分单宽
        // Dingbats：0x2700~0x27BF - 单宽
        // 希腊字母：0x0370~0x03FF - 单宽
        return 0;
    }
    // 四字节UTF-8：Emoji等（通常双宽）
    else if ((c[i] >= 0xF0 && c[i] <= 0xF4) && (i + 3 < len)) {
        unsigned int unicode = ((c[i] & 0x07) << 18) | ((c[i+1] & 0x3F) << 12) |
                               ((c[i+2] & 0x3F) << 6) | (c[i+3] & 0x3F);
        // Emoji范围：0x1F300~0x1F9FF
        if (unicode >= 0x1F300 && unicode <= 0x1F9FF) return 1;
        // 补充CJK：0x20000~0x2A6DF
        if (unicode >= 0x20000 && unicode <= 0x2A6DF) return 1;
        return 0;
    }
    // 双字节UTF-8：一般不是双宽字符
    return 0;
}

// 判断GBK字符是否为中文/全角字符（计2）
int is_gbk_double_width(const unsigned char *c) {
    // GBK中文：0xB0~0xF7 + 0xA1~0xFE；GBK全角字符：0xA1~0xA9 + 0xA1~0xFE
    return (c[0] >= 0x80) && 
           ((c[0] >= 0xB0 && c[0] <= 0xF7) || (c[0] >= 0xA1 && c[0] <= 0xA9));
}
/************************ 获取字符串实际显示长度（含中文） ************************/
// 跳过ANSI转义序列（CSI/OSC等），返回跳过后的位置
static int skip_ansi_escape(const unsigned char *u_str, int i, int len) {
    if (u_str[i] != 0x1B || i + 1 >= len) return i;
    
    unsigned char next = u_str[i + 1];
    
    // CSI序列：ESC [ ... 终结符
    if (next == '[') {
        int j = i + 2;
        while (j < len && u_str[j] >= 0x20 && u_str[j] <= 0x3F) j++;
        while (j < len && u_str[j] >= 0x20 && u_str[j] <= 0x2F) j++;
        if (j < len && u_str[j] >= 0x40 && u_str[j] <= 0x7E) {
            return j + 1;
        }
        return j;
    }
    // OSC序列：ESC ] ... (BEL或ST)
    if (next == ']') {
        int j = i + 2;
        while (j < len) {
            if (u_str[j] == 0x07) return j + 1;
            if (u_str[j] == 0x1B && j + 1 < len && u_str[j + 1] == '\\') return j + 2;
            j++;
        }
        return len;
    }
    // SS2/SS3序列：ESC N 或 ESC O + 1字符
    if (next == 'N' || next == 'O') {
        return (i + 3 < len) ? i + 3 : len;
    }
    // 其他简单转义：ESC + 单字符
    if (next >= 0x40 && next <= 0x5F) {
        return i + 2;
    }
    return i + 1;
}

unsigned long get_real_Length(const char *str, EncodingType *encoding) {
    if (str == NULL || *str == '\0') {
        if (encoding != NULL) *encoding = ENCODING_UNKNOWN;
        return 0;
    }

    EncodingType enc = detect_encoding(str);
    if (encoding != NULL) *encoding = enc;

    unsigned long real_len = 0;
    int i = 0;
    int len = (int)strlen(str);
    unsigned char *u_str = (unsigned char *)str;

    if (enc == ENCODING_UTF8) {
        while (i < len) {
            // 跳过ANSI转义序列
            if (u_str[i] == 0x1B) {
                i = skip_ansi_escape(u_str, i, len);
                continue;
            }
            // ASCII字符
            if (u_str[i] < 0x80) {
                real_len += 1;
                i += 1;
            } else {
                // 多字节UTF-8
                real_len += is_utf8_double_width(u_str, i, len) ? 2 : 1;
                if ((u_str[i] & 0xF8) == 0xF0) i += 4;
                else if ((u_str[i] & 0xF0) == 0xE0) i += 3;
                else if ((u_str[i] & 0xE0) == 0xC0) i += 2;
                else i += 1;
            }
        }
    } else if (enc == ENCODING_GBK) {
        while (i < len) {
            if (u_str[i] == 0x1B) {
                i = skip_ansi_escape(u_str, i, len);
                continue;
            }
            if (u_str[i] < 0x80) {
                real_len += 1;
                i += 1;
            } else {
                real_len += is_gbk_double_width(&u_str[i]) ? 2 : 1;
                i += 2;
            }
        }
    }

    return real_len;
}

// 根据边距类型返回填充过的字符串（包含空格），返回值需由调用方 free()
char * getSpaceContent(const char *content, size_t totalWidth, PrintMarginType marginType) {
    // empty content -> return string of spaces
    if(!content) {
        size_t sz = totalWidth + 1;
        char *out = (char*)malloc(sz);
        if(!out) return NULL;
        for(size_t i=0;i<totalWidth;i++) out[i] = ' ';
        out[totalWidth] = '\0';
        return out;
    }

    // 使用 get_real_Length 计算可见宽度（中文/全角计2），并据此计算需要填充的空格数
    unsigned long vis = get_real_Length(content, NULL);
    size_t visible = (size_t)vis;
    if(visible >= totalWidth) {
        return strdup(content);
    }
    size_t pad = totalWidth - visible;
    size_t leftPad = 0, rightPad = 0;
    switch(marginType) {
        case MARGIN_LEFT:
            leftPad = 0; rightPad = pad; break;
        case MARGIN_RIGHT:
            leftPad = pad; rightPad = 0; break;
        case MARGIN_CENTER:
            leftPad = pad / 2; rightPad = pad - leftPad; break;
        default:
            leftPad = 0; rightPad = pad; break;
    }

    size_t clen = strlen(content);
    size_t outsz = leftPad + clen + rightPad + 1;
    char *out = (char*)malloc(outsz);
    if(!out) return NULL;
    size_t pos = 0;
    for(size_t i=0;i<leftPad;i++) out[pos++] = ' ';
    memcpy(out + pos, content, clen); pos += clen;
    for(size_t i=0;i<rightPad;i++) out[pos++] = ' ';
    out[pos] = '\0';
    return out;
}

// 将原始行拆分为显示单元数组（每个单元是一个字符串），最后一个单元为 "EOL"。
char ** processRawChar(const char * rawLine) {
    if(!rawLine) return NULL;
    EncodingType enc = detect_encoding(rawLine);
    size_t blen = strlen(rawLine);
    // 预分配指针数组，最多每字节一个单元，再加EOL
    size_t cap = blen + 2;
    char **arr = (char**)malloc(sizeof(char*) * cap);
    if(!arr) return NULL;
    size_t idx = 0;
    size_t i = 0;
    const unsigned char *s = (const unsigned char*)rawLine;

    while (i < blen) {
        // 跳过并整段保留 ANSI ESC 序列（\x1b[...m）作为单个单元
        if (s[i] == 0x1B && i + 1 < blen && s[i+1] == '[') {
            size_t j = i + 2;
            while (j < blen && !(s[j] >= 0x40 && s[j] <= 0x7E)) j++;
            size_t len = (j < blen) ? (j + 1 - i) : (blen - i);
            char *tok = (char*)malloc(len + 1);
            if(!tok) break;
            memcpy(tok, s + i, len);
            tok[len] = '\0';
            arr[idx++] = tok;
            i += len;
            continue;
        }

        if (enc == ENCODING_UTF8) {
            unsigned char c = s[i];
            if (c < 0x80) {
                // ASCII 单字符
                char *tok = (char*)malloc(2);
                if(!tok) break;
                tok[0] = (char)c; tok[1] = '\0';
                arr[idx++] = tok;
                i += 1;
            } else {
                // 多字节 UTF-8
                size_t w = 1;
                if ((c & 0xF8) == 0xF0) w = 4;
                else if ((c & 0xF0) == 0xE0) w = 3;
                else if ((c & 0xE0) == 0xC0) w = 2;
                if (i + w > blen) w = blen - i;
                char *tok = (char*)malloc(w + 1);
                if(!tok) break;
                memcpy(tok, s + i, w);
                tok[w] = '\0';
                arr[idx++] = tok;
                i += w;
            }
        } else { // GBK: 非ASCII为双字节
            unsigned char c = s[i];
            if (c < 0x80) {
                char *tok = (char*)malloc(2);
                if(!tok) break;
                tok[0] = (char)c; tok[1] = '\0';
                arr[idx++] = tok;
                i += 1;
            } else {
                size_t w = (i + 1 < blen) ? 2 : 1;
                char *tok = (char*)malloc(w + 1);
                if(!tok) break;
                memcpy(tok, s + i, w);
                tok[w] = '\0';
                arr[idx++] = tok;
                i += w;
            }
        }
    }
    // 添加 EOL 标记
    arr[idx] = (char*)malloc(4);
    if (arr[idx]) strcpy(arr[idx], "EOL");
    idx++;
    // 可缩减数组大小
    arr[idx] = NULL;
    return arr;
}

void freeProcessedChars(char ** arr) {
    if(!arr) return;
    for (size_t i = 0; arr[i] != NULL; ++i) {
        free(arr[i]);
    }
    free(arr);
}
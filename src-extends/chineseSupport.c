
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
int is_utf8_double_width(const unsigned char *c, int i, int len) {
    // 三字节UTF-8：中文/全角字符
    if ((c[i] >= 0xE0 && c[i] <= 0xEF) && (i + 2 < len)) {
        unsigned int unicode = utf8_to_unicode(&c[i]);
        // 中文汉字：0x4E00~0x9FA5；全角标点/字符：0xFF00~0xFFEF
        return (unicode >= 0x4E00 && unicode <= 0x9FA5) || (unicode >= 0xFF00 && unicode <= 0xFFEF);
    }
    // 双字节UTF-8：全角字符（如全角字母/数字）
    else if ((c[i] >= 0xC0 && c[i] <= 0xDF) && (i + 1 < len)) {
        return 1;
    }
    return 0;
}

// 判断GBK字符是否为中文/全角字符（计2）
int is_gbk_double_width(const unsigned char *c) {
    // GBK中文：0xB0~0xF7 + 0xA1~0xFE；GBK全角字符：0xA1~0xA9 + 0xA1~0xFE
    return (c[0] >= 0x80) && 
           ((c[0] >= 0xB0 && c[0] <= 0xF7) || (c[0] >= 0xA1 && c[0] <= 0xA9));
}
/************************ 获取字符串实际显示长度（含中文） ************************/
unsigned long get_real_Length(const char * str, EncodingType *encoding) {
    // 1. 空字符串处理
    if (str == NULL || *str == '\0') {
        if (encoding != NULL) {
            *encoding = ENCODING_UNKNOWN;
        }
        return 0;
    }

    // 2. 检测编码
    EncodingType enc = detect_encoding(str);
    if (encoding != NULL) {
        *encoding = enc;
    }

    unsigned long real_len = 0;
    int i = 0;
    int len = strlen(str);
    unsigned char *u_str = (unsigned char *)str;

    // 3. 按编码精准计算实际长度
    if (enc == ENCODING_UTF8) {
        while (i < len) {
            // 跳过 ANSI 转义序列（例如颜色码：\x1b[...m）不计入显示长度
            if (u_str[i] == 0x1B && i + 1 < len && u_str[i+1] == '[') {
                int j = i + 2;
                // CSI 序列以 0x40('@') 到 0x7E('~') 的字节结束，常见为 'm'
                while (j < len && !(u_str[j] >= 0x40 && u_str[j] <= 0x7E)) {
                    j++;
                }
                if (j < len) {
                    // 跳过整个转义序列
                    i = j + 1;
                    continue;
                } else {
                    // 不完整的转义序列，直接跳出以防越界
                    break;
                }
            }
            if (u_str[i] < 0x80) {  // 半角ASCII（字母/数字/半角标点）：计1
                real_len += 1;
                i += 1;
            } else {
                // 中文/全角字符：计2
                if (is_utf8_double_width(u_str, i, len)) {
                    real_len += 2;
                } else {  // 非中文/全角的多字节字符（极少）：计1
                    real_len += 1;
                }
                // 按UTF-8规则步进
                if (u_str[i] < 0xE0) {
                    i += 2;
                } else if (u_str[i] < 0xF0) {
                    i += 3;
                } else {
                    i += 4;
                }
            }
        }
    } else if (enc == ENCODING_GBK) {
        while (i < len) {
            // 跳过 ANSI 转义序列（GBK 编码下也可能包含颜色码）
            if (u_str[i] == 0x1B && i + 1 < len && u_str[i+1] == '[') {
                int j = i + 2;
                while (j < len && !(u_str[j] >= 0x40 && u_str[j] <= 0x7E)) {
                    j++;
                }
                if (j < len) {
                    i = j + 1;
                    continue;
                } else {
                    break;
                }
            }
            if (u_str[i] < 0x80) {  // 半角ASCII：计1
                real_len += 1;
                i += 1;
            } else {  // 双字节GBK
                // 中文/全角字符：计2；其他双字节：计1
                real_len += is_gbk_double_width(&u_str[i]) ? 2 : 1;
                i += 2;
            }
        }
    }

    return real_len;
}
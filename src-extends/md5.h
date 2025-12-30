#ifndef MD5_H
#define MD5_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// MD5上下文结构体：存储中间状态和长度
typedef struct {
    uint32_t state[4];    // 四个32位寄存器 (A, B, C, D)
    uint32_t count[2];    // 消息长度（bit），低32位和高32位
    unsigned char buffer[64]; // 64字节缓冲区（512位）
} MD5_CTX;

// 循环左移宏
#define ROTATE_LEFT(x, n) ((x << n) | (x >> (32 - n)))

// MD5核心常量（T表）：T[i] = floor(4294967296 * abs(sin(i+1)))
static const uint32_t T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// 初始化MD5上下文
void MD5_Init(MD5_CTX *ctx);

// 辅助函数：将字节数组转换为32位无符号整数（小端序）
static uint32_t byte2uint32(const unsigned char *bytes) ;

// 处理512位（64字节）数据块
void MD5_Transform(MD5_CTX *ctx, const unsigned char block[64]) ;
// 输入数据（核心函数：处理任意长度的字节流）
void MD5_Update(MD5_CTX *ctx, const unsigned char *data, uint32_t len) ;
// 补位并输出最终哈希值（16字节）
void MD5_Final(unsigned char digest[16], MD5_CTX *ctx) ;
// 辅助函数：将16字节哈希值转换为32位十六进制字符串
void MD5_ToHex(const unsigned char digest[16], char hex[33]) ;

// 封装函数：直接计算字符串的MD5哈希（十六进制）
void MD5_String(const char *str, char hex[33]) ;

#endif // MD5_H
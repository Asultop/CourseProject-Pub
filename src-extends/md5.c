#include "md5.h"
void MD5_Init(MD5_CTX *ctx) {
    ctx->count[0] = ctx->count[1] = 0;
    // 初始状态（A, B, C, D）
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
}
static uint32_t byte2uint32(const unsigned char *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | 
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}
void MD5_Transform(MD5_CTX *ctx, const unsigned char block[64]) {
    uint32_t a = ctx->state[0], b = ctx->state[1];
    uint32_t c = ctx->state[2], d = ctx->state[3];
    uint32_t x[16]; // 64字节块拆分为16个32位整数
    uint32_t f, g;
    int i;

    // 将64字节块转换为16个32位小端序整数
    for (i = 0; i < 16; i++) {
        x[i] = byte2uint32(block + i * 4);
    }

    // 四轮运算（每轮16步）
    // 第一轮：F函数 (F = (B & C) | (~B & D))
    for (i = 0; i < 16; i++) {
        f = (b & c) | ((~b) & d);
        g = i;
        uint32_t temp = d;
        d = c;
        c = b;
        b = b + ROTATE_LEFT((a + f + T[i] + x[g]), (i < 4 ? 7 : (i < 8 ? 12 : (i < 12 ? 17 : 22))));
        a = temp;
    }

    // 第二轮：G函数 (G = (B & D) | (C & ~D))
    for (i = 16; i < 32; i++) {
        f = (b & d) | (c & (~d));
        g = (5 * i + 1) % 16;
        uint32_t temp = d;
        d = c;
        c = b;
        b = b + ROTATE_LEFT((a + f + T[i] + x[g]), (i < 20 ? 5 : (i < 24 ? 9 : (i < 28 ? 14 : 20))));
        a = temp;
    }

    // 第三轮：H函数 (H = B ^ C ^ D)
    for (i = 32; i < 48; i++) {
        f = b ^ c ^ d;
        g = (3 * i + 5) % 16;
        uint32_t temp = d;
        d = c;
        c = b;
        b = b + ROTATE_LEFT((a + f + T[i] + x[g]), (i < 40 ? 4 : (i < 44 ? 11 : (i < 48 ? 16 : 23))));
        a = temp;
    }

    // 第四轮：I函数 (I = C ^ (B | ~D))
    for (i = 48; i < 64; i++) {
        f = c ^ (b | (~d));
        g = (7 * i) % 16;
        uint32_t temp = d;
        d = c;
        c = b;
        b = b + ROTATE_LEFT((a + f + T[i] + x[g]), (i < 52 ? 6 : (i < 56 ? 10 : (i < 60 ? 15 : 21))));
        a = temp;
    }

    // 更新状态
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
}
void MD5_Update(MD5_CTX *ctx, const unsigned char *data, uint32_t len) {
    uint32_t i, j;

    // 计算当前缓冲区已有的字节数
    j = (ctx->count[0] >> 3) & 0x3F;
    // 更新总长度（bit）
    ctx->count[0] += len << 3;
    if (ctx->count[0] < (len << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += len >> 29;

    // 填充缓冲区，满64字节则处理
    if (j + len > 63) {
        // 先填充缓冲区至64字节
        memcpy(&ctx->buffer[j], data, i = 64 - j);
        MD5_Transform(ctx, ctx->buffer);
        // 处理剩余的完整64字节块
        for (; i + 63 < len; i += 64) {
            MD5_Transform(ctx, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }

    // 剩余数据存入缓冲区
    memcpy(&ctx->buffer[j], &data[i], len - i);
}
void MD5_Final(unsigned char digest[16], MD5_CTX *ctx) {
    unsigned char padding[64] = {0};
    uint32_t i, j;
    unsigned char len_buf[8]; // 存储总长度（bit）的8字节小端序

    // 补位规则：
    // 1. 先加一个1（0x80）
    // 2. 补0至长度 ≡ 56 mod 64
    // 3. 最后8字节存储总长度（bit）的小端序

    padding[0] = 0x80;
    // 计算需要补的0的数量
    j = (ctx->count[0] >> 3) & 0x3F;
    i = (j < 56) ? (56 - j) : (120 - j);

    // 存储总长度（bit）为8字节小端序
    len_buf[0] = (unsigned char)(ctx->count[0] & 0xFF);
    len_buf[1] = (unsigned char)((ctx->count[0] >> 8) & 0xFF);
    len_buf[2] = (unsigned char)((ctx->count[0] >> 16) & 0xFF);
    len_buf[3] = (unsigned char)((ctx->count[0] >> 24) & 0xFF);
    len_buf[4] = (unsigned char)(ctx->count[1] & 0xFF);
    len_buf[5] = (unsigned char)((ctx->count[1] >> 8) & 0xFF);
    len_buf[6] = (unsigned char)((ctx->count[1] >> 16) & 0xFF);
    len_buf[7] = (unsigned char)((ctx->count[1] >> 24) & 0xFF);

    // 补位并处理最后一个/两个块
    MD5_Update(ctx, padding, i);
    MD5_Update(ctx, len_buf, 8);

    // 将最终状态转换为16字节小端序哈希值
    for (i = 0; i < 4; i++) {
        digest[i]     = (unsigned char)(ctx->state[0] >> (i * 8)) & 0xFF;
        digest[i + 4] = (unsigned char)(ctx->state[1] >> (i * 8)) & 0xFF;
        digest[i + 8] = (unsigned char)(ctx->state[2] >> (i * 8)) & 0xFF;
        digest[i + 12] = (unsigned char)(ctx->state[3] >> (i * 8)) & 0xFF;
    }

    // 清空上下文（可选，防止内存泄露）
    memset(ctx, 0, sizeof(MD5_CTX));
}
void MD5_ToHex(const unsigned char digest[16], char hex[33]) {
    const char *hex_chars = "0123456789abcdef";
    int i;
    for (i = 0; i < 16; i++) {
        hex[i * 2] = hex_chars[(digest[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[digest[i] & 0x0F];
    }
    hex[32] = '\0'; // 字符串结束符
}
void MD5_String(const char *str, char hex[33]) {
    MD5_CTX ctx;
    unsigned char digest[16];
    MD5_Init(&ctx);
    MD5_Update(&ctx, (const unsigned char *)str, strlen(str));
    MD5_Final(digest, &ctx);
    MD5_ToHex(digest, hex);
}
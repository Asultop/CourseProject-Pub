#include "releaseRuntime.h"
#include "Def.h"
#include <limits.h>

int releaseRuntimeResources(const char* outputDir) {
    // 前置校验：空指针/空数据直接返回
    if (DataBase == NULL || DataBase_size == 0 || outputDir == NULL) {
        fprintf(stderr, "x> 参数错误：空指针或空数据\n");
        return -1;
    }

    uint8_t* dataBuffer = NULL;
    size_t dataSize = 0;

    // ZLIB 解压（可选）
    #if USE_ZLIB
    // 先校验压缩数据最小长度（原始大小8字节 + 至少1字节压缩数据）
    if (DataBase_size < 9) {
        fprintf(stderr, "x> 无效的压缩数据（长度不足）: %zu 字节\n", DataBase_size);
        return -1;
    }
    // 读取原始大小（小端）
    uint64_t originSizeLE;
    memcpy(&originSizeLE, DataBase + (DataBase_size - 8), 8);
    size_t originSize = (size_t)originSizeLE;

    // 解压数据
    dataBuffer = (uint8_t*)malloc(originSize);
    if (dataBuffer == NULL) {
        fprintf(stderr, "x> 解压内存分配失败: %zu 字节\n", originSize);
        return -1;
    }
    uLongf decompressSize = originSize;
    if (uncompress(dataBuffer, &decompressSize, DataBase, DataBase_size - 8) != Z_OK) {
        fprintf(stderr, "x> 数据解压失败\n");
        free(dataBuffer);
        return -1;
    }
    if (decompressSize != originSize) {
        fprintf(stderr, "x> 解压后大小不匹配（预期 %zu, 实际 %zu）\n", originSize, decompressSize);
        free(dataBuffer);
        return -1;
    }
    dataSize = decompressSize;
    #else
    dataBuffer = (uint8_t*)malloc(size);
    if (dataBuffer == NULL) {
        fprintf(stderr, "x> 数据缓冲区分配失败\n");
        return -1;
    }
    memcpy(dataBuffer, buffer, size);
    dataSize = size;
    #endif

    // 校验序列化数据最小长度（至少包含fileCount）
    if (dataSize < 4) {
        fprintf(stderr, "x> 无效的序列化数据（长度不足）: %zu 字节\n", dataSize);
        free(dataBuffer);
        return -1;
    }

    // 创建输出根目录
    if (recursiveMakeDir(outputDir) != 0) {
        free(dataBuffer);
        return -1;
    }

    size_t offset = 0;
    uint32_t fileCountLE;
    memcpy(&fileCountLE, dataBuffer + offset, 4);
    int fileCount = (int)fileCountLE;
    offset += 4;

    // 校验文件数合理性
    if (fileCount <= 0) {
        fprintf(stderr, "x> 无效的文件数量: %d\n", fileCount);
        free(dataBuffer);
        return -1;
    }

    // 还原每个文件
    for (int i = 0; i < fileCount; i++) {
        // 校验偏移是否越界（路径长度）
        if (offset + 4 > dataSize) {
            fprintf(stderr, "x> 数据越界：读取路径长度（偏移 %zu）\n", offset);
            free(dataBuffer);
            return -1;
        }
        // 读取路径长度（小端）
        uint32_t pathLenLE;
        memcpy(&pathLenLE, dataBuffer + offset, 4);
        size_t pathLen = (size_t)pathLenLE;
        offset += 4;

        // 校验路径长度合理性
        if (pathLen == 0 || pathLen >= PATH_MAX) {
            fprintf(stderr, "x> 无效的路径长度: %zu\n", pathLen);
            free(dataBuffer);
            return -1;
        }
        // 校验偏移是否越界（路径内容）
        if (offset + pathLen > dataSize) {
            fprintf(stderr, "x> 数据越界：读取路径内容（偏移 %zu, 长度 %zu）\n", offset, pathLen);
            free(dataBuffer);
            return -1;
        }

        // 读取路径
        char* filePath = (char*)malloc(pathLen);
        if (filePath == NULL) {
            fprintf(stderr, "x> 路径内存分配失败\n");
            free(dataBuffer);
            return -1;
        }
        memcpy(filePath, dataBuffer + offset, pathLen);
        offset += pathLen;

        // 校验偏移是否越界（文件大小）
        if (offset + 8 > dataSize) {
            fprintf(stderr, "x> 数据越界：读取文件大小（偏移 %zu）\n", offset);
            free(filePath);
            free(dataBuffer);
            return -1;
        }
        // 读取文件大小（小端）
        uint64_t fileSizeLE;
        memcpy(&fileSizeLE, dataBuffer + offset, 8);
        size_t fileSize = (size_t)fileSizeLE;
        offset += 8;

        // 校验偏移是否越界（文件内容）
        if (offset + fileSize > dataSize) {
            fprintf(stderr, "x>数据越界：读取文件内容（偏移 %zu, 大小 %zu）\n", offset, fileSize);
            free(filePath);
            free(dataBuffer);
            return -1;
        }

        // 读取文件内容
        uint8_t* content = NULL;
        if (fileSize > 0) {
            content = (uint8_t*)malloc(fileSize);
            if (content == NULL) {
                fprintf(stderr, "x> 文件内容内存分配失败: %s\n", filePath);
                free(filePath);
                free(dataBuffer);
                return -1;
            }
            memcpy(content, dataBuffer + offset, fileSize);
        }
        offset += fileSize;

        // 拼接输出路径
        char fullPath[PATH_MAX];
        int ret = snprintf(fullPath, sizeof(fullPath), "%s%c%s", outputDir, PATH_SEP, filePath);
        if (ret < 0 || ret >= (int)sizeof(fullPath)) {
            fprintf(stderr, "x> 输出路径溢出: %s + %s\n", outputDir, filePath);
            free(filePath);
            free(content);
            free(dataBuffer);
            return -1;
        }

        // 创建文件所在目录
        char dirPath[PATH_MAX];
        strcpy(dirPath, fullPath);
        char* lastSep = strrchr(dirPath, PATH_SEP);
        if (lastSep) {
            *lastSep = '\0';
            recursiveMakeDir(dirPath);
        }

        // 写入文件
        FILE* fp = fopen(fullPath, "wb");
        if (!fp) {
            fprintf(stderr, "x> 无法创建文件: %s - %s\n", fullPath, strerror(errno));
            free(filePath);
            free(content);
            continue;
        }
        size_t writeBytes = 0;
        if (fileSize > 0) {
            writeBytes = fwrite(content, 1, fileSize, fp);
        }
        fclose(fp);

        if (writeBytes != fileSize) {
            fprintf(stderr, "x> 写入文件失败: %s (预期 %zu, 实际 %zu)\n", fullPath, fileSize, writeBytes);
        } else {
            printf("√> 已还原: %s\n", fullPath);
        }

        free(filePath);
        free(content);
    }

    free(dataBuffer);
    printf("\n√> 还原完成！共还原 %d 个文件到 %s\n", fileCount, outputDir);
    return 0;
}

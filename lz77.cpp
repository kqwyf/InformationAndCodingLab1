#include <cstring>
#include <algorithm>

#include "lz77.h"

/*
 * 尝试匹配符号串a与b，至多匹配maxLen个符号。
 *
 * Returns:
 *     返回成功匹配的长度。
 */
int match(const char *a, const char *b, int maxLen) {
    for (int i = 0; i < maxLen; i++)
        if (a[i] != b[i])
            return i;
    return maxLen;
}

int compressLz77(const char *src, int srcLen, Lz77OutputUnit *dst, int dstMaxLen, int searchBufLen, int lookAheadBufLen) {
    // 检验输入合法性，既不为0也不为1时返回-2。见函数文档。
    for (int i = 0; i < srcLen; i++)
        if (src[i] != 0 && src[i] != 1)
            return -2;

    int len = 0;

    // 初始阶段，search buffer未被输入填满。
    // 此时始终不匹配（匹配长度为0），先使用searchBufLen个三元组将search buffer内的符号全部输出
    for (int i = 0; i < std::min(searchBufLen, srcLen); i++) {
        if (len >= dstMaxLen) return -1; // 检查输出缓冲区是否已满

        dst[len].offset = 0; // 无用参数
        dst[len].length = 0; // 未匹配任何符号
        dst[len].symbol = src[i]; // 当前符号
        len++;
    }

    // 压缩阶段，search buffer已被输入填满
    int pos = std::min(searchBufLen, srcLen); // look ahead buffer最左符号在src中的下标，表示buffer的位置，随着循环更新。
    while (pos < srcLen) {
        if (len >= dstMaxLen) return -1; // 检查输出缓冲区是否已满

        // 找出最长匹配
        int bestOffset = -1; // 已找到的最长匹配的偏移量
        int bestMatchLen = -1; // 已找到的最长匹配的长度
        for (int i = searchBufLen; i >= 1; i--) { // 从左至右遍历search buffer，以每个字符为首字符尝试匹配。
                                                  // 此处i相当于offset，因此是从searchBufLen减小到1。
            int matchLen = match(src + pos - i, src + pos,
                    std::min({           // 以下是match函数的第3个参数，用于决定最长匹配多长的符号串。
                        lookAheadBufLen, // 长度不可超过look ahead buffer
                        srcLen - pos,    // 长度不可超过输入数据
                    }));
            if (matchLen > bestMatchLen) {
                bestOffset = i;
                bestMatchLen = matchLen;
            }
        }
        if (bestMatchLen == 0) // search buffer中没有可匹配的串。
            bestOffset = 0;

        // 编码三元组并移动buffer
        dst[len].offset = bestOffset;
        dst[len].length = bestMatchLen;
        if (bestMatchLen < lookAheadBufLen && pos + bestMatchLen < srcLen) {
            // 仅当输入未被匹配完，且look ahead buffer中还有未匹配的符号时才填写symbol字段。
            dst[len].symbol = src[pos + bestMatchLen];
            pos += bestMatchLen + 1;
        } else {
            dst[len].symbol = -1; // TODO: 这里并不合理，不应该使用symbol字段的特殊值来表示此处没有符号。应考虑使用offset和length字段的特殊值来表示。
            pos += bestMatchLen;
        }
        len++;
    }

    return len;
}

int decompressLz77(const Lz77OutputUnit *src, int srcLen, char *dst, int dstMaxLen, int searchBufLen, int lookAheadBufLen) {
    int pos = 0; // look ahead buffer最左符号在dst中的下标，表示buffer的位置，随着循环更新。
    int len = 0; // 已输出的长度
    for (int i = 0; i < srcLen; i++) {
        if (src[i].offset == 0 || src[i].length == 0) { // 没有匹配串，直接输出未匹配字符
            if (len >= dstMaxLen) return -1; // 检查缓冲区是否已满
            dst[len++] = src[i].symbol;
            pos++;
        } else { // 有匹配串
            // 先输出匹配串
            int offset = src[i].offset;
            for (int j = 0; j < src[i].length; j++) {
                if (len >= dstMaxLen) return -1; // 输出每个符号前检查缓冲区是否已满
                dst[len++] = dst[pos - offset + j];
            }
            pos += src[i].length;

            // 然后检查匹配串之后是否有未匹配字符，有则输出
            if (src[i].symbol != -1) { // TODO: 不应该使用symbol字段的特殊值表示没有符号。见压缩算法中的TODO。
                if (len >= dstMaxLen) return -1; // 检查缓冲区是否已满
                dst[len++] = src[i].symbol;
                pos++;
            }
        }
    }

    return len;
}

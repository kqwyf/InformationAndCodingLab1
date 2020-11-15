#include <cstdint>
#include <cstring>
#include <algorithm>
#include <pthread.h>
#include <cstdio>

#include "lz77.h"

using std::vector;

/*
 * 尝试匹配符号串a与b，至多匹配maxLen个符号。
 *
 * Returns:
 *     返回成功匹配的长度。
 */
int match(const vector<char> &src, int a, int b, int maxLen) {
    for (int i = 0; i < maxLen; i++)
        if (src[a + i] != src[b + i])
            return i;
    return maxLen;
}

int _compressLz77(const vector<char> &src, int srcOffset, int srcLen, vector<Lz77OutputUnit> &dst, int searchBufLen, int lookAheadBufLen) {
    // 检验输入合法性，既不为0也不为1时返回-2。见函数文档。
    for (int i = 0; i < srcLen; i++)
        if (src[srcOffset + i] != 0 && src[srcOffset + i] != 1)
            return -2;

    // 初始阶段，search buffer未被输入填满。
    // 此时始终不匹配（匹配长度为0），先使用searchBufLen个三元组将search buffer内的符号全部输出
    for (int i = 0; i < std::min(searchBufLen, srcLen); i++) {
        Lz77OutputUnit newUnit;
        newUnit.offset = 0; // 无用参数
        newUnit.length = 0; // 未匹配任何符号
        newUnit.symbol = src[srcOffset + i]; // 当前符号
        dst.push_back(newUnit);
    }

    // 压缩阶段，search buffer已被输入填满
    int pos = std::min(searchBufLen, srcLen); // look ahead buffer最左符号在src中的下标，表示buffer的位置，随着循环更新。
    while (pos < srcLen) {
        printf("%d ", pos);
        // 找出最长匹配
        int bestOffset = -1; // 已找到的最长匹配的偏移量
        int bestMatchLen = -1; // 已找到的最长匹配的长度
        for (int i = searchBufLen; i >= 1; i--) { // 从左至右遍历search buffer，以每个字符为首字符尝试匹配。
                                                  // 此处i相当于offset，因此是从searchBufLen减小到1。
            int matchLen = match(src, srcOffset + pos - i, srcOffset + pos,
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
        Lz77OutputUnit newUnit;
        newUnit.offset = bestOffset;
        newUnit.length = bestMatchLen;
        if (bestMatchLen < lookAheadBufLen && pos + bestMatchLen < srcLen) {
            // 仅当输入未被匹配完，且look ahead buffer中还有未匹配的符号时才填写symbol字段。
            newUnit.symbol = src[srcOffset + pos + bestMatchLen];
            pos += bestMatchLen + 1;
        } else {
            newUnit.symbol = -1; // TODO: 这里并不合理，不应该使用symbol字段的特殊值来表示此处没有符号。应考虑使用offset和length字段的特殊值来表示。
            pos += bestMatchLen;
        }
        dst.push_back(newUnit);
    }

    return dst.size();
}

int compressLz77(const vector<char> &src, vector<Lz77OutputUnit> &dst, int searchBufLen, int lookAheadBufLen) {
    return _compressLz77(src, 0, src.size(), dst, searchBufLen, lookAheadBufLen);
}

int decompressLz77(const vector<Lz77OutputUnit> &src, vector<char> &dst, int searchBufLen, int lookAheadBufLen) {
    int pos = 0; // look ahead buffer最左符号在dst中的下标，表示buffer的位置，随着循环更新。
    for (int i = 0; i < src.size(); i++) {
        if (src[i].offset == 0 || src[i].length == 0) { // 没有匹配串，直接输出未匹配字符
            dst.push_back(src[i].symbol);
            pos++;
        } else { // 有匹配串
            // 先输出匹配串
            int offset = src[i].offset;
            for (int j = 0; j < src[i].length; j++) {
                dst.push_back(dst[pos - offset + j]);
            }
            pos += src[i].length;

            // 然后检查匹配串之后是否有未匹配字符，有则输出
            if (src[i].symbol != -1) { // TODO: 不应该使用symbol字段的特殊值表示没有符号。见压缩算法中的TODO。
                dst.push_back(src[i].symbol);
                pos++;
            }
        }
    }

    return dst.size();
}

/*
 * 由于Pthread仅支持传递一个参数，因此将所有参数打包为一个结构体，
 * 并将compressLz77和decompressLz77打包成_parallel_compressLz77和_parallel_decompressLz77
 */

struct CompressLz77Args {
    const vector<char> *src = NULL;
    int srcOffset;
    int srcLen;
    vector<Lz77OutputUnit> *dst = NULL;
    int searchBufLen;
    int lookAheadBufLen;
};

struct DecompressLz77Args {
    const vector<Lz77OutputUnit> *src;
    int srcOffset;
    int srcLen;
    vector<char> *dst;
    int searchBufLen;
    int lookAheadBufLen;
};

void *_parallel_compressLz77(void *args) {
    CompressLz77Args *xargs = (CompressLz77Args*) args;
    intptr_t len = _compressLz77(*(xargs->src), xargs->srcOffset, xargs->srcLen, *(xargs->dst), xargs->searchBufLen, xargs->lookAheadBufLen);
    return (void*)len;
}

int parallel_compressLz77(int num_t, const vector<char> &src, Lz77ParallelResult &dst, int searchBufLen, int lookAheadBufLen) {
    // 分发参数
    CompressLz77Args args[num_t];
    vector<Lz77OutputUnit> tmp_blocks[num_t];
    int block_len = (src.size() + num_t - 1) / num_t;

    if (dst.lens.size())
        dst.lens.clear();
    if (dst.blocks.size())
        dst.blocks.clear();

    for (int i = 0; i < num_t; ++i) {
        dst.blocks.push_back(tmp_blocks[i]);
        args[i].src = &src;
        args[i].srcOffset = block_len * i;
        args[i].srcLen = std::min(src.size() - (block_len * i),
                (unsigned long)block_len); // 长度
        args[i].dst = &dst.blocks.back();
        args[i].searchBufLen = searchBufLen;
        args[i].lookAheadBufLen = lookAheadBufLen;
    }

    // 创建子线程
    pthread_t threads[num_t];

    for (int i = 0; i < num_t; ++i) {
        int res_code = pthread_create(&threads[i], NULL, _parallel_compressLz77, (void*)&args[i]);
        if (res_code) return -1;
    }

    // 回收结果
    int len = 0;    // 统计每个子线程压缩结果各自长度之和

    for (int i = 0; i < num_t; ++i) {
        void *retval;
        if (pthread_join(threads[i], &retval))
            return -1;
        int _len = (intptr_t) retval;
        len += _len;
        dst.lens.push_back(_len);   // 每个子线程的返回值为压缩后的三元组个数
    }

    return len;
}

void *_parallel_decompressLz77(void *args) {
    DecompressLz77Args *xargs = (DecompressLz77Args*) args;
    intptr_t len = decompressLz77(*(xargs->src), *(xargs->dst), xargs->searchBufLen, xargs->lookAheadBufLen);
    return (void*)len;
}

int parallel_decompressLz77(const Lz77ParallelResult &src, vector<char> &dst, int searchBufLen, int lookAheadBufLen) {
    // 分发参数
    int num_t = src.lens.size();
    DecompressLz77Args args[num_t];
    vector<char> dsts[num_t];

    for (int i = 0; i < num_t; ++i) {
        args[i].src = &(src.blocks[i]);
        args[i].srcLen = src.lens[i];
        args[i].dst = &dsts[i];
        args[i].searchBufLen = searchBufLen;
        args[i].lookAheadBufLen = lookAheadBufLen;
    }

    // 创建子线程
    pthread_t threads[num_t];

    for (int i = 0; i < num_t; ++i) {
        int res_code = pthread_create(&threads[i], NULL, _parallel_decompressLz77, (void*)&args[i]);
        if (res_code) return -1;
    }

    // 回收结果
    int len = 0;    // 统计每个子线程解压缩结果各自长度之和

    for (int i = 0; i < num_t; ++i) {
        void *retval;
        if (pthread_join(threads[i], &retval)) return -1;
        int _len = (intptr_t) retval;
        // 将结果写到dst中
        for (int j = 0; j < _len; ++j)
            dst[len++] = (*(args[i].dst))[j];
    }

    return len;
}

#pragma once
#include <vector>

struct Lz77OutputUnit {
    int offset; // 相对于buffer边界的偏移量
    int length; // 匹配长度
    char symbol; // 下一个不匹配字符
};

/*
 * 使用LZ77算法压缩数据。
 * 算法假设search buffer与look ahead buffer长度均大于1。如果设置为1，有可能导致未知错误。
 *
 * Params:
 *     src : 输入数组，每个元素表示源数据中的1个符号。
 *     dst : 压缩结果输出数组，每个元素表示一个三元组，详见LZ77算法原理。
 *     searchBufLen : LZ77算法参数，search buffer的长度。
 *     lookAheadBufLen : LZ77算法参数，look ahead buffer的长度。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当压缩输出长度超过输出缓冲区长度时，函数返回-1。
 */
int compressLz77(const std::vector<char> &src, std::vector<Lz77OutputUnit> &dst, int searchBufLen, int lookAheadBufLen);

/*
 * 使用LZ77算法解压缩数据。
 * 参数意义与compressLz77相似。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当解压输出长度超过输出缓冲区长度时，函数返回-1。
 */
int decompressLz77(const std::vector<Lz77OutputUnit> &src, std::vector<char> &dst, int searchBufLen, int lookAheadBufLen);

/*
 * 并行压缩返回结果
 */
struct Lz77ParallelResult {
    std::vector<int> lens;
    std::vector<std::vector<Lz77OutputUnit>> blocks;
};

/**
 * 并行压缩，除了增加表示并行度的num_t参数外，其他参数与compressLz77相同。
 */
int parallel_compressLz77(int num_t, const std::vector<char> &src, Lz77ParallelResult &dst, int searchBufLen, int lookAheadBufLen);

/**
 * 并行解压，除了增加表示并行度的num_t参数外，其他参数与decompressLz77相同。
 */
int parallel_decompressLz77(const Lz77ParallelResult &src, std::vector<char> &dst, int searchBufLen, int lookAheadBufLen);

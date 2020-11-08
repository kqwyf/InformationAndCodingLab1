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
 *     src : 输入数组，每个元素表示源数据中的1个符号（目前按照1个bit来实现，即符号集中只有0和1。是否可以优化成支持其它进制符号集？）。
 *     srcLen : 输入数组的有效长度。（src和dst也可以换成vector从而不需显式指定长度）
 *     dst : 压缩结果输出数组，每个元素表示一个三元组，详见LZ77算法原理。
 *     dstMaxLen : 输出缓冲区的最大长度（按元素个数计）。当实际输出超过这一长度时，输出将被截断在这一长度，同时函数返回-1。
 *     searchBufLen : LZ77算法参数，search buffer的长度。
 *     lookAheadBufLen : LZ77算法参数，look ahead buffer的长度。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当压缩输出长度超过输出缓冲区长度时，函数返回-1。
 *     当输入中包含除0和1外的其它符号时，函数返回-2。
 */
int compressLz77(const char *src, int srcLen, Lz77OutputUnit *dst, int dstMaxLen, int searchBufLen, int lookAheadBufLen);

/*
 * 使用LZ77算法解压缩数据。
 * 参数意义与compressLz77相似。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当解压输出长度超过输出缓冲区长度时，函数返回-1。
 *     当输入中包含除0和1外的其它符号时，函数返回-2。（未实现）
 */
int decompressLz77(const Lz77OutputUnit *src, int srcLen, char *dst, int dstMaxLen, int searchBufLen, int lookAheadBufLen);

/*
 * 并行压缩返回结果
 */
struct Lz77ParallelResult {
    std::vector<int> lens;
    std::vector<Lz77OutputUnit*> blocks;
};

/**
 * 并行压缩，除了增加表示并行度的num_t参数外，其他参数与compressLz77相同。
 */
int parallel_compressLz77(int num_t, const char *src, int srcLen, Lz77ParallelResult *dst, int dstMaxLen, int searchBufLen, int lookAheadBufLen);

/**
 * 并行解压，除了增加表示并行度的num_t参数外，其他参数与decompressLz77相同。
 */
int parallel_decompressLz77(const Lz77ParallelResult *src, char *dst, int dstMaxLen, int searchBufLen, int lookAheadBufLen);

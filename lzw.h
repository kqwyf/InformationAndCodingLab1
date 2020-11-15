#pragma once
#include <vector>

struct LzWOutputUnit {
    int index; // 匹配词在字典中的下标
};

/*
 * 使用LZW算法压缩数据。
 * 算法假设search buffer与look ahead buffer长度均大于1。如果设置为1，有可能导致未知错误。
 *
 * Params:
 *     src : 输入数组，每个元素表示源数据中的1个符号（目前按照1个bit来实现，即符号集中只有0和1。是否可以优化成支持其它进制符号集？）。
 *     dst : 压缩结果输出数组，每个元素表示一个三元组，详见LZ78算法原理。
 *     dictSize: LZW算法参数，字典大小（entry最大数量）。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当压缩输出长度超过输出缓冲区长度时，函数返回-1。
 */
int compressLzW(const std::vector<char> &src, std::vector<LzWOutputUnit> &dst, int dictSize);

/*
 * 使用LZW算法解压缩数据。
 * 参数意义与compressLzW相似。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当解压输出长度超过输出缓冲区长度时，函数返回-1。
 */
int decompressLzW(const std::vector<LzWOutputUnit> &src, std::vector<char> &dst, int dictSize);


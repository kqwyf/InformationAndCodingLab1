#pragma once
#include <vector>

using namespace std;

struct Lz78OutputUnit {
    int index; // 匹配词在字典中的下标
    char symbol; // 下一个不匹配字符

    Lz78OutputUnit(int index, char symbol) {
        this->index = index;
        this->symbol = symbol;
    }
};

/*
 * 使用LZ78算法压缩数据。
 * 算法假设search buffer与look ahead buffer长度均大于1。如果设置为1，有可能导致未知错误。
 *
 * Params:
 *     src : 输入数组，每个元素表示源数据中的1个符号（目前按照1个bit来实现，即符号集中只有0和1。是否可以优化成支持其它进制符号集？）。
 *     srcLen : 输入数组的有效长度。（src和dst也可以换成vector从而不需显式指定长度）
 *     dst : 压缩结果输出数组，每个元素表示一个三元组，详见LZ78算法原理。
 *     dstMaxLen : 输出缓冲区的最大长度（按元素个数计）。当实际输出超过这一长度时，输出将被截断在这一长度，同时函数返回-1。
 *     dictSize: LZ78算法参数，字典大小（entry最大数量）。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当压缩输出长度超过输出缓冲区长度时，函数返回-1。
 *     当输入中包含除0和1外的其它符号时，函数返回-2。
 */
int compressLz78(const vector<char> &src, vector<Lz78OutputUnit> &dst, int dictSize);

/*
 * 使用LZ78算法解压缩数据。
 * 参数意义与compressLz78相似。
 *
 * Returns:
 *     正常情况下，函数返回输出长度（按输出元素个数计）。
 *     当解压输出长度超过输出缓冲区长度时，函数返回-1。
 *     当输入中包含除0和1外的其它符号时，函数返回-2。（未实现）
 */
int decompressLz78(const vector<Lz78OutputUnit> &src, vector<char> &dst, int dictSize);


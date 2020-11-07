#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "lz77.h"

const int MAX_INPUT_LENGTH = 1000; // 随机产生的输入的最长长度
const int MAX_OUTPUT_LENGTH = 2000; // 压缩输出缓冲区长度
const int MAX_DECODE_LENGTH = 10000; // 解压输出缓冲区长度

const int MAX_SEARCH_LENGTH = 100;
const int MAX_LOOKAHEAD_LENGTH = 200;

int main() {
    srand(time(0));

    int searchBufLen = rand() % (MAX_SEARCH_LENGTH - 2) + 2;
    int lookAheadBufLen = rand() % (MAX_LOOKAHEAD_LENGTH - 2) + 2;

    // 随机生成输入
    int inputLen = rand() % (MAX_INPUT_LENGTH - 2) + 2;
    char *src = new char[MAX_INPUT_LENGTH];
    for (int i = 0; i < inputLen; i++)
        src[i] = rand() % 2;

    // 压缩
    Lz77OutputUnit *dst = new Lz77OutputUnit[MAX_OUTPUT_LENGTH];
    int retval = compressLz77(src, inputLen, dst, MAX_OUTPUT_LENGTH, searchBufLen, lookAheadBufLen);

    // 解压缩
    char *out = new char[MAX_DECODE_LENGTH];
    retval = decompressLz77(dst, retval, out, MAX_DECODE_LENGTH, searchBufLen, lookAheadBufLen);

    // 检查
    if (inputLen != retval) return -1;
    for (int i = 0; i < inputLen; i++)
        if (src[i] != out[i]) return -1;

    return 0;
}

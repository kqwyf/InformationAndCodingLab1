#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "lz77.h"

const int NUM_TESTS = 10000;

const int MAX_INPUT_LENGTH = 1000; // 随机产生的输入的最长长度
const int MAX_OUTPUT_LENGTH = 2000; // 压缩输出缓冲区长度
const int MAX_DECODE_LENGTH = 10000; // 解压输出缓冲区长度

const int MAX_SEARCH_LENGTH = 100;
const int MAX_LOOKAHEAD_LENGTH = 200;

int main() {
    srand(time(0));

    for (int test = 0; test < NUM_TESTS; test++) {
        int searchBufLen = rand() % (MAX_SEARCH_LENGTH - 2) + 2;
        int lookAheadBufLen = rand() % (MAX_LOOKAHEAD_LENGTH - 2) + 2;

        // 随机生成输入
        int inputLen = rand() % (MAX_INPUT_LENGTH - 2) + 2;
        char *src = new char[MAX_INPUT_LENGTH];
        for (int i = 0; i < inputLen; i++) {
            src[i] = rand() % 2;
        }

        // 压缩
        Lz77OutputUnit *dst = new Lz77OutputUnit[MAX_OUTPUT_LENGTH];
        int retval = compressLz77(src, inputLen, dst, MAX_OUTPUT_LENGTH, searchBufLen, lookAheadBufLen);

        // 解压缩
        char *out = new char[MAX_DECODE_LENGTH];
        retval = decompressLz77(dst, retval, out, MAX_DECODE_LENGTH, searchBufLen, lookAheadBufLen);

        // 检查
        bool flag = false;
        if (inputLen != retval) flag = true;
        for (int i = 0; i < inputLen; i++) {
            if (src[i] != out[i]) {
                flag = true;
                break;
            }
        }

        // 若有错误则打印输入并退出
        if (flag) {
            printf("Error on input:\n");
            for (int i = 0; i < inputLen; i++)
                printf("%d", src[i]);
            printf("\n");
            printf("Input Length: %d\n", inputLen);
            printf("Search Buffer Length: %d\n", searchBufLen);
            printf("Look Ahead Buffer Length: %d\n", lookAheadBufLen);
            return -1;
        }
    }

    return 0;
}

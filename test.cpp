#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "lz77.h"

const int NUM_TESTS = 10;

const int MAX_INPUT_LENGTH = 800000; // 随机产生的输入的最长长度
const int MAX_OUTPUT_LENGTH = 800000; // 压缩输出缓冲区长度
const int MAX_DECODE_LENGTH = 800000; // 解压输出缓冲区长度

const int MAX_SEARCH_LENGTH = 500;
const int MAX_LOOKAHEAD_LENGTH = 500;

const int N_THREAD = 2;

#define show(x) printf("%d\n", x);

int rrand(int _max, int range) {
    int tmp = (rand() >> 3) * (rand() >> 2);
    return _max - tmp % range ;
}

int main() {
    time_t seed = time(0);
    time_t start_time, end_time;

    char *src = new char[MAX_INPUT_LENGTH];
    char *out = new char[MAX_DECODE_LENGTH];
    Lz77OutputUnit *dst1 = new Lz77OutputUnit[MAX_OUTPUT_LENGTH];
    Lz77ParallelResult *dst2 = new Lz77ParallelResult();

    srand(seed);
    start_time = clock();
    for (int test = 0; test < NUM_TESTS; test++) {
        int searchBufLen = rrand(MAX_SEARCH_LENGTH, 50);
        int lookAheadBufLen = rrand(MAX_LOOKAHEAD_LENGTH, 50);

        // 随机生成输入
        int inputLen = rrand(MAX_INPUT_LENGTH, 10000);
        for (int i = 0; i < inputLen; i++) src[i] = rand() % 2;

        // 压缩
        int retval = compressLz77(src, inputLen, dst1, MAX_OUTPUT_LENGTH, searchBufLen, lookAheadBufLen);
        // 解压缩
        retval = decompressLz77(dst1, retval, out, MAX_DECODE_LENGTH, searchBufLen, lookAheadBufLen);

        // 检查
        bool flag = false;
        if (inputLen != retval) flag = true;
        else {
            for (int i = 0; i < inputLen; i++) {
                if (src[i] != out[i]) {
                    flag = true;
                    break;
                }
            }
        }

        // 若有错误则打印输入并退出
        if (flag) {
            printf("Input Length: %d\n", inputLen);
            printf("Search Buffer Length: %d\n", searchBufLen);
            printf("Look Ahead Buffer Length: %d\n", lookAheadBufLen);
            return -1;
        }
    }
    end_time = clock();

    printf("Baseline: %lld ms\n", (end_time - start_time) * 1000 / CLOCKS_PER_SEC);

    srand(seed);
    start_time = clock();
    for (int test = 0; test < NUM_TESTS; test++) {
        int searchBufLen = rrand(MAX_SEARCH_LENGTH, 50);
        int lookAheadBufLen = rrand(MAX_LOOKAHEAD_LENGTH, 50);

        // 随机生成输入
        int inputLen = rrand(MAX_INPUT_LENGTH, 10000);
        for (int i = 0; i < inputLen; i++) src[i] = rand() % 2;

        // 压缩
        int retval = parallel_compressLz77(N_THREAD, src, inputLen, dst2, MAX_OUTPUT_LENGTH, searchBufLen, lookAheadBufLen);
        // 解压缩
        retval = parallel_decompressLz77(dst2, out, MAX_DECODE_LENGTH, searchBufLen, lookAheadBufLen);

        // 检查
        bool flag = false;
        if (inputLen != retval) flag = true;
        else {
            for (int i = 0; i < inputLen; i++) {
                if (src[i] != out[i]) {
                    flag = true;
                    break;
                }
            }
        }

        // 若有错误则打印输入并退出
        if (flag) {
            printf("Input Length: %d\n", inputLen);
            printf("Search Buffer Length: %d\n", searchBufLen);
            printf("Look Ahead Buffer Length: %d\n", lookAheadBufLen);
            return -1;
        }
    }
    end_time = clock();

    printf("Multi-core: %lld ms\n", (end_time - start_time) * 1000 / CLOCKS_PER_SEC);

    delete []src;
    delete []dst1;
    delete dst2;
    delete []out;

    return 0;
}

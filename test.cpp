#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <string>

#include "lz77.h"

int NUM_TESTS = 10;

int MAX_INPUT_LENGTH = 800000;            // 随机产生的输入的最长长度
int MAX_OUTPUT_LENGTH = MAX_INPUT_LENGTH; // 压缩输出缓冲区长度
int MAX_DECODE_LENGTH = MAX_INPUT_LENGTH; // 解压输出缓冲区长度

int MAX_SEARCH_LENGTH = 500;
int MAX_LOOKAHEAD_LENGTH = 500;

int N_THREAD = 2;
time_t seed = 0;

#define putline(x) printf("%s\n", x)

void show_usage() {
    putline("-n; --iter    number iters. (10)");
    putline("--input       maximum input length. (8000000)");
    putline("--search      maximum search buffer length. (500)");
    putline("--look        maximum look-ahead buffer length. (500)");
    putline("-t; --thread  thread. (2)");
    putline("-s; --seed    random seed. (0)");
    exit(0);
}

void unknown_arg_err() {
    putline("Unknown argument error");
    show_usage();
}

#define smatch(a,b)     !strcmp(a,b)
#define NEXT_INT_ARG    std::atoi(argv[++i])

/*
 * 解析命令行参数。
 */
void parse_arg(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];
        if (smatch(arg, "-n") || smatch(arg, "--iter")) NUM_TESTS = std::stoi(argv[++i]);
        else if (smatch(arg, "--input")) MAX_OUTPUT_LENGTH = MAX_DECODE_LENGTH = MAX_INPUT_LENGTH = NEXT_INT_ARG;
        else if (smatch(arg, "--search")) MAX_SEARCH_LENGTH = NEXT_INT_ARG;
        else if (smatch(arg, "--look")) MAX_LOOKAHEAD_LENGTH = NEXT_INT_ARG;
        else if (smatch(arg, "-t") || smatch(arg, "--thread")) N_THREAD = NEXT_INT_ARG;
        else if (smatch(arg, "-s") || smatch(arg, "--seed")) seed = (time_t)(NEXT_INT_ARG);
        else if (smatch(arg, "-h") || smatch(arg, "--help")) show_usage();
        else unknown_arg_err();
    }
}

int rrand(int _max, int range) {
    int tmp = (rand() >> 3) * (rand() >> 2);
    return _max - tmp % range ;
}

int main(int argc, char* argv[]) {
    parse_arg(argc, argv);

    time_t start_time, end_time;

    // 分配空间
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

    // 回收跑路
    delete []src;
    delete []dst1;
    delete dst2;
    delete []out;

    return 0;
}

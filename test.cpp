#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <string>

#include "lz77.h"
#include "lz78.h"

using namespace std;

int NUM_TESTS = 10;

int MAX_INPUT_LENGTH = 800000;            // 随机产生的输入的最长长度

int MAX_SEARCH_LENGTH = 500;
int MAX_LOOKAHEAD_LENGTH = 500;
int MAX_DICT_SIZE = 500;  // TODO: dictSize怎么确定？

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
#define NEXT_INT_ARG    atoi(argv[++i])

/*
 * 解析命令行参数。
 */
void parse_arg(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];
        if (smatch(arg, "-n") || smatch(arg, "--iter")) NUM_TESTS = stoi(argv[++i]);
        else if (smatch(arg, "--input")) MAX_INPUT_LENGTH = NEXT_INT_ARG;
        else if (smatch(arg, "--search")) MAX_SEARCH_LENGTH = NEXT_INT_ARG;
        else if (smatch(arg, "--look")) MAX_LOOKAHEAD_LENGTH = NEXT_INT_ARG;
        else if (smatch(arg, "-t") || smatch(arg, "--thread")) N_THREAD = NEXT_INT_ARG;
        else if (smatch(arg, "-s") || smatch(arg, "--seed")) seed = (time_t)(NEXT_INT_ARG);
        else if (smatch(arg, "-h") || smatch(arg, "--help")) show_usage();
        else unknown_arg_err();
    }
}

int rrand(int _max, int range) {
    int tmp = rand() & 0xffff;
    return _max - tmp * tmp % range;
}

bool test_lz77(const vector<char> &src, int searchBufLen, int lookAheadBufLen, time_t &time_cost) {
    // 分配空间
    vector<Lz77OutputUnit> dst77;
    vector<char> out;

    time_t start = clock();
    int retval = compressLz77(src, dst77, searchBufLen, lookAheadBufLen);
    retval = decompressLz77(dst77, out, searchBufLen, lookAheadBufLen);
    time_cost += clock() - start;

    bool flag = false;
    if (src.size() != retval) flag = false;
    else {
        for (int i = 0; i < retval; ++i) {
            if (src[i] != out[i]) {
                flag = true;
                break;
            }
        }
    }

    if (flag) {
        printf("Input Length: %zu\n", src.size());
        printf("Output Length: %zu\n", out.size());
        return false;
    }

    return true;
}

bool test_lz77_parallel(int num_t, const vector<char> &src, int searchBufLen, int lookAheadBufLen, time_t &time_cost) {
    // 分配空间
    Lz77ParallelResult dst77;
    vector<char> out;

    time_t start = clock();
    int retval = parallel_compressLz77(num_t, src, dst77, searchBufLen, lookAheadBufLen);
    retval = parallel_decompressLz77(dst77, out, searchBufLen, lookAheadBufLen);
    time_cost += clock() - start;

    bool flag = false;
    if (src.size() != retval) flag = false;
    else {
        for (int i = 0; i < retval; ++i) {
            if (src[i] != out[i]) {
                flag = true;
                break;
            }
        }
    }

    if (flag) {
        printf("Input Length: %zu\n", src.size());
        printf("Output Length: %zu\n", out.size());
        return false;
    }

    return true;
}

bool test_lz78(const vector<char> &src, int dictSize, time_t &time_cost) {
    // 分配空间
    vector<Lz78OutputUnit> dst78;
    vector<char> out;

    time_t start = clock();
    int retval = compressLz78(src, dst78, dictSize);
    retval = decompressLz78(dst78, out, dictSize);
    time_cost += clock() - start;

    // 检查
    bool flag = false;
    if (src.size() != retval) flag = true;
    else {
        for (int i = 0; i < retval; i++) {
            if (src[i] != out[i]) {
                flag = true;
                break;
            }
        }
    }

    if (flag) {
        // Do something
        return false;
    }

    return true;
}

bool test_lzw(const vector<char> &src, time_t &time_cost) {
    return true;
}

int main(int argc, char* argv[]) {
    parse_arg(argc, argv);

    time_t time_lz77 = 0, time_lz77_parall = 0, time_lz78 = 0, time_lzw = 0;

    // TODO: 压缩率测试

    printf("max input len: %d\n", MAX_INPUT_LENGTH);

    srand(seed);

    for (int test = 0; test < NUM_TESTS; ++test) {
        // 分配空间
        vector<char> src;

        // 随机生成输入配置
        int inputLen = rrand(MAX_INPUT_LENGTH, 10000);
        for (int i = 0; i < inputLen; i++) src.push_back(rand() % 2);   // TODO: 以byte为单位
        int searchBufLen = rrand(MAX_SEARCH_LENGTH, MAX_SEARCH_LENGTH - 5);
        int lookAheadBufLen = rrand(MAX_LOOKAHEAD_LENGTH, MAX_LOOKAHEAD_LENGTH - 5);
        int dictSize = rrand(MAX_DICT_SIZE, 100);

        test_lz77(src, searchBufLen, lookAheadBufLen, time_lz77);
        test_lz77_parallel(N_THREAD, src, searchBufLen, lookAheadBufLen, time_lz77_parall);
        test_lz78(src, dictSize, time_lz78);
    }

    printf("LZ77:       %lld ms\n", time_lz77 * 1000 / CLOCKS_PER_SEC / NUM_TESTS);
    printf("Multi-core: %lld ms\n", time_lz77_parall * 1000 / CLOCKS_PER_SEC / NUM_TESTS);
    printf("LZ78:       %lld ms\n", time_lz78 * 1000 / CLOCKS_PER_SEC / NUM_TESTS);
    printf("LZW:        %lld ms\n", time_lzw * 1000 / CLOCKS_PER_SEC / NUM_TESTS);

    return 0;
}

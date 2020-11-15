#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>

#include "lz77.h"
#include "lz78.h"

const char *usage = "Usage: main.exe -[7|8] -[C|D] -n <n_thread> --sb <searchBufLen> --lb <lookAheadBufLen> --ds <dictSize> -i <input_file> -o <output_file>";

int compress = 0; // 0->undefined,  1->compress, 2->decompress
int method = 0;   // 0->undefined,  1->LZ77,     2->LZ78
bool verbose = 0;
int searchBufLen = 2;
int lookAheadBufLen = 2;
int dictSize = 2;
int n_thread = 1;

char* input_file = NULL;
char* output_file = NULL;

void show_usage() {
    printf("%s\n", usage);
    exit(0);
}

void unknown_arg_err() {
    printf("Unknown argument error\n");
    show_usage();
    exit(-1);
}

void multi_def_err() {
    printf("Multi-definded error\n");
    show_usage();
    exit(-1);
}

/*
 * 解析命令行参数。
 */
int parse_arg(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];
        if (!strcmp(arg, "-C") || !strcmp(arg, "-c")) {
            if (compress && compress ^ 1) multi_def_err();
            compress = 1;
        }
        else if (!strcmp(arg, "-D") || !strcmp(arg, "-d")) {
            if (compress && compress ^ 2) multi_def_err();
            compress = 2;
        }
        else if (!strcmp(arg, "-7")) {
            if (method && method ^ 1) multi_def_err();
            method = 1;
        }
        else if (!strcmp(arg, "-8")) {
            if (method && method ^ 2) multi_def_err();
            method = 2;
        }
        else if (!strcmp(arg, "--sb") && argv[i+1][0] != '-') searchBufLen = std::stoi(argv[++i]);
        else if (!strcmp(arg, "--lb") && argv[i+1][0] != '-') lookAheadBufLen = std::stoi(argv[++i]);
        else if (!strcmp(arg, "--ds") && argv[i+1][0] != '-') dictSize = std::stoi(argv[++i]);
        else if (!strcmp(arg, "-n") && argv[i+1][0] != '-') n_thread = std::atoi(argv[++i]);
        else if (!strcmp(arg, "-i") && argv[i+1][0] != '-') input_file = argv[++i];
        else if (!strcmp(arg, "-o") && argv[i+1][0] != '-') output_file = argv[++i];
        else if (!strcmp(arg, "-v")) verbose = true;
        else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) show_usage();
        else unknown_arg_err();
    }

    if (!input_file || !output_file) {
        printf("No file error\n");
        show_usage();
        exit(-1);
    }

    if (!compress && (compress = 1));   // 默认执行：压缩
    if (!method && (method = 1));       // 默认采用：LZ77

    if (verbose) {
        if (compress == 1) printf("Compress with");
        else printf("Decompress with");
        if (method == 1) printf("LZ77 (SearchBufLen = %d, LookAheadBufLen = %d)\n", searchBufLen, lookAheadBufLen);
        else printf("LZ78\n");
        printf("%d thread(s)\nIn: %s, Out: %s\n", n_thread, input_file, output_file);
    }

    return 0;
}

/*
 * 读取文件字节数
 */
int get_file_size(const char *filename) {
    std::ifstream inFile(filename, std::ifstream::in | std::ifstream::binary);
    std::streampos begin = inFile.tellg(), end;
    inFile.seekg(0, std::ios::end);
    end = inFile.tellg();
    inFile.close();
    return (int)(end - begin);
}

void fileToArray(const char *src, int srcLen, char *dst) {
    for (int i = 0; i < srcLen; i++) {
        for (int j = 0; j < 8; j++) {
            dst[i * 8 + j] = (src[i] >> j) & 1;
        }
    }
}

void arrayToFile(const char *src, int srcLen, char *dst) {
    for (int i = 0; i < srcLen / 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i * 8 + j < srcLen) {
                dst[i] |= src[i] << j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // 解析参数
    parse_arg(argc, argv);

    std::ifstream inFile(input_file, std::ifstream::in | std::ifstream::binary);
    std::streampos begin = inFile.tellg(), end;
    inFile.seekg(0, std::ios::end);
    end = inFile.tellg();
    int size = end - begin;     // 获取文件大小
    int srcLen = size * 8;
    char *buffer = new char[size + 1];
    char *src = new char[srcLen + 1];
    inFile.seekg(0, std::ios::beg);
    inFile.read(buffer, size);
    fileToArray(buffer, size, src); // 将文件内容转换为仅由0和1构成的char数组
    inFile.close();

    // 执行{compress, decompress} x {LZ77, LZ78}中的一种
    switch (compress * 10 + method) {
        case 11:
            printf("Compress + LZ77\n");
            Lz77OutputUnit *dst = new Lz77OutputUnit[srcLen + 1];
            int outLen = compressLz77(src, srcLen, dst, srcLen, searchBufLen, lookAheadBufLen);
            std::ofstream outFile(output_file, std::ofstream::out | std::ofstream::binary);
            outFile.write((char*)dst, sizeof(Lz77OutputUnit) * outLen);
            outFile.close();
            break;
        case 12:
            printf("Compress + LZ78\n");
            // decompressLz77();
            break;
        case 21:
            printf("Decompress + LZ77\n");
            break;
        case 22:
            printf("Decompress + LZ78\n");
            break;
    }

    return 0;
}

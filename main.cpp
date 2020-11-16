#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>
#include <memory>

#include "lz77.h"
#include "lz78.h"
#include "lzw.h"

using namespace std;

const char *usage = "Usage: main.exe -[7|8|p|w] -[C|D] -n <n_thread> --sb <searchBufLen> --lb <lookAheadBufLen> --ds <dictSize> -i <input_file> -o <output_file>";

int compress = 0; // 0->undefined,  1->compress, 2->decompress
int method = 0;   // 0->undefined,  1->LZ77,     2->LZ78,      3->LZ77 parallel, 4-> LZW
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
            if (method != 0) multi_def_err();
            method = 1;
        }
        else if (!strcmp(arg, "-8")) {
            if (method != 0) multi_def_err();
            method = 2;
        }
        else if (!strcmp(arg, "-p")) {
            if (method != 0) multi_def_err();
            method = 3;
        }
        else if (!strcmp(arg, "-w")) {
            if (method != 0) multi_def_err();
            method = 4;
        }
        else if (!strcmp(arg, "--sb") && argv[i+1][0] != '-') searchBufLen = stoi(argv[++i]);
        else if (!strcmp(arg, "--lb") && argv[i+1][0] != '-') lookAheadBufLen = stoi(argv[++i]);
        else if (!strcmp(arg, "--ds") && argv[i+1][0] != '-') dictSize = stoi(argv[++i]);
        else if (!strcmp(arg, "-n") && argv[i+1][0] != '-') n_thread = atoi(argv[++i]);
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
    ifstream inFile(filename, ifstream::in | ifstream::binary);
    streampos begin = inFile.tellg(), end;
    inFile.seekg(0, ios::end);
    end = inFile.tellg();
    inFile.close();
    return (int)(end - begin);
}

int main(int argc, char *argv[]) {
    // 解析参数
    parse_arg(argc, argv);

    ifstream inFile(input_file, ifstream::in | ifstream::binary);
    streampos begin = inFile.tellg(), end;
    inFile.seekg(0, ios::end);
    end = inFile.tellg();
    int inSize = end - begin; // 获取文件大小
    inFile.seekg(0, ios::beg);

    char *inBuffer = new char[inSize];
    inFile.read(inBuffer, inSize);
    inFile.close();

    // 执行{compress, decompress} x {LZ77, LZ78}中的一种
    if (compress == 1) { // 压缩
      vector<char> src(inBuffer, inBuffer + inSize);
      if (method == 1) { // LZ77
        printf("Compress + LZ77\n");
        vector<Lz77OutputUnit> dst;
        int outLen = compressLz77(src, dst, searchBufLen, lookAheadBufLen);
        char *outBuffer = new char[sizeof(Lz77OutputUnit) * outLen];
        int len = 0;
        for (auto u : dst)
          len += u.write(outBuffer + len);
        ofstream outFile(output_file, ofstream::out | ofstream::binary);
        outFile.write(outBuffer, len);
        outFile.close();
        delete[] outBuffer;
        } else if (method == 2) { // LZ78
            printf("Compress + LZ78\n");
            vector<Lz78OutputUnit> dst;
            int outLen = compressLz78(src, dst, dictSize);
            char *outBuffer = new char[sizeof(Lz78OutputUnit) * outLen];
            int len = 0;
            for (auto u : dst)
                len += u.write(outBuffer + len);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(outBuffer, len);
            outFile.close();
            delete[] outBuffer;
        } else if (method == 3) { // LZ77 parallel
            printf("Compress + LZ77 parallel\n");
            Lz77ParallelResult dst;
            int outLen = parallel_compressLz77(n_thread, src, dst, searchBufLen, lookAheadBufLen);
            char *outBuffer = new char[sizeof(int) + sizeof(int) * outLen + sizeof(Lz77OutputUnit) * outLen];
            memcpy(outBuffer, &outLen, sizeof(outLen));
            int len = sizeof(outLen);
            for (auto u : dst.lens) {
                memcpy(outBuffer + len, &u, sizeof(u));
                len += sizeof(u);
            }
            for (auto v : dst.blocks)
                for (auto u : v)
                    len += u.write(outBuffer + len);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(outBuffer, len);
            outFile.close();
            delete[] outBuffer;
        } else if (method == 4) { // LZW
            printf("Compress + LZW\n");
            vector<LzWOutputUnit> dst;
            int outLen = compressLzW(src, dst, dictSize);
            char *outBuffer = new char[sizeof(LzWOutputUnit) * outLen];
            int len = 0;
            for (auto u : dst)
                len += u.write(outBuffer + len);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(outBuffer, len);
            outFile.close();
            delete[] outBuffer;
        }
    } else if (compress == 2) { // 解压
        if (method == 1) { // LZ77
            printf("Decompress + LZ77\n");
            vector<Lz77OutputUnit> src;
            int pos = 0;
            while (pos < inSize) {
                Lz77OutputUnit tmp;
                pos += tmp.read(inBuffer + pos);
                src.push_back(tmp);
            }
            vector<char> dst;
            decompressLz77(src, dst, searchBufLen, lookAheadBufLen);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(dst.data(), dst.size());
            outFile.close();
        } else if (method == 2) { // LZ78
            printf("Decompress + LZ78\n");
            vector<Lz78OutputUnit> src;
            int pos = 0;
            while (pos < inSize) {
                Lz78OutputUnit tmp(0, 0);
                pos += tmp.read(inBuffer + pos);
                src.push_back(tmp);
            }
            vector<char> dst;
            decompressLz78(src, dst, dictSize);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(dst.data(), dst.size());
            outFile.close();
        } else if (method == 3) { // LZ77 parallel
            printf("Decompress + LZ77 parallel\n");
            Lz77ParallelResult src;
            int pos = 0;
            int srcLen; // LZ77ParallenResult中lens的元素数
            memcpy(&srcLen, inBuffer, sizeof(srcLen));
            pos += sizeof(srcLen);
            for (int i = 0; i < srcLen; i++) {
                int tmp;
                memcpy(&tmp, inBuffer + pos, sizeof(tmp));
                src.lens.push_back(tmp);
                pos += sizeof(tmp);
            }
            for (int i = 0; i < srcLen; i++) {
                for (int j = 0; j < src.lens[i]; j++) {
                    Lz77OutputUnit tmp;
                    pos += tmp.read(inBuffer + pos);
                    src.blocks[j].push_back(tmp);
                }
            }
            vector<char> dst;
            parallel_decompressLz77(src, dst, searchBufLen, lookAheadBufLen);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(dst.data(), dst.size());
            outFile.close();
        } else if (method == 4) { // LZW
            printf("Decompress + LZW\n");
            vector<LzWOutputUnit> src;
            int pos = 0;
            while (pos < inSize) {
                LzWOutputUnit tmp;
                pos += tmp.read(inBuffer + pos);
                src.push_back(tmp);
            }
            vector<char> dst;
            decompressLzW(src, dst, dictSize);
            ofstream outFile(output_file, ofstream::out | ofstream::binary);
            outFile.write(dst.data(), dst.size());
            outFile.close();
        }
    }

    delete[] inBuffer;

    return 0;
}

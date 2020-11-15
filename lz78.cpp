#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "lz78.h"

#define N_SYMBOLS 256

typedef unsigned char sym_t;

/*
 * 字典树
 *
 * 在预先开辟的数组上实现字典树。数组相当于内存，int型数组下标相当于指针。
 */
template <int N>
struct Node {
    int parent; // 父节点指针，这一字段在压缩时不会用到
    sym_t symbol; // 从父节点到当前节点的边上的符号。尽管这一字段可以从父节点计算得到，但保存在此效率更高。这一字段在压缩时不会用到
    int child[N]; // 各子节点指针，去往第i个子节点的边上符号为i
};

int compressLz78(const vector<char> &src, vector<Lz78OutputUnit> &dst, int dictSize) {
    // 建立字典树
    Node<N_SYMBOLS> *tree = new Node<N_SYMBOLS>[dictSize];
    memset(tree, -1, sizeof(Node<N_SYMBOLS>) * dictSize); // 全部指针初始化为-1，表示空指针
    int root = 0;
    int treeSize = 1; // 初始时树中只有根节点

    // 压缩阶段
    int pos = 0; // 当前下标
    while (pos < src.size()) {
        // 在字典树中匹配
        int node = root;
        for (; pos < src.size(); pos++) {
            sym_t c = src[pos];
            if (tree[node].child[c] == -1) break;
            node = tree[node].child[c];
        }

        // 此时node指向应当输出的节点下标，或者说字典下标
        // 但不确定循环是由于输入结束而退出还是因为查到新词而退出
        if (pos < src.size()) { // 因发现新词而退出
            sym_t c = src[pos];
            // 向树中加入新节点，其中parent字段压缩时无用，因此不写
            if (treeSize < dictSize)
                tree[node].child[c] = treeSize++;
            // 输出
            dst.push_back(Lz78OutputUnit(node, c));
            pos++;  // 移动pos
        } else { // 因输入结束而退出
            // 输出
            dst.push_back(Lz78OutputUnit(-node, 0)); // 没有未匹配字符时，输出的index为负值，以此作为特殊标记
        }
    }

    return dst.size();
}

int decompressLz78(const vector<Lz78OutputUnit> &src, vector<char> &dst, int dictSize) {
    // 建立字典树
    Node<N_SYMBOLS> *tree = new Node<N_SYMBOLS>[dictSize];
    memset(tree, -1, sizeof(Node<N_SYMBOLS>) * dictSize); // 全部指针初始化为-1，表示空指针
    int root = 0;
    int treeSize = 1; // 初始时树中只有根节点

    // 解压阶段
    for (int i = 0; i < src.size(); i++) {
        // 首先将codeword逆序输出，因为已知字典下标时只能通过parent字段逆序遍历整个codeword
        int wordLen = 0; // 记录当前codeword的长度
        for (int node = std::abs(src[i].index); node != root; node = tree[node].parent) {
            dst.push_back(tree[node].symbol);
            wordLen++;
        }
        // 然后再将刚刚输出的codeword逆转过来
        reverse(dst.end() - wordLen, dst.end());
        // 有未匹配字符时，在输出末尾添加未匹配字符
        if (src[i].index >= 0) {
            dst.push_back(src[i].symbol);
            // 更新字典树，插入新节点
            if (treeSize < dictSize) {
                int newnode = treeSize++;
                int parent = std::abs(src[i].index); // 即将成为父节点的节点
                sym_t c = src[i].symbol; // 未匹配字符
                tree[parent].child[c] = newnode;
                tree[newnode].parent = parent;
                tree[newnode].symbol = c;
            }
        }
        // 无未匹配字符时，输入也应当遍历结束了
    }

    return dst.size();
}

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "lz78.h"

/*
 * 字典树
 *
 * 在预先开辟的数组上实现字典树。数组相当于内存，int型数组下标相当于指针。
 */
template <int N>
struct Node {
    int parent; // 父节点指针，这一字段在压缩时不会用到
    int symbol; // 从父节点到当前节点的边上的符号。尽管这一字段可以从父节点计算得到，但保存在此效率更高。这一字段在压缩时不会用到
    int child[N]; // 各子节点指针，去往第i个子节点的边上符号为i
};

int compressLz78(const char *src, int srcLen, Lz78OutputUnit *dst, int dstMaxLen, int dictSize) {
    // 检验输入合法性，既不为0也不为1时返回-2。见函数文档。
    for (int i = 0; i < srcLen; i++)
        if (src[i] != 0 && src[i] != 1)
            return -2;

    int len = 0;

    // 建立字典树
    Node<2> *tree = new Node<2>[dictSize]; // 目前仅实现2-ary输入上的压缩
    memset(tree, -1, sizeof(Node<2>) * dictSize); // 全部指针初始化为-1，表示空指针
    int root = 0;
    int treeSize = 1; // 初始时树中只有根节点

    // 压缩阶段
    int pos = 0; // 当前下标
    while (pos < srcLen) {
        if (len >= dstMaxLen) return -1; // 检查输出缓冲区是否已满

        // 在字典树中匹配
        int node = root;
        for (; pos < srcLen; pos++) {
            char c = src[pos];
            if (tree[node].child[c] == -1) break;
            node = tree[node].child[c];
        }

        // 此时node指向应当输出的节点下标，或者说字典下标
        // 但不确定循环是由于输入结束而退出还是因为查到新词而退出
        if (pos < srcLen) { // 因发现新词而退出
            char c = src[pos];
            // 向树中加入新节点，其中parent字段压缩时无用，因此不写
            if (treeSize < dictSize)
                tree[node].child[c] = treeSize++;
            // 输出
            dst[len].index = node;
            dst[len].symbol = c;
            len++;
            // 移动pos
            pos++;
        } else { // 因输入结束而退出
            // 输出
            dst[len].index = node;
            dst[len].symbol = -1; // 使用特殊值-1表示此处没有输出符号
            len++;
        }
    }

    return len;
}

int decompressLz78(const Lz78OutputUnit *src, int srcLen, char *dst, int dstMaxLen, int dictSize) {
    int len = 0; // 已输出的长度

    // 建立字典树
    Node<2> *tree = new Node<2>[dictSize]; // 目前仅实现2-ary输入上的压缩
    memset(tree, -1, sizeof(Node<2>) * dictSize); // 全部指针初始化为-1，表示空指针
    int root = 0;
    int treeSize = 1; // 初始时树中只有根节点

    // 解压阶段
    for (int i = 0; i < srcLen; i++) {
        // 首先将codeword逆序输出，因为已知字典下标时只能通过parent字段逆序遍历整个codeword
        int wordLen = 0; // 记录当前codeword的长度
        for (int node = src[i].index; node != root; node = tree[node].parent) {
            if (len >= dstMaxLen) return -1; // 检查输出缓冲区是否已满
            dst[len++] = tree[node].symbol;
            wordLen++;
        }
        // 然后再将刚刚输出的codeword逆转过来
        std::reverse(dst + len - wordLen, dst + len);
        // 有未匹配字符时，在输出末尾添加未匹配字符
        if (src[i].symbol != -1) {
            if (len >= dstMaxLen) return -1; // 检查输出缓冲区是否已满
            dst[len++] = src[i].symbol;
            // 更新字典树，插入新节点
            if (treeSize < dictSize) {
                int newnode = treeSize++;
                int parent = src[i].index; // 即将成为父节点的节点
                int c = src[i].symbol; // 未匹配字符
                tree[parent].child[c] = newnode;
                tree[newnode].parent = parent;
                tree[newnode].symbol = c;
            }
        }
        // 无未匹配字符时，输入也应当遍历结束了
    }

    return len;
}
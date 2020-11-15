#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "lzw.h"

using std::vector;

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

int compressLzW(const vector<char> &src, vector<LzWOutputUnit> &dst, int dictSize) {
    // 检验输入合法性，既不为0也不为1时返回-2。见函数文档。
    for (int i = 0; i < src.size(); i++)
        if (src[i] != 0 && src[i] != 1)
            return -2;

    // 建立字典树
    Node<2> *tree = new Node<2>[dictSize]; // 目前仅实现2-ary输入上的压缩
    memset(tree, -1, sizeof(Node<2>) * dictSize); // 全部指针初始化为-1，表示空指针
    int root = 0;
    for(int i = 0; i < 2; ++i){ //初始化一个带N个单字符的字典树。
        tree[0].child[i] = i+1;
        tree[i+1].parent = 0;
        tree[i+1].symbol = i;
    }
    int treeSize = 1 + 2; // 初始时树中只有根节点，和初始化的N个节点，共N+1个节点

    // 压缩阶段
    int pos = 0; // 当前下标
    while (pos < src.size()) {
        // 在字典树中匹配
        int node = root;
        for (; pos < src.size(); pos++) {
            char c = src[pos];
            if (tree[node].child[int(c)] == -1) break;
            node = tree[node].child[int(c)];
        }
        // 此时node指向应当输出的节点下标，或者说字典下标
        LzWOutputUnit newUnit;
        newUnit.index = node; // 将字典中找到的最长串的index输出
        dst.push_back(newUnit);

        // 但之前不确定循环是由于输入结束而退出还是因为查到新词而退出
        // 如果是查到新词而退出，那么需要把新词加到字典里去
        if (pos < src.size()) { // 因发现新词而退出
            char c = src[pos];
            // 向树中加入新节点
            if (treeSize < dictSize){
                tree[node].child[int(c)] = treeSize++;
                tree[treeSize-1].parent = node;
                tree[treeSize-1].symbol = c;
            }
        }
        // 如果是输入结束了，就结束了。
    }
    return dst.size();
}



int decompressLzW(const vector<LzWOutputUnit> &src, vector<char> &dst, int dictSize) {
    // 建立字典树
    Node<2> *tree = new Node<2>[dictSize]; // 目前仅实现2-ary输入上的压缩
    memset(tree, -1, sizeof(Node<2>) * dictSize); // 全部指针初始化为-1，表示空指针
    int root = 0;
    for(int i = 0; i < 2; ++i){ //初始化一个带N个单字符的字典树。
        tree[0].child[i] = i+1;
        tree[i+1].parent = 0;
        tree[i+1].symbol = i;
    }
    int treeSize = 1 + 2; // 初始时树中只有根节点，和初始化的N个节点，共N+1个节点

    // 解压阶段
    int last_node = 0;
    for (int i = 0; i < src.size(); i++) {
        // 首先将codeword逆序输出，因为已知字典下标时只能通过parent字段逆序遍历整个codeword
        int wordLen = 0; // 记录当前codeword的长度
        char first_char = 0; // 记录当前codeword的第一个字符

        int node;
        if (src[i].index >= treeSize){ // 如果当前node没在字典里面找到, 那么该node的字符串对应上一个node的加上一个新字符
            node = last_node;
        }else{
            node = src[i].index;
        }
        for (; node != root; node = tree[node].parent) {
            dst.push_back(tree[node].symbol);
            first_char = tree[node].symbol;
            wordLen++;
        }
        // 然后再将刚刚输出的codeword逆转过来
        std::reverse(dst.end() - wordLen, dst.end());
        if (src[i].index >= treeSize){
            dst.push_back(first_char);
        }

        if (last_node != 0) { // 不是第一个节点，前一个codeword加当前codeword的第一个字符，就是新字符串，插入字典树
            if (treeSize < dictSize) { // 如果字典树没有满
                int newnode = treeSize++;
                tree[last_node].child[int(first_char)] = newnode;
                tree[newnode].parent = last_node;
                tree[newnode].symbol = first_char;
            }
        }
        last_node = src[i].index; // 将当前node设置为last_node
    }
    // 无未匹配字符时，输入也应当遍历结束了
    return dst.size();
}

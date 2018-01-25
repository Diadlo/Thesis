#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <array>
#include "words_trie_tree.h"

#ifndef TRIE_NODES_COUNT 
# define TRIE_NODES_COUNT 0
#endif

#ifndef TRIE_TREE_INITIALIZER
# define TRIE_TREE_INITIALIZER {}
#endif

using array_t = std::array<const char*, TRIE_NODES_COUNT>;
constexpr array_t trieTree = TRIE_TREE_INITIALIZER;

int next_line(int id, const array_t& t, const char* str)
{
    for (; t[id].nextId != 0; id = t[id].nextId) {
        if (t[id].letter == *str) {
            return t[id].bottomId;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <word>\n";
        return -1;
    }

    auto str = argv[1];
    int id = trieTree[0].bottomId;
    while (*str != '\0' && !trieTree[id].isLeaf) {
        id = next_line(id, trieTree, str);
        str++;
    }

    auto found = *str == '\0' && trieTree[id].isLeaf;
    if (found) {
        std::cout << "found\n";
    } else {
        std::cout << "not found\n";
    }

    return 0;
}

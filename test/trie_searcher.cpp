#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#include "words_trie_tree.h"

#ifndef TRIE_NODES_COUNT
# define TRIE_NODES_COUNT 0
#endif

#ifndef TRIE_TREE_INITIALIZER
# define TRIE_TREE_INITIALIZER {}
#endif

constexpr std::array<TrieNode, TRIE_NODES_COUNT> trieTree = TRIE_TREE_INITIALIZER;

template<class Array>
int next_line(int id, const Array& array, const char* word)
{
    for (; array[id].nextId != 0; id = array[id].nextId) {
        if (array[id].letter == *word) {
            return array[id].bottomId;
        }
    }

    return array[id].bottomId;
}

template<class Array>
bool find_trie(const Array& array, const char* word)
{
    auto lastIsLeaf = false;
    auto id = array[0].bottomId;
    while (*word != '\0' && id != 0) {
        lastIsLeaf = array[id].isLeaf;
        id = next_line(id, array, word);
        word++;
    }

    return *word == '\0' && lastIsLeaf;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <word>\n";
        return -1;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const auto found = find_trie(trieTree, argv[1]);
    const auto finish = std::chrono::high_resolution_clock::now();

    const auto duration = finish - start;
    std::cout << duration.count() << " ticks\n";

    if (found) {
        std::cout << "found\n";
    } else {
        std::cout << "not found\n";
    }

    return 0;
}

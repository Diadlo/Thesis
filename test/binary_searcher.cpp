#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#include "words_array.h"

#ifndef WORDS_COUNT
# define WORDS_COUNT 0
#endif

#ifndef WORDS_INITIALIZER
# define WORDS_INITIALIZER {}
#endif

constexpr std::array<const char*, WORDS_COUNT> words = WORDS_INITIALIZER;

template<class Array>
bool find_binary(const Array& array, const char* word)
{
    return std::binary_search(words.begin(), words.end(), word,
            [](const char* s1, const char* s2) {
                return std::strcmp(s1, s2) < 0;
            });
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <word>\n";
        return -1;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const auto found = find_binary(words, argv[1]);
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

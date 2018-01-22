#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <array>
#include "words.h"

#ifndef WORDS_INITIALIZER
# define WORDS_INITIALIZER {}
#endif

constexpr std::array<const char*, 2> a = {
    "1", "2"
};

std::vector<const char*> words = WORDS_INITIALIZER;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <word>\n";
        return -1;
    }

    auto found = std::binary_search(words.begin(), words.end(), argv[1], 
            [](const char* s1, const char* s2) {
                return std::strcmp(s1, s2) < 0;
            });

    if (found) {
        std::cout << "found\n";
    } else {
        std::cout << "not found\n";
    }

    return 0;
}

#include <iostream>
#include <vector>
#include "words.h"

#ifndef WORDS_INITIALIZER
# define WORDS_INITIALIZER {}
#endif

std::vector<const char*> words = WORDS_INITIALIZER;

int main()
{
    for (const auto& word : words) {
        std::cout << word << '\n';
    }

    return 0;
}

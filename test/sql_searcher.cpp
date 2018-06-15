#include <chrono>
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <vector>

#include "sqlite_db.h"

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <db_filename> <word>\n";
        return -1;
    }

    const auto db_filename = argv[1];
    auto db = sqlite_db(db_filename);
    if (!db) {
        std::cerr << "Error opening database: " << db.error();
        return -1;
    }

    const auto word = std::string(argv[2]);
    const auto start = std::chrono::high_resolution_clock::now();
    const auto found = db.contains("words", "word", word);
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

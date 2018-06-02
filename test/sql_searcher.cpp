#include <chrono>
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <vector>

bool find_word(sqlite3* db, const std::string& word)
{
    constexpr auto query = "SELECT word FROM words WHERE word = ?;";
    sqlite3_stmt* res;
    auto rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_text(res, 1, word.c_str(), word.size(), nullptr);

    rc = sqlite3_step(res);
    const auto find = rc == SQLITE_ROW;
    sqlite3_finalize(res);
    return find;
}

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <word>\n";
        return -1;
    }

    sqlite3* db;
    auto rc = sqlite3_open_v2("words.db", &db,
            SQLITE_OPEN_READWRITE |
            SQLITE_OPEN_CREATE |
            SQLITE_OPEN_NOMUTEX, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db);
        return -1;
    }

    const auto word = std::string(argv[1]);
    const auto start = std::chrono::high_resolution_clock::now();
    const auto found = find_word(db, word);
    const auto finish = std::chrono::high_resolution_clock::now();

    const auto duration = finish - start;
    std::cout << duration.count() << " ticks\n";
    if (found) {
        std::cout << "found\n";
    } else {
        std::cout << "not found\n";
    }

    rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error closing database: " << sqlite3_errmsg(db);
        return -1;
    }

    return 0;
}

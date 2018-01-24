#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

std::vector<std::string> load_words(const std::string& file)
{
    std::ifstream in(file);
    auto str = std::string{};
    auto words = std::vector<std::string>{};
    while (in >> str) {
        words.push_back(str);
    }

    return words;
}

void insert_words(sqlite3* db, std::vector<std::string>::const_iterator begin, 
        std::vector<std::string>::const_iterator end)
{
    auto ss = std::stringstream{};
    ss << "INSERT INTO words(word) VALUES ";
    for (auto it = begin; it != end; ++it) {
        ss << "(?),";
    }
    ss.seekp(-1, std::ios_base::end);
    ss << ";";

    const auto query = ss.str();
    sqlite3_stmt* stmt;
    auto rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't prepare (" << sqlite3_errmsg(db) << ")\n";
        return;
    }

    int i = 1;
    for (auto it = begin; it != end; ++it) {
        auto word = it->c_str();
        sqlite3_bind_text(stmt, i, word, -1, NULL);
        i++;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Can't execute (" << sqlite3_errmsg(db) << ")\n";
    }

    sqlite3_finalize(stmt);
}

constexpr auto WORDS_PER_INSERT = 2000;

int main()
{
    sqlite3* db;
    auto rc = sqlite3_open_v2("words.db", &db,
            SQLITE_OPEN_READWRITE | 
            SQLITE_OPEN_CREATE | 
            SQLITE_OPEN_NOMUTEX, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db);
        return -1;
    }

    const auto words = load_words("words.txt");
    const auto size = words.size();
    std::cout << size << " words loaded\n";
    auto it = words.begin();
    for (int i = 0; i < size / WORDS_PER_INSERT; i++) {
        auto end = it + WORDS_PER_INSERT;
        insert_words(db, it, end);
        it = end;
    }

    insert_words(db, it, words.end());

    rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error closing database: " << sqlite3_errmsg(db);
        return -1;
    }

    return 0;
}

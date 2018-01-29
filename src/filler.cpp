#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

void create_database(sqlite3* db)
{
    const auto query = "CREATE TABLE IF NOT EXISTS words("
        "id INT PRIMARY KEY, "
        "word TEXT UNIQUE);";

    sqlite3_stmt* stmt;
    auto rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't prepare CREATE (" << sqlite3_errmsg(db) << ")\n";
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Can't execute CREATE (" << sqlite3_errmsg(db) << ")\n";
    }

    sqlite3_finalize(stmt);
}

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
    ss << "INSERT OR IGNORE INTO words(word) VALUES ";
    for (auto it = begin; it != end; ++it) {
        ss << "(?),";
    }
    ss.seekp(-1, std::ios_base::end);
    ss << ";";

    const auto query = ss.str();
    sqlite3_stmt* stmt;
    auto rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't prepare INSERT (" << sqlite3_errmsg(db) << ")\n";
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
        std::cerr << "Can't execute INSERT (" << sqlite3_errmsg(db) << ")\n";
    }

    sqlite3_finalize(stmt);
}

constexpr auto WORDS_PER_INSERT = 2000;

int main(int argc, const char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <words.txt> <db>\n";
        return -1;
    }

    sqlite3* db;
    auto words_txt = argv[1];
    auto words_db = argv[2];
    auto rc = sqlite3_open_v2(words_db, &db,
            SQLITE_OPEN_READWRITE | 
            SQLITE_OPEN_CREATE | 
            SQLITE_OPEN_NOMUTEX, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db);
        return -1;
    }

    create_database(db);
    const auto words = load_words(words_txt);
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

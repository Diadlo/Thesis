#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <vector>

std::vector<std::string> load_words(sqlite3* db)
{
    auto words = std::vector<std::string>{};
    constexpr auto query = "SELECT word FROM words ORDER BY word;";
    sqlite3_stmt* res;
    auto rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    if (rc != SQLITE_OK) {
        return {};
    }

    rc = sqlite3_step(res);
    while (rc == SQLITE_ROW) {
        const auto text = sqlite3_column_text(res, 0);
        const auto word = std::string{reinterpret_cast<const char*>(text)};
        words.push_back(word);
        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res);
    return words;
}

void replace(std::string& str, const std::string& before, const std::string& after) {
    auto start_pos = str.find(before);
    if (start_pos != std::string::npos) {
        str.replace(start_pos, before.length(), after);
    }
}

void write_def_array(const std::string& filename, const std::vector<std::string>& words)
{
    std::ofstream out(filename);
    out << "#define WORDS_COUNT " << words.size() << "\n";
    out << "#define WORDS_INITIALIZER { \\\n";
    for (auto& word : words) {
        auto outword = word;
        replace(outword, "\\", "\\\\");
        replace(outword, "\"", "\\\"");
        out << "\"" << outword << "\", \\\n";
    }

    out << "}\n";
}

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

    auto words = load_words(db);
    std::cout << words.size() << " words loaded\n";
    write_def_array("words.h", words);

    rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error closing database: " << sqlite3_errmsg(db);
        return -1;
    }

    return 0;
}

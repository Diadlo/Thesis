#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "trietree.h"

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
    out << "#pragma once\n";
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

void write_trie_array(const std::string& filename, const std::vector<std::string>& words)
{
    std::ofstream out(filename);
    TrieTree t;
    for (const auto& word : words) {
        t.insert(word);
    }

    auto nodes = t.getNodes();
    out << "#pragma once\n"
        << "struct TrieNode { \n"
        << "    char letter;\n"
        << "    bool isLeaf;\n"
        << "    int id, nextId, bottomId;\n"
        << "};\n"
        << "#define TRIE_NODES_COUNT " << nodes.size() << "\n"
        << "#define TRIE_TREE_INITIALIZER { \\\n";
    for (auto node : nodes) {
        const auto letter = node->letter;
        const auto sLetter = [letter]() {
            switch (letter)
            {
                case '\0':
                    return std::string("\\0");
                case '\'':
                    return std::string("\\'");
                default:
                    return std::string(1, letter);
            }
        }();

        out << "{"
            << "'" << sLetter << "', "
            << (node->isLeaf ? "true" : "false") << ", "
            << node->id << ", "
            << node->nextId << ", "
            << node->bottomId 
            << "},\\\n";
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
    write_def_array("words_array.h", words);
    write_trie_array("words_trie_tree.h", words);

    rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error closing database: " << sqlite3_errmsg(db);
        return -1;
    }

    return 0;
}

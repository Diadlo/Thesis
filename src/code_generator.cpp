#include <iostream>
#include <fstream>
#include <vector>
#include "trietree.h"
#include "sqlite_db.h"

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
//        << "    int id;\n"
        << "    int nextId;\n"
        << "    int bottomId;\n"
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

        out << "TrieNode{"
            << "'" << sLetter << "', "
            << (node->isLeaf ? "true" : "false") << ", "
//            << node->id << ", "
            << node->nextId << ", "
            << node->bottomId
            << "},\\\n";
    }
    out << "}\n";
}

int generate(const std::string& words_db,
             const std::string& array_filename,
             const std::string& tree_filename)
{
    sqlite_db db(words_db);
    if (!db) {
        std::cerr << "Error opening database: " << db.error();
        return -1;
    }

    const auto words = db.select_sorted_column("words", table_column::data_type::TEXT, "word");
    if (words.empty()) {
        std::cerr << "Can't load words: " << db.error();
        return -1;
    }

    std::cout << words.size() << " words loaded\n";
    write_def_array(array_filename, words);
    write_trie_array(tree_filename, words);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <db> <array output> "
            "<trie tree output>\n";
        return -1;
    }

    const auto words_db = argv[1];
    const auto array_filename = argv[2];
    const auto tree_filename = argv[3];
    return generate(words_db, array_filename, tree_filename);
}

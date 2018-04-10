#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>

void create_table(sqlite3* db)
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

template<class It>
void insert_words(sqlite3* db, It begin, It end)
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
        auto word = (*it).c_str();
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
constexpr auto MAX_LEN = 5;
constexpr auto LET_COUNT = 'z' - 'a';

class WordsGenerator
{
public:
    class Iterator
    {
    public:
        Iterator(int count, int words_per_count, int word_size, char* buf)
            : count{count}
            , words_per_count{words_per_count}
            , word_size{word_size}
            , buf{buf}
        {
        }

        Iterator& operator++()
        {
            count++;
            return *this;
        }

        Iterator operator+(int value)
        {
            auto it = *this;
            it.count += value;
            return it;
        }

        Iterator& operator*()
        {
            return *this;
        }

        bool operator!=(const Iterator& other)
        {
            return count != other.count;
        }

        const char* c_str() const
        {
            return buf + (count % words_per_count) * word_size;
        }

    public:
        int count;
        int words_per_count;
        int word_size;
        mutable char* buf;
    };

    WordsGenerator(int max_count, int max_length, int words_per_count)
        : max_count{max_count}
        , size{max_length + 1}
        , words_per_count{words_per_count}
        , buf{new char[words_per_count * size]}
    {
        generate();
    }

    ~WordsGenerator()
    {
        delete buf;
    }

    Iterator begin() const
    {
        return Iterator(0, words_per_count, size, buf);
    }

    Iterator end() const
    {
        return Iterator(max_count, words_per_count, size, nullptr);
    }

    void generate()
    {
        for (auto i = 0; i < words_per_count * size; i++) {
            if ((i + 1) % size == 0) {
                buf[i] = 0;
            } else {
                buf[i] = (std::rand() % LET_COUNT) + 'a';
            }
        }
    }


private:
    int max_count;
    int size;
    int words_per_count;
    char* buf;
};

int main(int argc, const char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <count> <db>\n";
        return -1;
    }

    sqlite3* db;
    auto count = atoi(argv[1]);
    auto words_db = argv[2];
    auto rc = sqlite3_open_v2(words_db, &db,
            SQLITE_OPEN_READWRITE | 
            SQLITE_OPEN_CREATE | 
            SQLITE_OPEN_NOMUTEX, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db);
        return -1;
    }

    create_table(db);
    std::srand(unsigned(std::time(0)));
    //const auto words = load_words(words_txt);
    //const auto size = words.size();
    //std::cout << size << " words loaded\n";
    auto words = WordsGenerator(count, MAX_LEN, WORDS_PER_INSERT);
    auto it = words.begin();
    for (int i = 0; i < count / WORDS_PER_INSERT; i++) {
        std::cout << i << "/" << count / WORDS_PER_INSERT << std::endl;
        auto end = it + WORDS_PER_INSERT;
        insert_words(db, it, end);
        words.generate();
        it = end;
    }

    if (count % WORDS_PER_INSERT != 0) {
        insert_words(db, it, words.end());
    }

    rc = sqlite3_close(db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error closing database: " << sqlite3_errmsg(db);
        return -1;
    }

    return 0;
}

#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>

constexpr auto WORDS_PER_INSERT = 2000;
constexpr auto MAX_LEN = 5;
constexpr auto LET_COUNT = 'z' - 'a';

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

class WordsGenerator
{
public:
    class Iterator
    {
    public:
        Iterator(int count, int words_per_count, int word_size, char* buf)
            : m_count{count}
            , m_words_per_count{words_per_count}
            , m_word_size{word_size}
            , m_buf{buf}
        {
            std::srand(unsigned(std::time(0)));
        }

        Iterator& operator++()
        {
            m_count++;
            return *this;
        }

        Iterator operator+(int value)
        {
            auto it = *this;
            it.m_count += value;
            return it;
        }

        Iterator& operator*()
        {
            return *this;
        }

        bool operator!=(const Iterator& other)
        {
            return m_count != other.m_count;
        }

        const char* c_str() const
        {
            return m_buf + (m_count % m_words_per_count) * m_word_size;
        }

    public:
        int m_count;
        int m_words_per_count;
        int m_word_size;
        mutable char* m_buf;
    };

    WordsGenerator(int max_count, int max_length, int words_per_count)
        : m_max_count{max_count}
        , m_size{max_length + 1}
        , m_words_per_count{words_per_count}
        , m_buf{new char[m_words_per_count * m_size]}
    {
        generate();
    }

    ~WordsGenerator()
    {
        delete m_buf;
    }

    Iterator begin() const
    {
        return Iterator(0, m_words_per_count, m_size, m_buf);
    }

    Iterator end() const
    {
        return Iterator(m_max_count, m_words_per_count, m_size, nullptr);
    }

    void generate()
    {
        for (auto i = 0; i < m_words_per_count * m_size; i++) {
            if ((i + 1) % m_size == 0) {
                m_buf[i] = 0;
            } else {
                m_buf[i] = (std::rand() % LET_COUNT) + 'a';
            }
        }
    }

    int size() const
    {
        return m_max_count;
    }

private:
    int m_max_count;
    int m_size;
    int m_words_per_count;
    char* m_buf;
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

    std::cout << "Count = " << count << "\n";
    std::cout << "DB = " << words_db << "\n";
    create_table(db);
    //const auto words = load_words("words.txt");
    auto words = WordsGenerator(count, MAX_LEN, WORDS_PER_INSERT);

    std::cout << words.size() << " words loaded\n";
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

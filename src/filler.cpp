#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "sqlite_db.h"

constexpr auto WORDS_PER_INSERT = 2000;
constexpr auto MAX_LEN = 5;
constexpr auto LET_COUNT = 'z' - 'a';

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

class words_generator
{
public:
    class iterator
    {
    public:
        iterator(int count, int word_size)
            : m_count{count}
            , m_word_size{word_size}
        {
        }

        iterator& operator++()
        {
            m_count++;
            return *this;
        }

        iterator operator+(int value)
        {
            auto it = *this;
            it.m_count += value;
            return it;
        }

        int operator-(const iterator& it)
        {
            return m_count - it.m_count;
        }

        std::string operator*()
        {
            auto randchar = []() -> char
            {
                const char charset[] = "abcdefghijklmnopqrstuvwxyz";
                const auto max_index = (sizeof(charset) - 1);
                return charset[rand() % max_index];
            };
            std::string str(m_word_size, 0);
            std::generate_n(str.begin(), m_word_size, randchar);
            return str;
        }

        bool operator!=(const iterator& other)
        {
            return m_count != other.m_count;
        }

    public:
        int m_count;
        int m_word_size;
    };

    words_generator(int max_count, int max_length, int words_per_count)
        : m_max_count{max_count}
        , m_size{max_length + 1}
    {
    }

    iterator begin() const
    {
        return iterator(0, m_size);
    }

    iterator end() const
    {
        return iterator(m_max_count, m_size);
    }

    int size() const
    {
        return m_max_count;
    }

private:
    int m_max_count;
    int m_size;
};

int fill(int count, const std::string& words_db)
{
    auto db = sqlite_db{words_db};
    if (!db) {
        std::cerr << "Error opening database: " << db.error();
        return -1;
    }

    using dt = table_column::data_type;
    using cf = table_column::column_flag;
    db.create_table("words", {
        { "id",   dt::INT,  cf::PK },
        { "word", dt::TEXT, cf::UNIQUE },
    });

    //const auto words = load_words("words.txt");
    auto words = words_generator(count, MAX_LEN, WORDS_PER_INSERT);

    std::cout << words.size() << " words loaded\n";
    auto res = db.insert_rows("words", {"word"}, words.begin(), words.end());
    if (!res) {
        std::cerr << "Can't insert words: " << db.error();
        return -1;
    }

    return 0;
}

int main(int argc, const char* argv[])
{
    std::srand(unsigned(std::time(0)));
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <count> <db>\n";
        return -1;
    }

    auto count = atoi(argv[1]);
    auto words_db = argv[2];
    return fill(count, words_db);
}

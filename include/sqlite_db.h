#ifndef _SQLITE_DB_H_
#define _SQLITE_DB_H_

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

#include <sqlite3.h>

struct table_column
{
    enum data_type
    {
        INT,
        TEXT
    };

    enum column_flag
    {
        PK = 1,
        UNIQUE = 2
    };

    std::string name;
    data_type type;
    column_flag flags;

    std::string get_string() const;
};

class sqlite_db
{
public:
    sqlite_db(const std::string& filename);
    ~sqlite_db();

    bool create_table(const std::string& name, const std::vector<table_column>& columns);
    std::vector<std::string> select_sorted_column(const std::string& table,
        table_column::data_type type, const std::string& column);
    bool contains(const std::string& table, const std::string& column, const std::string& value);

    std::string error() const;
    bool operator!() const;

    template<class It>
    bool insert_rows(const std::string& table_name,
            const std::vector<std::string>& columns,
            It begin, It end)
    {
        constexpr auto WORDS_PER_INSERT = 2000;
        const auto count = end - begin;
        auto it = begin;
        for (int i = 0; i < count / WORDS_PER_INSERT; i++) {
            auto range_end = it + WORDS_PER_INSERT;
            auto res = insert_rows_range(table_name, columns, it, range_end);
            if (!res) {
                return false;
            }

            it = range_end;
        }

        if (count % WORDS_PER_INSERT != 0) {
            auto res = insert_rows_range(table_name, columns, it, end);
            if (!res) {
                return false;
            }
        }

        return true;
    }

    template<class It>
    bool insert_rows_range(const std::string& table_name,
            const std::vector<std::string>& columns, It begin, It end)
    {
        auto ss = std::stringstream{};
        ss << "INSERT OR IGNORE INTO " << table_name << "(";
        std::copy(columns.begin(), columns.end(),
            std::ostream_iterator<std::string>(ss, ","));

        // Remove last comma
        ss.seekp(-1, std::ios_base::end);
        ss << ") VALUES ";
        for (auto it = begin; it != end; ++it) {
            ss << "(?),";
        }

        // Remove last comma
        ss.seekp(-1, std::ios_base::end);
        ss << ";";

        const auto query = ss.str();
        sqlite3_stmt* stmt;
        auto rc = sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            return false;
        }

        int i = 1;
        auto v = std::vector<std::string>{};
        v.reserve(end - begin);
        for (auto it = begin; it != end; ++it) {
            v.push_back(*it);
            auto word = v.back().c_str();
            sqlite3_bind_text(stmt, i, word, -1, NULL);
            i++;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }

private:
    sqlite3* m_db = nullptr;
    int m_res = SQLITE_OK;
};

#endif // _SQLITE_DB_H_

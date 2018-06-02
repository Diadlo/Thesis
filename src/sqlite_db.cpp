#include "sqlite_db.h"
#include <map>

std::string get_type_name(table_column::data_type type)
{
    const auto map = std::map<table_column::data_type, std::string> {
        { table_column::data_type::INT,  "INT" },
        { table_column::data_type::TEXT, "TEXT" },
    };
    return map.at(type);
}

std::string table_column::get_string() const
{
    using cf = table_column::column_flag;
    const auto SPACE = std::string{" "};
    const auto PK_TEXT = std::string{"PRIMARY KEY "};
    const auto UNIQUE_TEXT = std::string{"UNIQUE "};
    const auto is_pk = (flags & cf::PK) == cf::PK;
    const auto is_unique = (flags & cf::UNIQUE) == cf::UNIQUE;
    return name + SPACE +
        get_type_name(type) + SPACE +
        (is_pk     ? PK_TEXT     : SPACE) +
        (is_unique ? UNIQUE_TEXT : SPACE);
}

sqlite_db::sqlite_db(const std::string& filename)
{
    m_res = sqlite3_open_v2(filename.c_str(), &m_db,
            SQLITE_OPEN_READWRITE |
            SQLITE_OPEN_CREATE |
            SQLITE_OPEN_NOMUTEX, nullptr);
}

template<class T>
void join(T t, std::ostringstream& ss, const std::string& sep)
{
    std::copy(t.begin(), t.end(), std::ostream_iterator<std::string>(ss, sep.c_str()));
    // Remove last comma
    ss.seekp(-1, std::ios_base::end);
}

bool sqlite_db::create_table(const std::string& name,
        const std::vector<table_column>& columns)
{
    std::ostringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS " << name << "(";
    auto lines = std::vector<std::string>{};
    std::transform(columns.begin(), columns.end(), std::back_inserter(lines),
            [](const table_column& c) {
                return c.get_string();
    });

    join(lines, ss, ",");
    ss << ");";

    const auto query = ss.str();
    sqlite3_stmt* stmt;
    m_res = sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, 0);
    if (m_res != SQLITE_OK) {
        return false;
    }

    m_res = sqlite3_step(stmt);
    if (m_res != SQLITE_DONE) {
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

sqlite_db::~sqlite_db()
{
    m_res = sqlite3_close(m_db);
}

std::string sqlite_db::error() const
{
    return sqlite3_errmsg(m_db);
}

bool sqlite_db::operator!() const
{
    return m_res != SQLITE_OK;
}

std::vector<std::string> sqlite_db::select_sorted_column(
        const std::string& table,
        table_column::data_type type,
        const std::string& column)
{
    auto words = std::vector<std::string>{};
    std::ostringstream ss;
    ss << "SELECT " << column << " FROM " << table << " ORDER BY " << column << ";";

    const auto query = ss.str();
    sqlite3_stmt* res;
    auto rc = sqlite3_prepare_v2(m_db, query.c_str(), -1, &res, 0);
    if (rc != SQLITE_OK) {
        return {};
    }

    rc = sqlite3_step(res);
    while (rc == SQLITE_ROW) {
        switch (type) {
        case table_column::data_type::INT: {
            const auto num = sqlite3_column_int(res, 0);
            words.push_back(std::to_string(num));
            break;
        }
        case table_column::data_type::TEXT: {
            const auto text = sqlite3_column_text(res, 0);
            words.push_back(std::string{reinterpret_cast<const char*>(text)});
            break;
        }
        }

        rc = sqlite3_step(res);
    }

    sqlite3_finalize(res);
    return words;
}


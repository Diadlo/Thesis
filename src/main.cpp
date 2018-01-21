#include <sqlite3.h>
#include <iostream>

int main()
{
    sqlite3* sqlite;
    auto res = sqlite3_open_v2("words.db", &sqlite, 
            SQLITE_OPEN_READWRITE | 
            SQLITE_OPEN_CREATE | 
            SQLITE_OPEN_NOMUTEX, nullptr);
    if (res != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(sqlite);
        return -1;
    }

    res = sqlite3_close(sqlite);
    if (res != SQLITE_OK) {
        std::cerr << "Error closing database: " << sqlite3_errmsg(sqlite);
        return -1;
    }

    return 0;
}

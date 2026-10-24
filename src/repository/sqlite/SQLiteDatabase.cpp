
#include "repository/sqlite/SQLiteDatabase.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <stdexcept>
#include <utility>

namespace lms::repository::sqlite {

SQLiteDatabase::SQLiteDatabase(const std::string& databasePath)
: databasePath_(databasePath),
      connection_(
        databasePath,
        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
      ) {
    if (databasePath.empty()) {
        throw std::invalid_argument(
            "Database path must not be empty."
        );
    }

    // Enforce foreign-key relationships.
    connection_.exec("PRAGMA foreign_keys = ON;");

    // Wait up to 5 seconds if another connection holds a lock.
    connection_.exec("PRAGMA busy_timeout = 5000;");
}

SQLite::Database& SQLiteDatabase::connection() noexcept {
    return connection_;
}

const std::string& SQLiteDatabase::path() const noexcept {
    return databasePath_;
}

void SQLiteDatabase::execute(const std::string& sql) {
    connection_.exec(sql);
}

} // namespace lms::repository::sqlite

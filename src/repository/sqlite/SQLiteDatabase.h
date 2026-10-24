#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

namespace lms::repository::sqlite {

class SQLiteDatabase {
public:
    explicit SQLiteDatabase(const std::string& databasePath);

    ~SQLiteDatabase() = default;

    // Prevent accidental copying of the database connection.
    SQLiteDatabase(const SQLiteDatabase&) = delete;
    SQLiteDatabase& operator=(const SQLiteDatabase&) = delete;

    // Access the underlying SQLiteCpp database.
    [[nodiscard]] SQLite::Database& connection() noexcept;

    [[nodiscard]] const std::string& path() const noexcept;

    // Execute SQL that does not return rows.
    void execute(const std::string& sql);

private:
    std::string databasePath_;
    SQLite::Database connection_;
};

} // namespace lms::repository::sqlite

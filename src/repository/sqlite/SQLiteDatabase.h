#pragma once

#include <string>

namespace lms::repository::sqlite {

// SQLiteCpp-backed implementation will be added after vendoring the dependency.
class SQLiteDatabase {
public:
    explicit SQLiteDatabase(std::string databasePath);
    [[nodiscard]] const std::string& path() const noexcept;

private:
    std::string databasePath_;
};

} // namespace lms::repository::sqlite

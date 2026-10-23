#include "repository/sqlite/SQLiteDatabase.h"

#include <utility>

namespace lms::repository::sqlite {

SQLiteDatabase::SQLiteDatabase(std::string databasePath)
    : databasePath_(std::move(databasePath)) {}

const std::string& SQLiteDatabase::path() const noexcept {
    return databasePath_;
}

} // namespace lms::repository::sqlite

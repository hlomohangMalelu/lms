#include "repository/sqlite/SQLiteDatabase.h"

#include <exception>
#include <iostream>

int main() {
    try {
        lms::repository::sqlite::SQLiteDatabase database(
            "data/lms.db"
        );

        database.execute(
            "CREATE TABLE IF NOT EXISTS connection_test ("
            "id INTEGER PRIMARY KEY, "
            "message TEXT NOT NULL"
            ");"
        );

        database.execute(
            "INSERT INTO connection_test (message) "
            "VALUES ('Hello, SQLite!');"
        );

        SQLite::Statement query(
            database.connection(),
            "SELECT message FROM connection_test "
            "ORDER BY id DESC LIMIT 1;"
        );

        if (query.executeStep()) {
            std::cout << "Latest message: "<< query.getColumn("message").getString()<< '\n';
        } else {
            std::cout << "No messages found in the database.\n";
        }

        std::cout << "SQLite database connected successfully.\n";
        std::cout << "Database: " << database.path() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Database error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}


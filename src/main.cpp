#include "repository/sqlite/SQLiteDatabase.h"
#include "security/BcryptPasswordHasher.h"
#include "service/AuthService.h"
#include "service/AdminService.h"
#include "service/SubmissionService.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "menus/admin/AdminMenu.h"
#include "menus/student/StudentMenu.h"
#include "menus/lecturer/LecturerMenu.h"
#include "menus/head_of_department/HODMenu.h"

namespace {
// Reads a line of input from the user.
std::string prompt(const std::string& label) {
    std::cout << label;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void executeSchema(SQLite::Database& db) {
    const std::string candidates[] = {
        "db/schema.sql",
        "../db/schema.sql",
        "../../db/schema.sql"
    };

    for (const auto& path : candidates) {
        std::ifstream in(path);

        if (!in) {
            continue;
        }

        std::string sql(
            (std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>()
        );

        db.exec(sql);
        return;
    }

    throw std::runtime_error(
        "Could not find db/schema.sql. "
        "Run the application from the project directory."
    );
}

void ensureAdmin(
    SQLite::Database& db,
    const lms::security::BcryptPasswordHasher& hasher
) {
    SQLite::Statement query(
        db,
        "SELECT COUNT(*) FROM users"
    );

    query.executeStep();

    if (query.getColumn(0).getInt64() != 0) {
        return;
    }

    const auto hash = hasher.hash("ChangeMe123!");

    SQLite::Statement insert(
        db,
        "INSERT INTO users "
        "(username, password_hash, full_name, role) "
        "VALUES (?, ?, ?, 'ADMIN')"
    );

    insert.bind(1, "admin");
    insert.bind(2, hash);
    insert.bind(3, "System Administrator");
    insert.exec();

    std::cout
        << "\nFirst-run administrator created. "
        << "Login: admin / ChangeMe123!\n"
        << "Change this password immediately "
        << "from the account menu.\n\n";
}
} // namespace

int main() {
    try {
        std::filesystem::create_directories("data");

        lms::repository::sqlite::SQLiteDatabase wrapper(
            "data/lms.db"
        );

        auto& db = wrapper.connection();

        db.exec("PRAGMA foreign_keys=ON");

        executeSchema(db);

        lms::security::BcryptPasswordHasher hasher;

        ensureAdmin(db, hasher);

        lms::service::AuthService auth(db, hasher);

        for (;;) {
            std::cout
                <<"\n-------------------------------------------\n"
                << "    MODERATED LEARNING MANAGEMENT SYSTEM    "
                <<"\n-------------------------------------------\n"
                << "1 Login\n"
                << "0 Exit\n"
                <<"-------------------------------------------\n";

            const auto choice = prompt("Choice: ");

            if (choice == "0" || !std::cin) {
                break;
            }

            if (choice != "1") {
                std::cout << "Unknown choice.\n";
                continue;
            }

            try {
                const auto user = auth.login(
                    prompt("Username: "),
                    prompt("Password: ")
                );

                if (!user) {
                    std::cout
                        << "Login failed. "
                        << "Check credentials or account status.\n";
                    continue;
                }

                std::cout
                    << "Welcome, "
                    << user->fullName
                    << " ("
                    << user->role
                    << ").\n";

                if (user->role == "ADMIN") {
                    lms::menus::adminMenu(db, *user, hasher);
                } else if (user->role == "STUDENT") {
                    lms::menus::studentMenu(db, *user, hasher);
                } else if (user->role == "LECTURER") {
                    lms::menus::lecturerMenu(db, *user, hasher);
                } else if (user->role == "HOD") {
                    lms::menus::hodMenu(db, *user, hasher);
                } else {
                    std::cout << "Unsupported role.\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
#include "repository/sqlite/SQLiteDatabase.h"
#include "security/BcryptPasswordHasher.h"
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

namespace lms::menus {
namespace {
using lms::service::AuthenticatedUser;
std::string prompt(const std::string& label) {
    std::cout << label;

    std::string value;
    std::getline(std::cin, value);

    return value;
}

long long number(const std::string& label) {
    const auto value = prompt(label);

    std::size_t used = 0;
    const long long result = std::stoll(value, &used);

    if (used != value.size() || result <= 0) {
        throw std::invalid_argument(
            "Enter a positive whole number."
        );
    }

    return result;
}

double realNumber(const std::string& label) {
    const auto value = prompt(label);

    std::size_t used = 0;
    const double result = std::stod(value, &used);

    if (used != value.size() || result < 0.0) {
        throw std::invalid_argument(
            "Enter a non-negative number."
        );
    }

    return result;
}

std::string now(SQLite::Database& db) {
    SQLite::Statement query(
        db,
        "SELECT strftime('%Y-%m-%d %H:%M:%S', 'now', 'localtime')"
    );

    if (!query.executeStep()) {
        throw std::runtime_error(
            "Unable to get current time."
        );
    }

    return query.getColumn(0).getString();
}

void list(SQLite::Database& db, const std::string& sql) {
    SQLite::Statement query(db, sql);

    int rows = 0;

    while (query.executeStep()) {
        ++rows;

        for (int i = 0; i < query.getColumnCount(); ++i) {
            if (i) {
                std::cout << " | ";
            }

            const auto column = query.getColumn(i);

            std::cout
                << column.getName()
                << "="
                << (column.isNull()
                        ? "NULL"
                        : column.getString());
        }

        std::cout << '\n';
    }

    if (!rows) {
        std::cout << "No records found.\n";
    }
}

void changePassword(
    SQLite::Database&,
    lms::service::AuthService& auth,
    const AuthenticatedUser& user
) {
    auth.changePassword(
        user.id,
        prompt("Current password: "),
        prompt("New password (minimum 12 characters): ")
    );

    std::cout << "Password changed.\n";
}
} // namespace

void adminMenu(
    SQLite::Database& db,
    const AuthenticatedUser& user,
    lms::security::BcryptPasswordHasher& hasher
) {
    lms::service::AdminService admin(db);

    for (;;) {
        std::cout
            << "\nADMIN MENU\n"
            << "1 Create department\n"
            << "2 Create course\n"
            << "3 Create user\n"
            << "4 Register student profile\n"
            << "5 Register lecturer profile\n"
            << "6 Assign head of department\n"
            << "7 Assign lecturer to course\n"
            << "8 Enrol student\n"
            << "9 Deactivate user\n"
            << "10 List users\n"
            << "11 List courses\n"
            << "12 Change password\n"
            << "0 Logout\n";

        const auto choice = prompt("Choice: ");

        try {
            if (choice == "0") {
                return;
            }

            if (choice == "1") {
                std::cout
                    << "Created department id "
                    << admin.createDepartment(
                           prompt("Department name: ")
                       )
                    << '\n';
            } else if (choice == "2") {
                std::cout
                    << "Created course id "
                    << admin.createCourse(
                           prompt("Course code: "),
                           prompt("Course title: "),
                           number("Department id: ")
                       )
                    << '\n';
            } else if (choice == "3") {
                const auto username = prompt("Username: ");
                const auto name = prompt("Full name: ");
                const auto role = prompt(
                    "Role (STUDENT/LECTURER/HOD/ADMIN): "
                );

                const auto password = prompt(
                    "Initial password (minimum 12 characters): "
                );

                if (password.size() < 12) {
                    throw std::invalid_argument(
                        "Password must contain at least 12 characters."
                    );
                }

                std::cout
                    << "Created user id "
                    << admin.createUser(
                           username,
                           hasher.hash(password),
                           name,
                           role
                       )
                    << '\n';
            } else if (choice == "4") {
                admin.registerStudent(
                    number("User id: "),
                    prompt("Student number: ")
                );
            } else if (choice == "5") {
                admin.registerLecturer(
                    number("User id: "),
                    prompt("Staff number: ")
                );
            } else if (choice == "6") {
                admin.assignHeadOfDepartment(
                    number("User id: "),
                    number("Department id: ")
                );
            } else if (choice == "7") {
                admin.assignLecturerToCourse(
                    number("Lecturer user id: "),
                    number("Course id: ")
                );
            } else if (choice == "8") {
                admin.enrolStudent(
                    number("Student user id: "),
                    number("Course id: ")
                );
            } else if (choice == "9") {
                admin.deactivateUser(
                    number("User id to deactivate: ")
                );
            } else if (choice == "10") {
                list(
                    db,
                    "SELECT id, username, full_name, role, active "
                    "FROM users ORDER BY id"
                );
            } else if (choice == "11") {
                list(
                    db,
                    "SELECT c.id, c.code, c.title, "
                    "d.name AS department "
                    "FROM courses c "
                    "JOIN departments d ON d.id = c.department_id "
                    "ORDER BY c.code"
                );
            } else if (choice == "12") {
                lms::service::AuthService auth(db, hasher);
                changePassword(db, auth, user);
            } else {
                std::cout << "Unknown choice.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }
}

} // namespace lms::menus

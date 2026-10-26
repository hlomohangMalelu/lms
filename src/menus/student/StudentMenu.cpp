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

#include "menus/student/StudentMenu.h"

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

void studentMenu(
    SQLite::Database& db,
    const AuthenticatedUser& user,
    lms::security::BcryptPasswordHasher& hasher
) {
    lms::service::SubmissionService submissions(db);

    for (;;) {
        std::cout
            << "\nSTUDENT MENU\n"
            << "1 My courses and assignments\n"
            << "2 Submit/resubmit assignment\n"
            << "3 View released marks and feedback\n"
            << "4 Change password\n"
            << "0 Logout\n";

        const auto choice = prompt("Choice: ");

        try {
            if (choice == "0") {
                return;
            }

            if (choice == "1") {
                SQLite::Statement query(
                    db,
                    "SELECT c.code, c.title, "
                    "a.id AS assignment_id, "
                    "a.title AS assignment, "
                    "a.deadline, a.max_mark "
                    "FROM enrolments e "
                    "JOIN courses c ON c.id = e.course_id "
                    "LEFT JOIN assignments a ON a.course_id = c.id "
                    "WHERE e.student_id = ? "
                    "ORDER BY c.code, a.deadline"
                );

                query.bind(
                    1,
                    static_cast<std::int64_t>(user.id)
                );

                bool any = false;

                while (query.executeStep()) {
                    any = true;

                    for (
                        int i = 0;
                        i < query.getColumnCount();
                        ++i
                    ) {
                        if (i) {
                            std::cout << " | ";
                        }

                        auto column = query.getColumn(i);

                        std::cout
                            << column.getName()
                            << "="
                            << (column.isNull()
                                    ? "-"
                                    : column.getString());
                    }

                    std::cout << '\n';
                }

                if (!any) {
                    std::cout
                        << "No enrolled courses or assignments.\n";
                }
            } else if (choice == "2") {
                const auto assignment = number("Assignment id: ");
                const auto path = prompt(
                    "Submission file path/reference: "
                );

                SQLite::Statement enrolled(
                    db,
                    "SELECT 1 FROM assignments a "
                    "JOIN enrolments e ON e.course_id = a.course_id "
                    "WHERE a.id = ? AND e.student_id = ?"
                );

                enrolled.bind(
                    1,
                    static_cast<std::int64_t>(assignment)
                );
                enrolled.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );

                if (!enrolled.executeStep()) {
                    throw std::runtime_error(
                        "You are not enrolled in the assignment course."
                    );
                }

                std::cout
                    << "Saved submission id "
                    << submissions.submit(
                           assignment,
                           user.id,
                           path,
                           now(db)
                       )
                    << '\n';
            } else if (choice == "3") {
                SQLite::Statement query(
                    db,
                    "SELECT c.code, a.title, "
                    "s.id AS submission_id, "
                    "s.raw_mark, s.final_mark, s.feedback, "
                    "s.released_at "
                    "FROM submissions s "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN courses c ON c.id = a.course_id "
                    "WHERE s.student_id = ? "
                    "AND s.status = 'RELEASED' "
                    "ORDER BY s.released_at DESC"
                );

                query.bind(
                    1,
                    static_cast<std::int64_t>(user.id)
                );

                bool any = false;

                while (query.executeStep()) {
                    any = true;

                    for (
                        int i = 0;
                        i < query.getColumnCount();
                        ++i
                    ) {
                        if (i) {
                            std::cout << " | ";
                        }

                        auto column = query.getColumn(i);

                        std::cout
                            << column.getName()
                            << "="
                            << (column.isNull()
                                    ? "-"
                                    : column.getString());
                    }

                    std::cout << '\n';
                }

                if (!any) {
                    std::cout << "No released marks yet.\n";
                }
            } else if (choice == "4") {
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

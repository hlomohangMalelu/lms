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

#include "menus/lecturer/LecturerMenu.h"

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

void lecturerMenu(
    SQLite::Database& db,
    const AuthenticatedUser& user,
    lms::security::BcryptPasswordHasher& hasher
) {
    for (;;) {
        std::cout
            <<"\n--------------------------------------\n"
            << "           LECTURER MENU\n"
            <<"--------------------------------------\n"
            << "1 My courses\n"
            << "2 Create assignment\n"
            << "3 View submissions\n"
            << "4 Mark submission\n"
            << "5 Grant extension\n"
            << "6 Change password\n"
            << "0 Logout\n"
            <<"--------------------------------------\n";

        const auto choice = prompt("Choice: ");

        try {
            if (choice == "0") {
                return;
            }

            if (choice == "1") {
                SQLite::Statement query(
                    db,
                    "SELECT c.id, c.code, c.title "
                    "FROM course_lecturers cl "
                    "JOIN courses c ON c.id = cl.course_id "
                    "WHERE cl.lecturer_id = ? "
                    "ORDER BY c.code"
                );

                query.bind(
                    1,
                    static_cast<std::int64_t>(user.id)
                );

                while (query.executeStep()) {
                    std::cout
                        << query.getColumn(0).getInt64()
                        << " | "
                        << query.getColumn(1).getString()
                        << " | "
                        << query.getColumn(2).getString()
                        << '\n';
                }
            } else if (choice == "2") {
                const auto course = number("Course id: ");

                SQLite::Statement check(
                    db,
                    "SELECT 1 FROM course_lecturers "
                    "WHERE lecturer_id = ? AND course_id = ?"
                );

                check.bind(
                    1,
                    static_cast<std::int64_t>(user.id)
                );
                check.bind(
                    2,
                    static_cast<std::int64_t>(course)
                );

                if (!check.executeStep()) {
                    throw std::runtime_error(
                        "You are not assigned to that course."
                    );
                }

                const auto title = prompt("Assignment title: ");
                const auto instructions = prompt("Instructions: ");
                const auto deadline = prompt(
                    "Deadline (YYYY-MM-DD HH:MM:SS): "
                );

                const auto max = realNumber("Maximum mark: ");

                if (max <= 0) {
                    throw std::invalid_argument(
                        "Maximum mark must be positive."
                    );
                }

                SQLite::Statement insert(
                    db,
                    "INSERT INTO assignments "
                    "(course_id, created_by, title, instructions, "
                    "deadline, max_mark) "
                    "VALUES (?, ?, ?, ?, ?, ?)"
                );

                insert.bind(
                    1,
                    static_cast<std::int64_t>(course)
                );
                insert.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );
                insert.bind(3, title);
                insert.bind(4, instructions);
                insert.bind(5, deadline);
                insert.bind(6, max);
                insert.exec();

                std::cout
                    << "Created assignment id "
                    << db.getLastInsertRowid()
                    << '\n';
            } else if (choice == "3") {
                const auto course = number("Course id: ");

                SQLite::Statement check(
                    db,
                    "SELECT 1 FROM course_lecturers "
                    "WHERE lecturer_id = ? AND course_id = ?"
                );

                check.bind(
                    1,
                    static_cast<std::int64_t>(user.id)
                );
                check.bind(
                    2,
                    static_cast<std::int64_t>(course)
                );

                if (!check.executeStep()) {
                    throw std::runtime_error(
                        "You are not assigned to that course."
                    );
                }

                SQLite::Statement query(
                    db,
                    "SELECT s.id AS submission_id, a.title, "
                    "u.full_name AS student, s.status, s.days_late, "
                    "s.raw_mark, s.final_mark, f.file_path "
                    "FROM submissions s "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN users u ON u.id = s.student_id "
                    "LEFT JOIN submission_files f "
                    "ON f.submission_id = s.id "
                    "WHERE a.course_id = ? "
                    "ORDER BY a.id, u.full_name"
                );

                query.bind(
                    1,
                    static_cast<std::int64_t>(course)
                );

                while (query.executeStep()) {
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
            } else if (choice == "4") {
                const auto sid = number("Submission id: ");

                SQLite::Statement scope(
                    db,
                    "SELECT a.max_mark, a.course_id, "
                    "s.final_mark, s.status, s.days_late "
                    "FROM submissions s "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN course_lecturers cl "
                    "ON cl.course_id = a.course_id "
                    "WHERE s.id = ? AND cl.lecturer_id = ?"
                );

                scope.bind(
                    1,
                    static_cast<std::int64_t>(sid)
                );
                scope.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );

                if (!scope.executeStep()) {
                    throw std::runtime_error(
                        "Submission not found in your assigned courses."
                    );
                }

                const auto max = scope.getColumn(0).getDouble();
                const auto status = scope.getColumn(3).getString();

                if (status == "RELEASED" || status == "MODERATED") {
                    throw std::runtime_error(
                        "This submission cannot be marked "
                        "in its current state."
                    );
                }

                const auto mark = realNumber("Raw mark: ");

                if (mark > max) {
                    throw std::invalid_argument(
                        "Mark exceeds maximum."
                    );
                }

                const auto feedback = prompt("Feedback: ");
                const auto reason = prompt("Audit reason: ");

                if (
                    reason.find_first_not_of(" \t\r\n")
                    == std::string::npos
                ) {
                    throw std::invalid_argument(
                        "Audit reason required."
                    );
                }

                SQLite::Transaction tx(db);

                SQLite::Statement old(
                    db,
                    "SELECT final_mark FROM submissions WHERE id = ?"
                );

                old.bind(1, static_cast<std::int64_t>(sid));
                old.executeStep();

                SQLite::Statement audit(
                    db,
                    "INSERT INTO audit_log "
                    "(submission_id, changed_by, old_value, "
                    "new_value, reason) "
                    "VALUES (?, ?, ?, ?, ?)"
                );

                audit.bind(
                    1,
                    static_cast<std::int64_t>(sid)
                );
                audit.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );

                if (old.getColumn(0).isNull()) {
                    audit.bind(3);
                } else {
                    audit.bind(
                        3,
                        old.getColumn(0).getDouble()
                    );
                }

                const double penalty = std::min(
                    1.0,
                    static_cast<double>(
                        scope.getColumn(4).getInt()
                    ) * 0.10
                );

                const double finalMark = mark * (1.0 - penalty);

                audit.bind(4, finalMark);
                audit.bind(5, reason);
                audit.exec();

                SQLite::Statement update(
                    db,
                    "UPDATE submissions SET "
                    "status = 'MARKED', "
                    "raw_mark = ?, "
                    "final_mark = ?, "
                    "penalty_fraction = CASE "
                    "WHEN days_late > 3 THEN 1.0 "
                    "ELSE days_late * 0.1 END, "
                    "feedback = ? "
                    "WHERE id = ?"
                );

                update.bind(1, mark);
                update.bind(2, finalMark);
                update.bind(3, feedback);
                update.bind(
                    4,
                    static_cast<std::int64_t>(sid)
                );
                update.exec();

                tx.commit();

                std::cout << "Mark saved and audited.\n";
            } else if (choice == "5") {
                const auto assignment = number("Assignment id: ");
                const auto student = number("Student user id: ");
                const auto deadline = prompt(
                    "New deadline (YYYY-MM-DD HH:MM:SS): "
                );
                const auto reason = prompt("Reason: ");

                SQLite::Statement scope(
                    db,
                    "SELECT 1 FROM assignments a "
                    "JOIN course_lecturers cl "
                    "ON cl.course_id = a.course_id "
                    "WHERE a.id = ? AND cl.lecturer_id = ?"
                );

                scope.bind(
                    1,
                    static_cast<std::int64_t>(assignment)
                );
                scope.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );

                if (!scope.executeStep()) {
                    throw std::runtime_error(
                        "Assignment not found in your assigned courses."
                    );
                }

                SQLite::Statement nextId(
                    db,
                    "SELECT COALESCE(MAX(id), 0) + 1 FROM extensions"
                );

                nextId.executeStep();

                lms::service::SubmissionService service(db);

                service.grantExtension(
                    nextId.getColumn(0).getInt64(),
                    assignment,
                    student,
                    user.id,
                    deadline,
                    reason
                );

                std::cout << "Extension saved.\n";
            } else if (choice == "6") {
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

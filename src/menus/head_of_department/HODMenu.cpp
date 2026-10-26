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

#include "menus/head_of_department/HODMenu.h"

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

void hodMenu(
    SQLite::Database& db,
    const AuthenticatedUser& user,
    lms::security::BcryptPasswordHasher& hasher
) {
    for (;;) {
        std::cout
            << "\nHEAD OF DEPARTMENT MENU\n"
            << "1 View marked submissions\n"
            << "2 Moderate mark\n"
            << "3 Approve mark\n"
            << "4 Release mark\n"
            << "5 Change password\n"
            << "0 Logout\n";

        const auto choice = prompt("Choice: ");

        try {
            if (choice == "0") {
                return;
            }

            if (choice == "1") {
                list(
                    db,
                    "SELECT s.id AS submission_id, c.code, a.title, "
                    "u.full_name, s.raw_mark, s.final_mark, s.status "
                    "FROM submissions s "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN courses c ON c.id = a.course_id "
                    "JOIN users u ON u.id = s.student_id "
                    "JOIN departments d ON d.id = c.department_id "
                    "JOIN heads_of_department h "
                    "ON h.department_id = d.id "
                    "WHERE h.user_id = "
                    + std::to_string(user.id)
                    + " AND s.status IN ('MARKED', 'MODERATED') "
                      "ORDER BY c.code, a.id"
                );
            } else if (choice == "2") {
                const auto sid = number("Submission id: ");
                const auto mark = realNumber("Moderated mark: ");
                const auto notes = prompt("Moderation notes: ");
                const auto reason = prompt("Audit reason: ");

                SQLite::Transaction tx(db);

                SQLite::Statement old(
                    db,
                    "SELECT s.final_mark, a.max_mark "
                    "FROM submissions s "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN courses c ON c.id = a.course_id "
                    "JOIN heads_of_department h "
                    "ON h.department_id = c.department_id "
                    "WHERE s.id = ? AND h.user_id = "
                    + std::to_string(user.id)
                    + " AND s.status IN ('MARKED', 'MODERATED')"
                );

                old.bind(1, static_cast<std::int64_t>(sid));

                if (!old.executeStep()) {
                    throw std::runtime_error(
                        "Submission is not available for moderation."
                    );
                }

                if (mark > old.getColumn(1).getDouble()) {
                    throw std::invalid_argument(
                        "Mark exceeds maximum."
                    );
                }

                if (old.getColumn(0).isNull()) {
                    throw std::runtime_error(
                        "No mark to moderate."
                    );
                }

                if (old.getColumn(0).getDouble() != mark) {
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
                    audit.bind(
                        3,
                        old.getColumn(0).getDouble()
                    );
                    audit.bind(4, mark);
                    audit.bind(5, reason);
                    audit.exec();
                }

                SQLite::Statement update(
                    db,
                    "UPDATE submissions SET "
                    "final_mark = ?, "
                    "status = 'MODERATED', "
                    "released_at = NULL, "
                    "released_by = NULL "
                    "WHERE id = ?"
                );

                update.bind(1, mark);
                update.bind(
                    2,
                    static_cast<std::int64_t>(sid)
                );
                update.exec();

                SQLite::Statement record(
                    db,
                    "INSERT INTO mark_records "
                    "(submission_id, moderated_by, moderated_mark, "
                    "moderation_notes, moderated_at) "
                    "VALUES (?, ?, ?, ?, ?) "
                    "ON CONFLICT(submission_id) DO UPDATE SET "
                    "moderated_by = excluded.moderated_by, "
                    "moderated_mark = excluded.moderated_mark, "
                    "moderation_notes = excluded.moderation_notes, "
                    "moderated_at = excluded.moderated_at, "
                    "approved_by = NULL, "
                    "approved_at = NULL"
                );

                record.bind(
                    1,
                    static_cast<std::int64_t>(sid)
                );
                record.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );
                record.bind(3, mark);
                record.bind(4, notes);
                record.bind(5, now(db));
                record.exec();

                tx.commit();

                std::cout
                    << "Moderation saved; approval is required again.\n";
            } else if (choice == "3") {
                const auto sid = number("Submission id: ");

                SQLite::Transaction tx(db);

                SQLite::Statement query(
                    db,
                    "SELECT s.status "
                    "FROM submissions s "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN courses c ON c.id = a.course_id "
                    "JOIN heads_of_department h "
                    "ON h.department_id = c.department_id "
                    "WHERE s.id = ? AND h.user_id = "
                    + std::to_string(user.id)
                );

                query.bind(
                    1,
                    static_cast<std::int64_t>(sid)
                );

                if (
                    !query.executeStep() ||
                    query.getColumn(0).getString() != "MODERATED"
                ) {
                    throw std::runtime_error(
                        "Only moderated submissions can be approved."
                    );
                }

                SQLite::Statement update(
                    db,
                    "UPDATE mark_records SET "
                    "approved_by = ?, approved_at = ? "
                    "WHERE submission_id = ? "
                    "AND moderated_mark IS NOT NULL"
                );

                update.bind(
                    1,
                    static_cast<std::int64_t>(user.id)
                );
                update.bind(2, now(db));
                update.bind(
                    3,
                    static_cast<std::int64_t>(sid)
                );
                update.exec();

                if (db.getChanges() == 0) {
                    throw std::runtime_error(
                        "Moderation record not found."
                    );
                }

                tx.commit();

                std::cout << "Approved.\n";
            } else if (choice == "4") {
                const auto sid = number("Submission id: ");

                SQLite::Transaction tx(db);

                SQLite::Statement query(
                    db,
                    "SELECT s.final_mark, m.approved_at "
                    "FROM submissions s "
                    "JOIN mark_records m ON m.submission_id = s.id "
                    "JOIN assignments a ON a.id = s.assignment_id "
                    "JOIN courses c ON c.id = a.course_id "
                    "JOIN heads_of_department h "
                    "ON h.department_id = c.department_id "
                    "WHERE s.id = ? AND h.user_id = "
                    + std::to_string(user.id)
                    + " AND s.status = 'MODERATED'"
                );

                query.bind(
                    1,
                    static_cast<std::int64_t>(sid)
                );

                if (
                    !query.executeStep() ||
                    query.getColumn(0).isNull() ||
                    query.getColumn(1).isNull()
                ) {
                    throw std::runtime_error(
                        "Mark must be moderated and approved "
                        "before release."
                    );
                }

                SQLite::Statement update(
                    db,
                    "UPDATE submissions SET "
                    "status = 'RELEASED', "
                    "released_at = ?, released_by = ? "
                    "WHERE id = ?"
                );

                update.bind(1, now(db));
                update.bind(
                    2,
                    static_cast<std::int64_t>(user.id)
                );
                update.bind(
                    3,
                    static_cast<std::int64_t>(sid)
                );
                update.exec();

                tx.commit();

                std::cout << "Mark released to student.\n";
            } else if (choice == "5") {
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

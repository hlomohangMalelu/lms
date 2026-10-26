
#include "service/SubmissionService.h"

#include <stdexcept>

namespace lms::service {

SubmissionService::SubmissionService(SQLite::Database& db)
    : database_(db) {
}

long long SubmissionService::submit(
    long long assignmentId,
    long long studentId,
    const std::string& filePath,
    const std::string& submittedAt
) {
    if (filePath.empty() || submittedAt.empty()) {
        throw std::invalid_argument(
            "A submission file and timestamp are required."
        );
    }

    SQLite::Transaction tx(database_);

    SQLite::Statement assignment(
        database_,
        "SELECT deadline FROM assignments WHERE id = ?"
    );
    assignment.bind(1, static_cast<std::int64_t>(assignmentId));

    if (!assignment.executeStep()) {
        throw std::runtime_error("Assignment not found.");
    }

    std::string deadline = assignment.getColumn(0).getString();

    SQLite::Statement extension(
        database_,
        "SELECT new_deadline FROM extensions "
        "WHERE assignment_id = ? AND student_id = ?"
    );
    extension.bind(1, static_cast<std::int64_t>(assignmentId));
    extension.bind(2, static_cast<std::int64_t>(studentId));

    if (extension.executeStep()) {
        deadline = extension.getColumn(0).getString();
    }

    SQLite::Statement lateness(
        database_,
        "SELECT CASE "
        "WHEN datetime(?) <= datetime(?) THEN 0 "
        "ELSE CAST(julianday(?) - julianday(?) AS INTEGER) + "
        "CASE "
        "WHEN (julianday(?) - julianday(?)) > "
        "CAST(julianday(?) - julianday(?) AS INTEGER) "
        "THEN 1 ELSE 0 "
        "END "
        "END"
    );

    lateness.bind(1, submittedAt);
    lateness.bind(2, deadline);
    lateness.bind(3, submittedAt);
    lateness.bind(4, deadline);
    lateness.bind(5, submittedAt);
    lateness.bind(6, deadline);
    lateness.bind(7, submittedAt);
    lateness.bind(8, deadline);

    if (!lateness.executeStep()) {
        throw std::runtime_error(
            "Could not calculate submission lateness."
        );
    }

    const int daysLate = lateness.getColumn(0).getInt();

    if (daysLate > 3) {
        throw std::logic_error(
            "Submission is more than three days late "
            "and no valid extension applies."
        );
    }

    const std::string status =
        daysLate > 0 ? "LATE" : "SUBMITTED";

    SQLite::Statement existing(
        database_,
        "SELECT id, status FROM submissions "
        "WHERE assignment_id = ? AND student_id = ?"
    );
    existing.bind(1, static_cast<std::int64_t>(assignmentId));
    existing.bind(2, static_cast<std::int64_t>(studentId));

    long long id = 0;

    if (existing.executeStep()) {
        id = existing.getColumn("id").getInt64();

        const auto existingStatus =
            existing.getColumn("status").getString();

        if (
            existingStatus == "RELEASED" ||
            existingStatus == "MODERATED" ||
            existingStatus == "MARKED"
        ) {
            throw std::logic_error(
                "A marked or released submission "
                "cannot be resubmitted."
            );
        }

        SQLite::Statement update(
            database_,
            "UPDATE submissions SET "
            "status = ?, "
            "days_late = ?, "
            "penalty_fraction = 0, "
            "raw_mark = NULL, "
            "final_mark = NULL, "
            "feedback = NULL, "
            "released_at = NULL, "
            "released_by = NULL "
            "WHERE id = ?"
        );

        update.bind(1, status);
        update.bind(2, daysLate);
        update.bind(3, static_cast<std::int64_t>(id));
        update.exec();
    } else {
        SQLite::Statement insert(
            database_,
            "INSERT INTO submissions "
            "(assignment_id, student_id, status, "
            "days_late, penalty_fraction) "
            "VALUES (?, ?, ?, ?, 0)"
        );

        insert.bind(1, static_cast<std::int64_t>(assignmentId));
        insert.bind(2, static_cast<std::int64_t>(studentId));
        insert.bind(3, status);
        insert.bind(4, daysLate);
        insert.exec();

        id = database_.getLastInsertRowid();
    }

    // Store the uploaded-file reference separately to avoid
    // changing the Submission domain model.
    database_.exec(
        "CREATE TABLE IF NOT EXISTS submission_files ("
        "submission_id INTEGER PRIMARY KEY "
        "REFERENCES submissions(id) ON DELETE RESTRICT, "
        "file_path TEXT NOT NULL, "
        "submitted_at TEXT NOT NULL)"
    );

    SQLite::Statement file(
        database_,
        "INSERT INTO submission_files "
        "(submission_id, file_path, submitted_at) "
        "VALUES (?, ?, ?) "
        "ON CONFLICT(submission_id) DO UPDATE SET "
        "file_path = excluded.file_path, "
        "submitted_at = excluded.submitted_at"
    );

    file.bind(1, static_cast<std::int64_t>(id));
    file.bind(2, filePath);
    file.bind(3, submittedAt);
    file.exec();

    tx.commit();

    return id;
}

void SubmissionService::grantExtension(
    long long extensionId,
    long long assignmentId,
    long long studentId,
    long long lecturerId,
    const std::string& newDeadline,
    const std::string& reason
) {
    if (
        newDeadline.empty() ||
        reason.find_first_not_of(" \t\r\n") == std::string::npos
    ) {
        throw std::invalid_argument(
            "Extension deadline and reason are required."
        );
    }

    SQLite::Statement query(
        database_,
        "INSERT INTO extensions "
        "(id, assignment_id, student_id, granted_by, "
        "new_deadline, reason) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(assignment_id, student_id) "
        "DO UPDATE SET "
        "granted_by = excluded.granted_by, "
        "new_deadline = excluded.new_deadline, "
        "reason = excluded.reason"
    );

    query.bind(1, static_cast<std::int64_t>(extensionId));
    query.bind(2, static_cast<std::int64_t>(assignmentId));
    query.bind(3, static_cast<std::int64_t>(studentId));
    query.bind(4, static_cast<std::int64_t>(lecturerId));
    query.bind(5, newDeadline);
    query.bind(6, reason);

    query.exec();
}

} // namespace lms::service

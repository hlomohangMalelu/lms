
#include "service/SubmissionService.h"

#include <stdexcept>

namespace lms::service {

SubmissionService::SubmissionService(SQLite::Database& db, const lms::security::IAuthorizationPolicy& authorization)
    : database_(db), authorization_(authorization) {
}

long long SubmissionService::submit(
    long long assignmentId,
    long long studentId,
    const std::string& filePath,
    const std::string& submittedAt
) {
    authorization_.requireCanSubmit(studentId, assignmentId);
    if (filePath.empty() || submittedAt.empty()) {
        throw std::invalid_argument(
            "A submission file and timestamp are required."
        );
    }

    SQLite::Transaction tx(database_);

    SQLite::Statement assignment(
        database_,
        "SELECT a.deadline, "
        "(SELECT ce.new_deadline FROM class_extensions ce "
        " WHERE ce.assignment_id = a.id) AS class_deadline "
        "FROM assignments a WHERE a.id = ?"
    );
    assignment.bind(1, static_cast<std::int64_t>(assignmentId));

    if (!assignment.executeStep()) {
        throw std::runtime_error("Assignment not found.");
    }

    std::string deadline = assignment.getColumn(0).getString();
    if (!assignment.getColumn(1).isNull()) {
        deadline = assignment.getColumn(1).getString();
    }

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

    // Late work is still accepted after three days. The marking strategy
    // assigns a zero final mark for this equivalence class, as required by UC01.

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
    int attemptNumber = 1;

    if (existing.executeStep()) {
        id = existing.getColumn("id").getInt64();

        SQLite::Statement nextAttempt(
            database_,
            "SELECT COALESCE(MAX(attempt_number), 0) + 1 "
            "FROM submission_attempts WHERE submission_id = ?"
        );
        nextAttempt.bind(1, static_cast<std::int64_t>(id));
        if (!nextAttempt.executeStep()) {
            throw std::runtime_error("Could not determine the next submission attempt.");
        }
        attemptNumber = nextAttempt.getColumn(0).getInt();

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

    database_.exec(
        "CREATE TABLE IF NOT EXISTS submission_attempts ("
        "id INTEGER PRIMARY KEY, "
        "submission_id INTEGER NOT NULL REFERENCES submissions(id) ON DELETE RESTRICT, "
        "attempt_number INTEGER NOT NULL CHECK (attempt_number >= 1), "
        "file_path TEXT NOT NULL, "
        "submitted_at TEXT NOT NULL CHECK (datetime(submitted_at) IS NOT NULL), "
        "status TEXT NOT NULL CHECK (status IN ('SUBMITTED', 'LATE')), "
        "days_late INTEGER NOT NULL CHECK (days_late >= 0), "
        "UNIQUE (submission_id, attempt_number))"
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

    SQLite::Statement attempt(
        database_,
        "INSERT INTO submission_attempts "
        "(submission_id, attempt_number, file_path, submitted_at, status, days_late) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );
    attempt.bind(1, static_cast<std::int64_t>(id));
    attempt.bind(2, attemptNumber);
    attempt.bind(3, filePath);
    attempt.bind(4, submittedAt);
    attempt.bind(5, status);
    attempt.bind(6, daysLate);
    attempt.exec();

    tx.commit();

    return id;
}

void SubmissionService::grantClassExtension(
    long long extensionId,
    long long assignmentId,
    long long lecturerId,
    const std::string& newDeadline,
    const std::string& reason
) {
    authorization_.requireCanGrantClassExtension(lecturerId, assignmentId);
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
        "INSERT INTO class_extensions "
        "(id, assignment_id, granted_by, new_deadline, reason) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(assignment_id) DO UPDATE SET "
        "granted_by = excluded.granted_by, "
        "new_deadline = excluded.new_deadline, "
        "reason = excluded.reason"
    );
    query.bind(1, static_cast<std::int64_t>(extensionId));
    query.bind(2, static_cast<std::int64_t>(assignmentId));
    query.bind(3, static_cast<std::int64_t>(lecturerId));
    query.bind(4, newDeadline);
    query.bind(5, reason);
    query.exec();
}

void SubmissionService::grantExtension(
    long long extensionId,
    long long assignmentId,
    long long studentId,
    long long lecturerId,
    const std::string& newDeadline,
    const std::string& reason
) {
    authorization_.requireCanGrantExtension(lecturerId, assignmentId, studentId);
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

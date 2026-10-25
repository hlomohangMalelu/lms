#include "repository/submission/SQLiteSubmissionRepository.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace lms::repository {

    SQLiteSubmissionRepository::SQLiteSubmissionRepository(
        SQLite::Database& database
    )
        : database_(database) {}

    void SQLiteSubmissionRepository::save(
        const lms::domain::Submission& submission
    ) {
        SQLite::Statement query(
            database_,
            "INSERT INTO submissions ("
            "id, assignment_id, student_id, status, days_late, "
            "penalty_fraction, raw_mark, final_mark, feedback, "
            "released_at, released_by"
            ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

        query.bind(
            1, static_cast<std::int64_t>(submission.submissionId())
        );
        query.bind(
            2, static_cast<std::int64_t>(submission.assignmentId())
        );
        query.bind(
            3, static_cast<std::int64_t>(submission.studentId())
        );
        query.bind(4, submission.status());
        query.bind(5, submission.daysLate());
        query.bind(6, submission.penaltyFraction());

        if (submission.rawMark().has_value()) {
            query.bind(7, submission.rawMark().value());
        } else {
            query.bind(7);
        }

        if (submission.finalMark().has_value()) {
            query.bind(8, submission.finalMark().value());
        } else {
            query.bind(8);
        }

        if (submission.feedback().has_value()) {
            query.bind(9, submission.feedback().value());
        } else {
            query.bind(9);
        }

        if (submission.releasedAt().has_value()) {
            query.bind(10, submission.releasedAt().value());
        } else {
            query.bind(10);
        }

        if (submission.releasedBy().has_value()) {
            query.bind(
                11,
                static_cast<std::int64_t>(submission.releasedBy().value())
            );
        } else {
            query.bind(11);
        }

        query.exec();
    }

    lms::domain::Submission SQLiteSubmissionRepository::findById(
        long long submissionId
    ) {
        SQLite::Statement query(
            database_,
            "SELECT id, assignment_id, student_id, status, days_late, "
            "penalty_fraction, raw_mark, final_mark, feedback, "
            "released_at, released_by "
            "FROM submissions WHERE id = ?"
        );

        query.bind(1, static_cast<std::int64_t>(submissionId));

        if (!query.executeStep()) {
            throw std::runtime_error("Submission not found.");
        }

        std::optional<double> rawMark;
        std::optional<double> finalMark;
        std::optional<std::string> feedback;
        std::optional<std::string> releasedAt;
        std::optional<long long> releasedBy;

        if (!query.isColumnNull(6)) {
            rawMark = query.getColumn(6).getDouble();
        }

        if (!query.isColumnNull(7)) {
            finalMark = query.getColumn(7).getDouble();
        }

        if (!query.isColumnNull(8)) {
            feedback = query.getColumn(8).getString();
        }

        if (!query.isColumnNull(9)) {
            releasedAt = query.getColumn(9).getString();
        }

        if (!query.isColumnNull(10)) {
            releasedBy = query.getColumn(10).getInt64();
        }

        return lms::domain::Submission(
            query.getColumn(0).getInt64(),
            query.getColumn(1).getInt64(),
            query.getColumn(2).getInt64(),
            query.getColumn(3).getString(),
            query.getColumn(4).getInt(),
            query.getColumn(5).getDouble(),
            rawMark,
            finalMark,
            feedback,
            releasedAt,
            releasedBy
        );
    }

    void SQLiteSubmissionRepository::update(
        const lms::domain::Submission& submission
    ) {
        // Check existence separately so unchanged values are still valid.
        SQLite::Statement existenceQuery(
            database_,
            "SELECT 1 FROM submissions WHERE id = ?"
        );

        existenceQuery.bind(
            1, static_cast<std::int64_t>(submission.submissionId())
        );

        if (!existenceQuery.executeStep()) {
            throw std::runtime_error("Submission not found.");
        }

        SQLite::Statement query(
            database_,
            "UPDATE submissions SET "
            "assignment_id = ?, student_id = ?, status = ?, days_late = ?, "
            "penalty_fraction = ?, raw_mark = ?, final_mark = ?, feedback = ?, "
            "released_at = ?, released_by = ? "
            "WHERE id = ?"
        );

        query.bind(
            1, static_cast<std::int64_t>(submission.assignmentId())
        );
        query.bind(
            2, static_cast<std::int64_t>(submission.studentId())
        );
        query.bind(3, submission.status());
        query.bind(4, submission.daysLate());
        query.bind(5, submission.penaltyFraction());

        if (submission.rawMark().has_value()) {
            query.bind(6, submission.rawMark().value());
        } else {
            query.bind(6);
        }

        if (submission.finalMark().has_value()) {
            query.bind(7, submission.finalMark().value());
        } else {
            query.bind(7);
        }

        if (submission.feedback().has_value()) {
            query.bind(8, submission.feedback().value());
        } else {
            query.bind(8);
        }

        if (submission.releasedAt().has_value()) {
            query.bind(9, submission.releasedAt().value());
        } else {
            query.bind(9);
        }

        if (submission.releasedBy().has_value()) {
            query.bind(
                10,
                static_cast<std::int64_t>(submission.releasedBy().value())
            );
        } else {
            query.bind(10);
        }

        query.bind(
            11, static_cast<std::int64_t>(submission.submissionId())
        );

        query.exec();
    }

}

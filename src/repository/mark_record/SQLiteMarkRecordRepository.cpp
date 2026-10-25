#include "repository/mark_record/SQLiteMarkRecordRepository.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace lms::repository {

    SQLiteMarkRecordRepository::SQLiteMarkRecordRepository(
        SQLite::Database& database
    )
        : database_(database) {}

    void SQLiteMarkRecordRepository::save(
        const lms::domain::MarkRecord& record
    ) {
        SQLite::Statement query(
            database_,
            "INSERT INTO mark_records ("
            "id, submission_id, moderated_by, moderated_mark, "
            "moderation_notes, moderated_at, approved_by, approved_at"
            ") VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
            "ON CONFLICT(submission_id) DO UPDATE SET "
            "moderated_by = excluded.moderated_by, "
            "moderated_mark = excluded.moderated_mark, "
            "moderation_notes = excluded.moderation_notes, "
            "moderated_at = excluded.moderated_at, "
            "approved_by = excluded.approved_by, "
            "approved_at = excluded.approved_at"
        );

        query.bind(1, static_cast<std::int64_t>(record.markRecordId()));
        query.bind(2, static_cast<std::int64_t>(record.submissionId()));

        if (record.moderatedBy()) {
            query.bind(3, static_cast<std::int64_t>(*record.moderatedBy()));
        } else {
            query.bind(3);
        }

        if (record.moderatedMark()) {
            query.bind(4, *record.moderatedMark());
        } else {
            query.bind(4);
        }

        if (record.moderationNotes()) {
            query.bind(5, *record.moderationNotes());
        } else {
            query.bind(5);
        }

        if (record.moderatedAt()) {
            query.bind(6, *record.moderatedAt());
        } else {
            query.bind(6);
        }

        if (record.approvedBy()) {
            query.bind(7, static_cast<std::int64_t>(*record.approvedBy()));
        } else {
            query.bind(7);
        }

        if (record.approvedAt()) {
            query.bind(8, *record.approvedAt());
        } else {
            query.bind(8);
        }

        query.exec();
    }

    lms::domain::MarkRecord
    SQLiteMarkRecordRepository::findBySubmissionId(
        long long submissionId
    ) {
        SQLite::Statement query(
            database_,
            "SELECT id, submission_id, moderated_by, moderated_mark, "
            "moderation_notes, moderated_at, approved_by, approved_at "
            "FROM mark_records WHERE submission_id = ?"
        );

        query.bind(1, static_cast<std::int64_t>(submissionId));

        if (!query.executeStep()) {
            throw std::runtime_error("Mark record not found.");
        }

        std::optional<long long> moderatedBy;
        std::optional<double> moderatedMark;
        std::optional<std::string> moderationNotes;
        std::optional<std::string> moderatedAt;
        std::optional<long long> approvedBy;
        std::optional<std::string> approvedAt;

        if (!query.isColumnNull(2)) {
            moderatedBy = query.getColumn(2).getInt64();
        }
        if (!query.isColumnNull(3)) {
            moderatedMark = query.getColumn(3).getDouble();
        }
        if (!query.isColumnNull(4)) {
            moderationNotes = query.getColumn(4).getString();
        }
        if (!query.isColumnNull(5)) {
            moderatedAt = query.getColumn(5).getString();
        }
        if (!query.isColumnNull(6)) {
            approvedBy = query.getColumn(6).getInt64();
        }
        if (!query.isColumnNull(7)) {
            approvedAt = query.getColumn(7).getString();
        }

        return lms::domain::MarkRecord(
            query.getColumn(0).getInt64(),
            query.getColumn(1).getInt64(),
            moderatedBy,
            moderatedMark,
            moderationNotes,
            moderatedAt,
            approvedBy,
            approvedAt
        );
    }

}

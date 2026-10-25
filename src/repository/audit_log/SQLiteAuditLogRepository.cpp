#include "repository/audit_log/SQLiteAuditLogRepository.h"

#include <cstdint>

namespace lms::repository {

    SQLiteAuditLogRepository::SQLiteAuditLogRepository(
        SQLite::Database& database
    )
        : database_(database) {}

    void SQLiteAuditLogRepository::save(
        const lms::domain::AuditLog& auditLog
    ) {
        SQLite::Statement query(
            database_,
            "INSERT INTO audit_log ("
            "id, submission_id, changed_by, changed_at, "
            "old_value, new_value, reason"
            ") VALUES (?, ?, ?, ?, ?, ?, ?)"
        );

        query.bind(
            1, static_cast<std::int64_t>(auditLog.auditLogId())
        );
        query.bind(
            2, static_cast<std::int64_t>(auditLog.submissionId())
        );
        query.bind(
            3, static_cast<std::int64_t>(auditLog.changedBy())
        );
        query.bind(4, auditLog.changedAt());

        if (auditLog.oldValue().has_value()) {
            query.bind(5, auditLog.oldValue().value());
        } else {
            query.bind(5);
        }

        query.bind(6, auditLog.newValue());
        query.bind(7, auditLog.reason());

        query.exec();
    }

}

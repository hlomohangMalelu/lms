#pragma once

#include "repository/audit_log/IAuditLogRepository.h"

#include <SQLiteCpp/Database.h>

namespace lms::repository {

    class SQLiteAuditLogRepository : public IAuditLogRepository {
        public:
            explicit SQLiteAuditLogRepository(SQLite::Database& database);

            void save(const lms::domain::AuditLog& auditLog) override;

        private:
            // The application owns the database connection.
            SQLite::Database& database_;
    };

}

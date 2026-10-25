#pragma once

#include "domain/audit_log/AuditLog.h"

namespace lms::repository {

    class IAuditLogRepository {
        public:
            virtual ~IAuditLogRepository() = default;

            // Records a mark change in the audit log.
            virtual void save(const lms::domain::AuditLog& auditLog) = 0;
    };

}

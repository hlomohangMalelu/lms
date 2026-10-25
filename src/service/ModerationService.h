#pragma once

#include "repository/audit_log/IAuditLogRepository.h"
#include "repository/mark_record/IMarkRecordRepository.h"
#include "repository/submission/ISubmissionRepository.h"

#include <string>

namespace lms::service {

    class ModerationService {
        public:
            ModerationService(
                lms::repository::ISubmissionRepository& submissionRepository,
                lms::repository::IMarkRecordRepository& markRecordRepository,
                lms::repository::IAuditLogRepository& auditLogRepository
            );

            void moderateMark(
                long long submissionId,
                long long headOfDepartmentId,
                long long markRecordId,
                long long auditLogId,
                double moderatedMark,
                double maxMark,
                const std::string& moderationNotes,
                const std::string& reason,
                const std::string& changedAt
            );

            void approveMark(
                long long submissionId,
                long long headOfDepartmentId,
                const std::string& approvedAt
            );

            void releaseMark(
                long long submissionId,
                long long headOfDepartmentId,
                const std::string& releasedAt
            );

        private:
            lms::repository::ISubmissionRepository& submissionRepository_;
            lms::repository::IMarkRecordRepository& markRecordRepository_;
            lms::repository::IAuditLogRepository& auditLogRepository_;
    };

} // namespace lms::service

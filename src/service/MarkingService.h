#pragma once

#include "domain/late_penalty/ILatePenaltyStrategy.h"
#include "repository/audit_log/IAuditLogRepository.h"
#include "repository/submission/ISubmissionRepository.h"

#include <string>

namespace lms::service {

    class MarkingService {
        public:
            MarkingService(
                lms::repository::ISubmissionRepository& submissionRepository,
                lms::repository::IAuditLogRepository& auditLogRepository,
                const lms::domain::ILatePenaltyStrategy& penaltyStrategy
            );

            void markSubmission(
                long long submissionId,
                long long lecturerId,
                long long auditLogId,
                double rawMark,
                double maxMark,
                const std::string& feedback,
                const std::string& reason,
                const std::string& changedAt
            );

        private:
            lms::repository::ISubmissionRepository& submissionRepository_;
            lms::repository::IAuditLogRepository& auditLogRepository_;
            const lms::domain::ILatePenaltyStrategy& penaltyStrategy_;
    };

}

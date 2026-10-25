
#include "service/MarkingService.h"

#include "domain/audit_log/AuditLog.h"
#include "domain/submission/Submission.h"

#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>

namespace lms::service {

    MarkingService::MarkingService(
        lms::repository::ISubmissionRepository& submissionRepository,
        lms::repository::IAuditLogRepository& auditLogRepository,
        const lms::domain::ILatePenaltyStrategy& penaltyStrategy
        
    ): submissionRepository_(submissionRepository),
          auditLogRepository_(auditLogRepository),
          penaltyStrategy_(penaltyStrategy) {}

    void MarkingService::markSubmission(
        long long submissionId,
        long long lecturerId,
        long long auditLogId,
        double rawMark,
        double maxMark,
        const std::string& feedback,
        const std::string& reason,
        const std::string& changedAt
    ) {
        if (!std::isfinite(maxMark) || maxMark <= 0.0) {
            throw std::invalid_argument(
                "Maximum mark must be a positive finite number."
            );
        }

        if (!std::isfinite(rawMark) ||
            rawMark < 0.0 ||
            rawMark > maxMark) {
            throw std::invalid_argument(
                "Raw mark must be between zero and the maximum mark."
            );
        }

        if (reason.find_first_not_of(" \t\n\r") == std::string::npos) {
            throw std::invalid_argument(
                "A reason is required for every mark change."
            );
        }

        if (changedAt.empty()) {
            throw std::invalid_argument(
                "The mark-change timestamp is required."
            );
        }

        auto submission = submissionRepository_.findById(submissionId);

        if (submission.status() == "RELEASED") {
            throw std::logic_error(
                "A released submission cannot be changed by marking."
            );
        }

        const double finalMark = penaltyStrategy_.calculateFinalMark(
            rawMark,
            maxMark,
            submission.daysLate()
        );

        const double penaltyFraction =
            submission.daysLate() > 3
                ? 1.0
                : static_cast<double>(submission.daysLate()) * 0.10;

        const std::optional<double> oldMark = submission.finalMark();

        const lms::domain::AuditLog auditLog(
            auditLogId,
            submissionId,
            lecturerId,
            changedAt,
            oldMark,
            finalMark,
            reason
        );

        const lms::domain::Submission updatedSubmission(
            submission.submissionId(),
            submission.assignmentId(),
            submission.studentId(),
            "MARKED",
            submission.daysLate(),
            penaltyFraction,
            rawMark,
            finalMark,
            std::optional<std::string>(feedback),
            submission.releasedAt(),
            submission.releasedBy()
        );

        // Write the audit entry before changing the stored submission.
        auditLogRepository_.save(auditLog);
        submissionRepository_.update(updatedSubmission);
    }

}


#include "service/ModerationService.h"

#include "domain/audit_log/AuditLog.h"
#include "domain/mark_record/MarkRecord.h"
#include "domain/submission/Submission.h"

#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>

namespace lms::service {

ModerationService::ModerationService(
    lms::repository::ISubmissionRepository& submissionRepository,
    lms::repository::IMarkRecordRepository& markRecordRepository,
    lms::repository::IAuditLogRepository& auditLogRepository
)
    : submissionRepository_(submissionRepository),
      markRecordRepository_(markRecordRepository),
      auditLogRepository_(auditLogRepository) {
}

void ModerationService::moderateMark(
    long long submissionId,
    long long headOfDepartmentId,
    long long markRecordId,
    long long auditLogId,
    double moderatedMark,
    double maxMark,
    const std::string& moderationNotes,
    const std::string& reason,
    const std::string& changedAt
) {
    if (!std::isfinite(maxMark) || maxMark <= 0.0) {
        throw std::invalid_argument("Maximum mark must be greater than zero.");
    }

    if (!std::isfinite(moderatedMark) ||
        moderatedMark < 0.0 ||
        moderatedMark > maxMark) {
        throw std::invalid_argument(
            "Moderated mark must be between zero and the maximum mark."
        );
    }

    if (reason.find_first_not_of(" \t\n\r") == std::string::npos) {
        throw std::invalid_argument("Audit reason cannot be empty.");
    }

    if (changedAt.empty()) {
        throw std::invalid_argument("Change timestamp cannot be empty.");
    }

    if (moderationNotes.find_first_not_of(" \t\n\r") ==
        std::string::npos) {
        throw std::invalid_argument("Moderation notes cannot be empty.");
    }

    auto submission = submissionRepository_.findById(submissionId);

    if (submission.status() != "MARKED" &&
        submission.status() != "MODERATED") {
        throw std::logic_error(
            "Only marked or previously moderated submissions can be moderated."
        );
    }

    if (!submission.finalMark().has_value()) {
        throw std::logic_error(
            "The submission does not have a final mark to moderate."
        );
    }

    const double oldMark = submission.finalMark().value();

    // Record the change only when the mark actually changes.
    if (oldMark != moderatedMark) {
        lms::domain::AuditLog auditLog(
            auditLogId,
            submissionId,
            headOfDepartmentId,
            changedAt,
            std::optional<double>(oldMark),
            moderatedMark,
            reason
        );

        auditLogRepository_.save(auditLog);
    }

    // Update the submission with the moderated mark.
    lms::domain::Submission updatedSubmission(
        submission.submissionId(),
        submission.assignmentId(),
        submission.studentId(),
        "MODERATED",
        submission.daysLate(),
        submission.penaltyFraction(),
        submission.rawMark(),
        std::optional<double>(moderatedMark),
        submission.feedback(),
        std::nullopt,
        std::nullopt
    );

    submissionRepository_.update(updatedSubmission);

    // A new moderation decision requires approval again.
    lms::domain::MarkRecord record(
        markRecordId,
        submissionId,
        std::optional<long long>(headOfDepartmentId),
        std::optional<double>(moderatedMark),
        std::optional<std::string>(moderationNotes),
        std::optional<std::string>(changedAt),
        std::nullopt,
        std::nullopt
    );

    markRecordRepository_.save(record);
}

void ModerationService::approveMark(
    long long submissionId,
    long long headOfDepartmentId,
    const std::string& approvedAt
) {
    if (approvedAt.empty()) {
        throw std::invalid_argument("Approval timestamp cannot be empty.");
    }

    auto submission = submissionRepository_.findById(submissionId);

    if (submission.status() != "MODERATED") {
        throw std::logic_error(
            "Only moderated submissions can be approved."
        );
    }

    auto record = markRecordRepository_.findBySubmissionId(submissionId);

    if (!record.moderatedMark().has_value() ||
        !record.moderatedBy().has_value()) {
        throw std::logic_error(
            "A moderation decision is required before approval."
        );
    }

    lms::domain::MarkRecord approvedRecord(
        record.markRecordId(),
        record.submissionId(),
        record.moderatedBy(),
        record.moderatedMark(),
        record.moderationNotes(),
        record.moderatedAt(),
        std::optional<long long>(headOfDepartmentId),
        std::optional<std::string>(approvedAt)
    );

    markRecordRepository_.save(approvedRecord);
}

void ModerationService::releaseMark(
    long long submissionId,
    long long headOfDepartmentId,
    const std::string& releasedAt
) {
    if (releasedAt.empty()) {
        throw std::invalid_argument("Release timestamp cannot be empty.");
    }

    auto submission = submissionRepository_.findById(submissionId);

    if (submission.status() != "MODERATED") {
        throw std::logic_error(
            "Only moderated submissions can be released."
        );
    }

    auto record = markRecordRepository_.findBySubmissionId(submissionId);

    if (!record.approvedBy().has_value() ||
        !record.approvedAt().has_value()) {
        throw std::logic_error(
            "The mark must be approved before it can be released."
        );
    }

    if (!submission.finalMark().has_value()) {
        throw std::logic_error(
            "Cannot release a submission without a final mark."
        );
    }

    lms::domain::Submission releasedSubmission(
        submission.submissionId(),
        submission.assignmentId(),
        submission.studentId(),
        "RELEASED",
        submission.daysLate(),
        submission.penaltyFraction(),
        submission.rawMark(),
        submission.finalMark(),
        submission.feedback(),
        std::optional<std::string>(releasedAt),
        std::optional<long long>(headOfDepartmentId)
    );

    submissionRepository_.update(releasedSubmission);
}

} // namespace lms::service

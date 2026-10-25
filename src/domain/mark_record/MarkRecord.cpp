#include "domain/mark_record/MarkRecord.h"

#include <utility>

namespace lms::domain {

    MarkRecord::MarkRecord(
        long long markRecordId,
        long long submissionId,
        std::optional<long long> moderatedBy,
        std::optional<double> moderatedMark,
        std::optional<std::string> moderationNotes,
        std::optional<std::string> moderatedAt,
        std::optional<long long> approvedBy,
        std::optional<std::string> approvedAt
        
    ): markRecordId_(markRecordId),
        submissionId_(submissionId),
        moderatedBy_(moderatedBy),
        moderatedMark_(moderatedMark),
        moderationNotes_(std::move(moderationNotes)),
        moderatedAt_(std::move(moderatedAt)),
        approvedBy_(approvedBy),
        approvedAt_(std::move(approvedAt)) {}

    long long MarkRecord::markRecordId() const noexcept {
        return markRecordId_;
    }

    long long MarkRecord::submissionId() const noexcept {
        return submissionId_;
    }

    const std::optional<long long>& MarkRecord::moderatedBy() const noexcept {
        return moderatedBy_;
    }

    const std::optional<double>& MarkRecord::moderatedMark() const noexcept {
        return moderatedMark_;
    }

    const std::optional<std::string>& MarkRecord::moderationNotes() const noexcept {
        return moderationNotes_;
    }

    const std::optional<std::string>& MarkRecord::moderatedAt() const noexcept {
        return moderatedAt_;
    }

    const std::optional<long long>& MarkRecord::approvedBy() const noexcept {
        return approvedBy_;
    }

    const std::optional<std::string>& MarkRecord::approvedAt() const noexcept {
        return approvedAt_;
    }

}

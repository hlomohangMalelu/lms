
#include "domain/submission/Submission.h"

#include <utility>

namespace lms::domain {

    Submission::Submission(
        long long submissionId,
        long long assignmentId,
        long long studentId,
        std::string status,
        int daysLate,
        double penaltyFraction,
        std::optional<double> rawMark,
        std::optional<double> finalMark,
        std::optional<std::string> feedback,
        std::optional<std::string> releasedAt,
        std::optional<long long> releasedBy
        
    ): submissionId_(submissionId),
        assignmentId_(assignmentId),
        studentId_(studentId),
        status_(std::move(status)),
        daysLate_(daysLate),
        penaltyFraction_(penaltyFraction),
        rawMark_(rawMark),
        finalMark_(finalMark),
        feedback_(std::move(feedback)),
        releasedAt_(std::move(releasedAt)),
        releasedBy_(releasedBy) {}

    long long Submission::submissionId() const noexcept {
        return submissionId_;
    }

    long long Submission::assignmentId() const noexcept {
        return assignmentId_;
    }

    long long Submission::studentId() const noexcept {
        return studentId_;
    }

    const std::string& Submission::status() const noexcept {
        return status_;
    }

    int Submission::daysLate() const noexcept {
        return daysLate_;
    }

    double Submission::penaltyFraction() const noexcept {
        return penaltyFraction_;
    }

    const std::optional<double>& Submission::rawMark() const noexcept {
        return rawMark_;
    }

    const std::optional<double>& Submission::finalMark() const noexcept {
        return finalMark_;
    }

    const std::optional<std::string>& Submission::feedback() const noexcept {
        return feedback_;
    }

    const std::optional<std::string>& Submission::releasedAt() const noexcept {
        return releasedAt_;
    }

    const std::optional<long long>& Submission::releasedBy() const noexcept {
        return releasedBy_;
    }

}

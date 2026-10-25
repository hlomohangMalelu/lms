#include "domain/audit_log/AuditLog.h"

#include <utility>

namespace lms::domain {

    AuditLog::AuditLog(
        long long auditLogId,
        long long submissionId,
        long long changedBy,
        std::string changedAt,
        std::optional<double> oldValue,
        double newValue,
        std::string reason
    ): auditLogId_(auditLogId),
        submissionId_(submissionId),
        changedBy_(changedBy),
        changedAt_(std::move(changedAt)),
        oldValue_(oldValue),
        newValue_(newValue),
        reason_(std::move(reason)) {}

    long long AuditLog::auditLogId() const noexcept {
        return auditLogId_;
    }

    long long AuditLog::submissionId() const noexcept {
        return submissionId_;
    }

    long long AuditLog::changedBy() const noexcept {
        return changedBy_;
    }

    const std::string& AuditLog::changedAt() const noexcept {
        return changedAt_;
    }

    const std::optional<double>& AuditLog::oldValue() const noexcept {
        return oldValue_;
    }

    double AuditLog::newValue() const noexcept {
        return newValue_;
    }

    const std::string& AuditLog::reason() const noexcept {
        return reason_;
    }

}

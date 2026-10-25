#pragma once

#include <optional>
#include <string>

namespace lms::domain {

    class AuditLog {
        private:
            long long auditLogId_;
            long long submissionId_;
            long long changedBy_;
            std::string changedAt_;
            std::optional<double> oldValue_;
            double newValue_;
            std::string reason_;

        public:
            AuditLog(
                long long auditLogId,
                long long submissionId,
                long long changedBy,
                std::string changedAt,
                std::optional<double> oldValue,
                double newValue,
                std::string reason
            );

            long long auditLogId() const noexcept;
            long long submissionId() const noexcept;
            long long changedBy() const noexcept;

            const std::string& changedAt() const noexcept;
            const std::optional<double>& oldValue() const noexcept;
            double newValue() const noexcept;
            const std::string& reason() const noexcept;
    };

}

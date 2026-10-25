#pragma once

#include <optional>
#include <string>

namespace lms::domain {

    class MarkRecord {
        private:
            long long markRecordId_;
            long long submissionId_;
            std::optional<long long> moderatedBy_;
            std::optional<double> moderatedMark_;
            std::optional<std::string> moderationNotes_;
            std::optional<std::string> moderatedAt_;
            std::optional<long long> approvedBy_;
            std::optional<std::string> approvedAt_;

        public:
            MarkRecord(
                long long markRecordId,
                long long submissionId,
                std::optional<long long> moderatedBy,
                std::optional<double> moderatedMark,
                std::optional<std::string> moderationNotes,
                std::optional<std::string> moderatedAt,
                std::optional<long long> approvedBy,
                std::optional<std::string> approvedAt
            );

            long long markRecordId() const noexcept;
            long long submissionId() const noexcept;

            const std::optional<long long>& moderatedBy() const noexcept;
            const std::optional<double>& moderatedMark() const noexcept;
            const std::optional<std::string>& moderationNotes() const noexcept;
            const std::optional<std::string>& moderatedAt() const noexcept;
            const std::optional<long long>& approvedBy() const noexcept;
            const std::optional<std::string>& approvedAt() const noexcept;
    };

}
